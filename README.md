export LLVM_DIR=<installation/dir/of/llvm/19>
mkdir build
cd build
cmake -DLT_LLVM_INSTALL_DIR=$LLVM_DIR .
make
