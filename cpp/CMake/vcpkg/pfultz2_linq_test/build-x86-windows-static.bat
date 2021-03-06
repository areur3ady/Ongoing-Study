vcpkg install pfultz2-linq:x86-windows-static
vcpkg install boost-assign boost-foreach boost-test --triplet x86-windows-static
mkdir build-x86-windows-static
cd build-x86-windows-static
cmake .. -DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake" -DVCPKG_TARGET_TRIPLET=x86-windows-static -DCMAKE_GENERATOR_PLATFORM=Win32 -DUSE_MSVC_STATIC_RUNTIME=ON
cmake --build . --config Release
ctest -C Release
pause
