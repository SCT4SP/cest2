#!/bin/bash

if [ "$1" == "" ]; then                                                           echo "error: Please provide a destination directory for the C'est constexpr headers."
  echo e.g. bash install.sh \$PWD/../install/constexpr-std-headers
  exit -1
fi

INSTALL_DIR=$1
echo INSTALL DIR IS "$INSTALL_DIR"

CPPCONFIG="$(find "$INSTALL_DIR" -type f -name c++config.h 2>/dev/null)"

make -C x86_64-pc-linux-gnu/libstdc++-v3/include prefix="$INSTALL_DIR" install-data-am && make -C x86_64-pc-linux-gnu/libstdc++-v3/libsupc++ prefix="$INSTALL_DIR" install-data-am

if [ "$CPPCONFIG" == "" ]; then
  echo File doesnT exist
else
  echo File EXISTS: "$CPPCONFIG"
fi

##### Add the following two lines before _GLIBCXX17_INLINE
##### Remove the ##### here
##### // Defined if extra constexpr support is present.
#define _GLIBCXX_CEST_VERSION 1

#if _GLIBCXX_CEST_VERSION
# ifndef _GLIBCXX_CEST_CONSTEXPR
#  define _GLIBCXX_CEST_CONSTEXPR _GLIBCXX23_CONSTEXPR
# endif
#endif
