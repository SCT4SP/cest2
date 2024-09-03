#/bin/bash

# Adding -O3 causes iostream_tests.cpp and stringstream_tests.cpp to fail due
# to the use of a static member in ~locale(). Fix with dlsym interposition.

# Contains bin, lib64, include,constexpr-std-headers, etc.
GNUC_ROOT="$CEST2_INCLUDE/../.."
#GNUC_ROOT="/opt/gcc-latest" # No. Need C'est 2 binaries, for runtime, for now
VER=14.0.1    # previously 14.0.0

# The _GLIBCXX_CEST_CONSTEXPR and _GLIBCXX_CEST_VERSION below are not required
# when using SCT4SP/gcc/install, but
MYGCC="$GNUC_ROOT/bin/g++ -g -std=c++26 -Winvalid-constexpr -Wl,-rpath,"$GNUC_ROOT/lib64:$LD_LIBRARY_PATH" -I $CEST2_INCLUDE/c++/$VER -I $CEST2_INCLUDE/c++/$VER/x86_64-pc-linux-gnu -D_GLIBCXX_CEST_CONSTEXPR=constexpr -D_GLIBCXX_CEST_VERSION=1 -fsanitize=address -static-libasan"

MYCLANG="clang++ -g -std=c++26 -Winvalid-constexpr -Wl,-rpath,"$GNUC_ROOT/lib64:$LD_LIBRARY_PATH" -I $CEST2_INCLUDE/c++/$VER -I $CEST2_INCLUDE/c++/$VER/x86_64-pc-linux-gnu -L $GNUC_ROOT/lib64 -D_GLIBCXX_CEST_CONSTEXPR=constexpr -D_GLIBCXX_CEST_VERSION=1 -fsanitize=address"

FILES_GCC="deque_tests.cpp list_tests.cpp queue_tests.cpp forward_list_tests.cpp map_tests.cpp set_tests.cpp iostream_tests.cpp algorithm_tests.cpp stringstream_tests.cpp cctype_tests.cpp memory_tests.cpp functional_tests.cpp policy_tests.cpp string_tests.cpp chrono_tests.cpp"

FILES_CLANG="deque_tests.cpp list_tests.cpp queue_tests.cpp forward_list_tests.cpp map_tests.cpp set_tests.cpp iostream_tests.cpp algorithm_tests.cpp stringstream_tests.cpp cctype_tests.cpp memory_tests.cpp functional_tests.cpp policy_tests.cpp string_tests.cpp chrono_tests.cpp"

for FILE in $FILES_GCC
do
  $MYGCC $FILE -ltbb && ./a.out   #  -ltbb is only needed for policy_tests.cpp
done

for FILE in $FILES_CLANG
do
  $MYCLANG $FILE -ltbb && ./a.out
done
