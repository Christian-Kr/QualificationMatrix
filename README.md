![Linux](https://github.com/Christian-Kr/QualificationMatrix/actions/workflows/ubuntu-latest.yml/badge.svg?branch=master)

# QualificationMatrix

This project creates a software that will help you in managing a qualification matrix for 
your employees and includes the following main features:

- An overview about all trainings for your employees.
- A way to handle all certificates for trainings.
- Create signing list for your trainings.

## Install

### Requirements

The depends on the following libraries:

- Botan (https://botan.randombit.net)
- Qt (https://www.qt.io)

To build the project you need the following tools:

- GCC (https://gcc.gnu.org) or Clang (https://clang.llvm.org)
- CMake (https://cmake.org)

The software should build on current Window, Linux and Mac OS X (because of Qt and Botan).

### Build

Be sure the software you need to build is installed on your system and is part of the 
system paths.

Clone the project:
```Bash
git clone https://github.com/Christian-Kr/QualificationMatrix
```

Build the project:
```Bash
cd QualificationMatrix
mkdir build
cd build
cmake ..
make
```

### Installation

The installation process has not been finished yet, so just start the application from the build 
folder for now. You can manually copy the executable file to a different path with the qt libraries.

## Contribution

If you want to help with the development, just create a github project fork it get into the wild.
Additionally to the github account, you can contact the developer by writing an email to
CerebrosuS_aedd_gmx_dot_net.

## License

The software QualificationMatrix is developed and distributed under the terms of the 
GPLv3 (https://www.gnu.org/licenses/gpl-3.0.en.html). A copy of the license can be found in the
file COPYING.txt.
