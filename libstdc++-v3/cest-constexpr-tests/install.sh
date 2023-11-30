#!/bin/bash

if [ "$1" == "" ]; then                                                           echo "error: Please provide a destination directory for the C'est constexpr headers."
  echo e.g. bash install.sh \$PWD/../install/constexpr-std-headers
  echo Note that this script should be called from the CMake build directory
  exit -1
fi

INSTALL_DIR=$1
echo INSTALL DIR IS "$INSTALL_DIR"

CPPCONFIG="$(find "$INSTALL_DIR" -type f -name c++config.h 2>/dev/null)"

make -C x86_64-pc-linux-gnu/libstdc++-v3/include prefix="$INSTALL_DIR" install-data-am && make -C x86_64-pc-linux-gnu/libstdc++-v3/libsupc++ prefix="$INSTALL_DIR" install-data-am

# if "c++config.h didn't exist earlier", then edit the one installed by make
# on the lines above, to include the 2 _GLIBCXX_CEST_ ... macros
if [ "$CPPCONFIG" == "" ]; then
  CPPCONFIG="$(find "$INSTALL_DIR" -type f -name c++config.h 2>/dev/null)"
  sed -i -e 's/#ifndef _GLIBCXX17_INLINE/\/\/ Defined if extra constexpr support is present.\n#define _GLIBCXX_CEST_VERSION 1\n\n#if _GLIBCXX_CEST_VERSION\n# ifndef _GLIBCXX_CEST_CONSTEXPR\n#  define _GLIBCXX_CEST_CONSTEXPR _GLIBCXX23_CONSTEXPR\n# endif\n#endif\n\n#ifndef _GLIBCXX17_INLINE/' "$CPPCONFIG"
fi

##### The sed command adds the following two lines before _GLIBCXX17_INLINE

# // Defined if extra constexpr support is present.
#define _GLIBCXX_CEST_VERSION 1

#if _GLIBCXX_CEST_VERSION
# ifndef _GLIBCXX_CEST_CONSTEXPR
#  define _GLIBCXX_CEST_CONSTEXPR _GLIBCXX23_CONSTEXPR
# endif
#endif
