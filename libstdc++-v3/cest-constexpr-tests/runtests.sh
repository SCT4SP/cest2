#/bin/bash

# Adding -O3 causes iostream_tests.cpp and stringstream_tests.cpp to fail due
# to the use of a static member in ~locale(). Fix with dlsym interposition.

GNUC_ROOT="$HOME/repos/SCT4SP/gcc/install"
GCC_BIN=$GNUC_ROOT/bin/g++
#GCC_BIN=/opt/gcc-latest/bin/g++ # This doesn't work ATM
#CLANG_ROOT="$HOME/code/ext/llvm_src/build-25-Oct-2023-Wed/llvm-project-main/install"
CLANG_ROOT=$CLANGOZ_ROOT   # Any recent (months) clang will work

# The _GLIBCXX_CEST_CONSTEXPR and _GLIBCXX_CEST_VERSION below are not required
# when using SCT4SP/gcc/install, but
MYGCC="$GCC_BIN -g -std=c++26 -Winvalid-constexpr -Wl,-rpath,"$GNUC_ROOT/lib64:$LD_LIBRARY_PATH" -I $GNUC_ROOT/constexpr-std-headers/include/c++/14.0.0 -I $GNUC_ROOT/constexpr-std-headers/include/c++/14.0.0/x86_64-pc-linux-gnu -D_GLIBCXX_CEST_CONSTEXPR=constexpr -D_GLIBCXX_CEST_VERSION=1 -fsanitize=address -static-libasan"

MYCLANG="$CLANG_ROOT/bin/clang++ -g -std=c++2c -Winvalid-constexpr -Wl,-rpath,"$GNUC_ROOT/lib64:$LD_LIBRARY_PATH" -I $GNUC_ROOT/constexpr-std-headers/include/c++/14.0.0 -I $GNUC_ROOT/constexpr-std-headers/include/c++/14.0.0/x86_64-pc-linux-gnu -L $GNUC_ROOT/lib64 -D_GLIBCXX_CEST_CONSTEXPR=constexpr -D_GLIBCXX_CEST_VERSION=1 -fsanitize=address"

FILES_GCC="deque_tests.cpp list_tests.cpp queue_tests.cpp forward_list_tests.cpp map_tests.cpp set_tests.cpp iostream_tests.cpp algorithm_tests.cpp stringstream_tests.cpp cctype_tests.cpp memory_tests.cpp functional_tests.cpp policy_tests.cpp"

FILES_CLANG="deque_tests.cpp list_tests.cpp queue_tests.cpp forward_list_tests.cpp map_tests.cpp set_tests.cpp iostream_tests.cpp algorithm_tests.cpp stringstream_tests.cpp cctype_tests.cpp memory_tests.cpp functional_tests.cpp policy_tests.cpp"

for FILE in $FILES_GCC
do
  $MYGCC $FILE -ltbb && ./a.out   #  -ltbb is only needed for policy_tests.cpp
done

for FILE in $FILES_CLANG
do
  $MYCLANG $FILE -ltbb && ./a.out
done
