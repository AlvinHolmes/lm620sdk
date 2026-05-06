make clean

rm CMakeCache.txt cmake_install.cmake cmake_uninstall.cmake CTestTestfile.cmake *.map Makefile *test
rm -rf CMakeFiles
cmake -G"Unix Makefiles" -DCMAKE_TOOLCHAIN_FILE=toolchain.cmake \
	  -DCMAKE_C_FLAGS="-march=rv32ima_zca_zcb_zcmp_zcmt_zba_zbb_zbc_zbs -mabi=ilp32 -mcmodel=medlow -mdiv -ffunction-sections -fno-builtin-printf -fno-common -fdata-sections -fshort-enums -fno-builtin -mstrict-align -MMD -MP -msave-restore -fno-unroll-loops -fno-strict-aliasing -Werror=int-conversion -D_USE_CMPL_GCC -Dgcc -DN310 -DRISCV_DSP -D_GLIBCXX_INCLUDE_NEXT_C_HEADERS -Wno-unused-function -Wno-unknown-pragmas -Wno-builtin-macro-redefined -Wno-address -Wno-comment -U__FILE__ -gdwarf-4 -gstrict-dwarf" \
      -DCMAKE_INSTALL_PREFIX= ../build \
	  -DCMAKE_BUILD_TYPE=RELEASE \
	  -DENABLE_STATIC=1 \
	  -DENABLE_SHARED=0 \
	  ../
	  
make

cp *.a ../../
cp *.h ../../

make clean
rm CMakeCache.txt cmake_install.cmake cmake_uninstall.cmake CTestTestfile.cmake *.map Makefile *test
rm -rf CMakeFiles





