#!/bin/bash

if [ "$1" == "" ]; then                                                           echo "error: Please provide a destination directory for the C'est constexpr headers."
  echo e.g. bash install.sh \$PWD/../install/constexpr-std-headers
  echo Note that this script should be called from the CMake build directory
  exit -1
fi

INSTALL_DIR=$1
echo INSTALL DIR IS "$INSTALL_DIR"

make -C x86_64-pc-linux-gnu/libstdc++-v3/include prefix="$INSTALL_DIR" install-data-am && make -C x86_64-pc-linux-gnu/libstdc++-v3/libsupc++ prefix="$INSTALL_DIR" install-data-am

# c++config.h will be installed by the make commands above; but will not
# include the 2 crucial _GLIBCXX_CEST_ ... macros. So add them using sed:
CPPCONFIG="$(find "$INSTALL_DIR" -type f -name c++config.h 2>/dev/null)"
sed -i -e 's/#ifndef _GLIBCXX17_INLINE/\/\/ Defined if extra constexpr support is present.\n#define _GLIBCXX_CEST_VERSION 1\n\n#if _GLIBCXX_CEST_VERSION\n# ifndef _GLIBCXX_CEST_CONSTEXPR\n#  define _GLIBCXX_CEST_CONSTEXPR _GLIBCXX26_CONSTEXPR\n# endif\n#endif\n\n#ifndef _GLIBCXX17_INLINE/' "$CPPCONFIG"

##### The sed command adds the following two lines before _GLIBCXX17_INLINE

# // Defined if extra constexpr support is present.
#define _GLIBCXX_CEST_VERSION 1

#if _GLIBCXX_CEST_VERSION
# ifndef _GLIBCXX_CEST_CONSTEXPR
#  define _GLIBCXX_CEST_CONSTEXPR _GLIBCXX26_CONSTEXPR
# endif
#endif
