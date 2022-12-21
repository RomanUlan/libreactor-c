# libreactor-c

## Description
This is library which implements reactor POSA 2 design pattern using epoll.
Code is written in C language but following OOP style. It also includes unit/module development tests.

## Content
```
<ROOTDIR> --> it is just a marker of folder where you store libreactor-c
|
--libreactor-c
  |
  --include -> it contains public header files which will be installed by default in /usr/include
  |
  --src -> it contains main sources which will be compiled to libreactor-c.so
  |
  --tst
  | |
  | --example-usage -> it contains example implementation of telnet echo server using libreactor-c
  | |
  | --unit-tests -> it contains unit tests written using googletest and c-mock
  |
  -- LICENSE.md -> license file
  |
  -- makefile -> makefile which should be used for builds
  |
  -- README.md -> file which you're reading right now ;)
```

## Dependencies
From deployment point of view this library has just a Linux system dependencies - epoll.
Unit tests are based on:
* GTest (googletest: https://github.com/google/googletest)
* C-mock (developed by Hubert Jagodziński: https://github.com/hjagodzinski/C-Mock)
* and for coverage lcov and genhtml is used.

## Build and installation

### Production build
Just run
```
$ make
```

### Build with debugging symbols
Just run
```
$ make debug
```

### Build and run tests
Just run
```
$ make tst
```
To view coverage report use you favorite WEB browser and watch `tst/unit-tests/html/index.html` file, e.g.
```
$ google-chrome tst/unit-tests/html/index.html
```
**Note 1:** GTest and C-mock main directories should be in the same directory where you store libreactor-c directory <ROOTDIR>. <BR>
**Note 2:** This will run also the coverage report.


### Install library in system
Just run as root
```
# make install
```

### Uninstall library from the system
Just run as root
```
# make uninstall
```

### Cleaning builds
Clean only library build using:
```
$ make clean
```
Clean including deps and tests:
```
$ make clean-all
```

## Example
To run the example stored in <ROOTDIR>/libreactor-c/tst/example-usage just jump
to this folder and run as follow:
```
$ make
$ ./run.sh
```
