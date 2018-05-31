

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

/**
 *  @brief A helper include file to implement the Python modules
 *
 *  Use this helper file this way:
 *
 *  #include "pymodHelper.h"
 *  DEFINE_PYMOD(mymod, "mymod", "KLayout Test module klayout.mymod")
 */

#include <Python.h>
#include "pya.h"
#include "gsi.h"
#include "gsiExpression.h"

static PyObject *
module_init (const char *mod_name, const char *mod_description)
{
  static pya::PythonModule module;
  std::string mod_qname (std::string ("klayout.") + mod_name);
  std::string import_text ("'import " + mod_qname + "'");
  
  PYA_TRY
  
    gsi::initialize ();

    //  required for the tiling processor for example
    gsi::initialize_expressions ();

    module.init (mod_qname.c_str (), mod_description);
    module.make_classes (mod_name);

    return module.take_module ();

  PYA_CATCH_ANYWHERE
  
  module.delete_module ();
  return 0;
}

#if PY_MAJOR_VERSION < 3

#define DEFINE_PYMOD(__name__, __name_str__, __description__) \
  PyMODINIT_FUNC \
  DEF_INSIDE_PUBLIC \
  init##__name__ () \
  { \
    module_init (__name_str__, __description__); \
  } \

#else

#define DEFINE_PYMOD(__name__, __name_str__, __description__) \
  PyMODINIT_FUNC \
  DEF_INSIDE_PUBLIC \
  PyInit_##__name__ () \
  { \
    return module_init (__name_str__, __description__); \
  } \

#endif

