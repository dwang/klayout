
/*

  KLayout Layout Viewer
  Copyright (C) 2006-2018 Matthias Koefferlein

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

*/


#ifndef _HDR_pya
#define _HDR_pya

#include "pyaRefs.h"

#include "gsi.h"
#include "gsiInterpreter.h"
#include "tlScriptError.h"
#include "pyaCommon.h"

#include <list>
#include <string>
#include <set>
#include <memory>

struct _typeobject;
typedef _typeobject PyTypeObject;

struct _frame;
typedef _frame PyFrameObject;

struct PyMethodDef;
struct PyGetSetDef;

namespace gsi
{
  class MethodBase;
}

namespace pya
{

/**
 *  Two helper macros that frame a piece of code which potentially executes Python code
 */

#define PYTHON_BEGIN_EXEC \
  try { \
    PythonInterpreter::instance ()->begin_execution ();

#define PYTHON_END_EXEC \
    PythonInterpreter::instance ()->end_execution (); \
  } catch (...) { \
    PythonInterpreter::instance ()->end_execution (); \
    throw; \
  } 

/**
 *  Two helper macros that translate C++ exceptions into Python errors
 */

#define PYA_TRY \
  { \
    try {

#define PYA_CATCH(where) \
    } catch (tl::ExitException &ex) { \
      PyErr_SetObject (PyExc_SystemExit, PyLong_FromLong (ex.status ())); \
    } catch (std::exception &ex) { \
      std::string msg = std::string(ex.what ()) + tl::to_string (QObject::tr (" in ")) + (where); \
      PyErr_SetString (PyExc_RuntimeError, msg.c_str ()); \
    } catch (tl::Exception &ex) { \
      std::string msg; \
      msg = ex.msg () + tl::to_string (QObject::tr (" in ")) + (where); \
      PyErr_SetString (PyExc_RuntimeError, msg.c_str ()); \
    } catch (...) { \
      std::string msg = tl::to_string (QObject::tr ("Unspecific exception in ")) + (where); \
      PyErr_SetString (PyExc_RuntimeError, msg.c_str ()); \
    } \
  }

#define PYA_CATCH_ANYWHERE \
    } catch (tl::ExitException &ex) { \
      PyErr_SetObject (PyExc_SystemExit, PyLong_FromLong (ex.status ())); \
    } catch (std::exception &ex) { \
      PyErr_SetString (PyExc_RuntimeError, ex.what ()); \
    } catch (tl::Exception &ex) { \
      PyErr_SetString (PyExc_RuntimeError, ex.msg ().c_str ()); \
    } catch (...) { \
      PyErr_SetString (PyExc_RuntimeError, tl::to_string (QObject::tr ("Unspecific exception in ")).c_str ()); \
    } \
  }

/**
 *  @brief A class encapsulating a python exception
 */
class PYA_PUBLIC PythonError
  : public tl::ScriptError
{
public:
  PythonError (const char *msg, const char *cls, const std::vector <tl::BacktraceElement> &backtrace)
    : tl::ScriptError (msg, cls, backtrace)
  { }

  PythonError (const char *msg, const char *sourcefile, int line, const char *cls, const std::vector <tl::BacktraceElement> &backtrace)
    : tl::ScriptError (msg, sourcefile, line, cls, backtrace)
  { }

  PythonError (const PythonError &d)
    : tl::ScriptError (d)
  { }
};

class MethodTable;

/**
 *  @brief A representative for a Python module
 *  Instantiate this object to represent a python module.
 *  If used externally (for a library), call init with the module name.
 *  Then call make_classes to create the individual classes.
 */
class PYA_PUBLIC PythonModule
{
public:
  /**
   *  @brief Constructor
   */
  PythonModule ();

  /**
   *  @brief Destructor
   */
  ~PythonModule ();

  /**
   *  @brief Initializes the module
   *  This entry point is for external use where the module has not been created yet
   */
  void init (const char *mod_name, const char *description);

  /**
   *  @brief Initializes the module
   *  This entry point is for internal use where the module is created by the interpreter
   */
  void init (const char *mod_name, PyObject *module);

  /**
   *  @brief Creates the classes after init has been called
   */
  void make_classes (const char *mod_name = 0);

  /**
   *  @brief Gets the GSI class for a Python class
   */
  static const gsi::ClassBase *cls_for_type (PyTypeObject *type);

  /**
   *  @brief The reverse: gets a Python class for a GSI class or NULL if there is no binding
   */
  static PyTypeObject *type_for_cls (const gsi::ClassBase *cls);

  /**
   *  @brief Returns additional Python-specific documentation for the given method
   *  If no specific documentation exists, an empty string is returned.
   */
  static std::string python_doc (const gsi::MethodBase *method);

  /**
   *  @brief Gets the PyModule object
   */
  PyObject *module ();

  /**
   *  @brief Gets the PyModule object
   *  This method will release the ownership over the PyObject
   */
  PyObject *take_module ();

  /**
   *  @brief Deletes the PyModule object
   */
  void delete_module ();

private:
  void add_python_doc (const gsi::ClassBase &cls, const MethodTable *mt, int mid, const std::string &doc);
  PyMethodDef *make_method_def ();
  PyGetSetDef *make_getset_def ();
  char *make_string (const std::string &s);

  std::list<std::string> m_string_heap;
  std::vector<PyMethodDef *> m_methods_heap;
  std::vector<PyGetSetDef *> m_getseters_heap;

  std::string m_mod_name, m_mod_description;
  std::string m_base_class_name;
  PythonRef mp_module;
  PythonRef mp_base_class;
  char *mp_mod_def;

  static std::map<const gsi::MethodBase *, std::string> m_python_doc;
  static std::map <PyTypeObject *, const gsi::ClassBase *> m_cls_map;
  static std::map <const gsi::ClassBase *, PyTypeObject *> m_rev_cls_map;
};

/**
 *  @brief The python interpreter wrapper class
 */
class PYA_PUBLIC PythonInterpreter
  : public gsi::Interpreter
{
public:
  /**
   *  @brief The constructor 
   */
  PythonInterpreter ();

  /**
   *  @brief The destructor 
   */
  ~PythonInterpreter ();

  /**
   *  @brief Add the given path to the search path
   */
  void add_path (const std::string &path);

  /**
   *  @brief Adds a package location to this interpreter
   */
  void add_package_location (const std::string &package_path);

  /**
   *  @brief Removes a package location from this interpreter
   */
  void remove_package_location (const std::string &package_path);

  /**
   *  @brief Requires the given module
   */
  void require (const std::string &filename);

  /**
   *  @brief Set the given debugger scope
   *
   *  The debugger scope is the element to which the back trace will be reduced.
   *  Specifically this suppresses calls from inner functions called from that file.
   *  This is useful for DSL implementations.
   */
  void set_debugger_scope (const std::string &filename);

  /**
   *  @brief Removes the debugger scope
   */
  void remove_debugger_scope ();

  /**
   *  @brief Ignores the next exception
   *
   *  This is useful for suppressing reraised exceptions in the debugger.
   */
  void ignore_next_exception ();

  /**
   *  @brief Load the given file
   */
  void load_file (const std::string &filename);

  /**
   *  @brief Implementatiom of gsi::Interpreter::eval_string
   */
  void eval_string (const char *string, const char *filename = 0, int line = 1, int context = -1);

  /**
   *  @brief Implementatiom of gsi::Interpreter::eval_expr
   */
  tl::Variant eval_expr (const char *string, const char *filename = 0, int line = 1, int context = -1);
 
  /**
   *  @brief Implementatiom of gsi::Interpreter::eval_string_and_print
   */ 
  void eval_string_and_print (const char *string, const char *filename = 0, int line = 1, int context = -1);

  /**
   *  @brief Returns an inspector for the given context
   */
  virtual gsi::Inspector *inspector (int context = -1);

  /**
   *  @brief Defines a global variable with the given name and value 
   */
  void define_variable (const std::string &name, const std::string &value);

  /**
   *  @brief Gets a value indicating whether the interpreter is available
   */
  bool available () const;

  /**
   *  @brief Installs the given console for output
   */
  void push_console (gsi::Console *console);

  /**
   *  @brief Removes the given console
   */
  void remove_console (gsi::Console *console);

  /**
   *  @brief Installs the given execution handler
   *
   *  The execution handler is informed when the interpreter enters code execution (also
   *  from the outside, i.e. a method reimplementing a C++ method or a event handler).
   *  During execution, the handler receives trace events which allow him to intercept
   *  execution.
   */
  void push_exec_handler (gsi::ExecutionHandler *exec_handler);

  /**
   *  @brief Removes the given execution handler
   */
  void remove_exec_handler (gsi::ExecutionHandler *exec_handler);

  /**
   *  @brief Fetch the version string
   *
   *  Returns an empty string when no Python interpreter is installed.
   */
  std::string version () const;

  /**
   *  @brief Returns the current console
   */
  gsi::Console *current_console () const;

  /**
   *  @brief Indicates the start of execution of a Python script
   *  This method is intended for internal use.
   */
  void begin_execution ();

  /**
   *  @brief Indicates the end of execution of a Python script
   *  This method is intended for internal use.
   */
  void end_execution ();

  /**
   *  @brief Callback from the trace function
   */
  int trace_func (PyFrameObject *frame, int event, PyObject *arg);

  /**
   *  @brief Returns the singleton reference
   */
  static PythonInterpreter *instance ();

  /**
   *  @brief Provide a first (basic) initialization
   */
  static void initialize ();

private:
  size_t prepare_trace (PyObject *);
  tl::Variant eval_int (const char *string, const char *filename, int line, bool eval_expr, int context);
  void get_context (int context, PythonRef &globals, PythonRef &locals, const char *file);
  char *make_string (const std::string &s);

  std::list<std::string> m_string_heap;

  PythonRef m_stdout_channel, m_stderr_channel;
  PythonPtr m_stdout, m_stderr;
  std::set<std::string> m_package_paths;

  gsi::Console *mp_current_console;
  std::vector<gsi::Console *> m_consoles;
  gsi::ExecutionHandler *mp_current_exec_handler;
  std::vector<gsi::ExecutionHandler *> m_exec_handlers;

  //  trace function context
  int m_current_exec_level;
  bool m_in_trace;
  bool m_block_exceptions;
  bool m_ignore_next_exception;
  std::string m_debugger_scope;
  PyFrameObject *mp_current_frame;
  std::map<PyObject *, size_t> m_file_id_map;
  wchar_t *mp_py3_app_name;
  pya::PythonModule m_pya_module;
};

}

#endif

