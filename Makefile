default: help

help:
	@echo "make build PYTHON_VERSION=B37"
	@echo "make deploy PYTHON_VERSION=B37"
	@echo "make test MACOS_VERSION=HighSierra"
	@echo "Valid Mac OS Versions: [Yosemite, ElCapitan, Sierra, HighSierra]"
	@echo "Valid Python Version: [nil, Sys, B37]"

build:
	./build4mac.py -p $(PYTHON_VERSION) -q Qt5Brew -c
	./build4mac.py -p $(PYTHON_VERSION) -q Qt5Brew

deploy:
	./build4mac.py -p $(PYTHON_VERSION) -q Qt5Brew -y
	
test:
	qt5.pkg.macos--release/klayout.app/Contents/MacOS/klayout -b -r test-pylib-script.py
	cd qt5.build.macos-$(MACOS_VERSION)-release
	ln -s klayout.app/Contents/MacOS/klayout klayout
	export TESTTMP=testtmp
	export TESTSRC=..
	./ut_runner -h
	./ut_runner -s
