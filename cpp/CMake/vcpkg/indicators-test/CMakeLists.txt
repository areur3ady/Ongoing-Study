cmake_minimum_required(VERSION 3.9)

project(indicators-test CXX)

find_package(indica CONFIG REQUIRED)

if (NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/demo.cpp")
    file(DOWNLOAD
        "https://raw.githubusercontent.com/p-ranav/indicators/master/demo/demo.cpp"
        "${CMAKE_CURRENT_BINARY_DIR}/demo.cpp"
    )
endif ()

add_executable(indicators-test "${CMAKE_CURRENT_BINARY_DIR}/demo.cpp")

if (MSVC)
    target_compile_definitions(indicators-test PRIVATE NOMINMAX)
    target_compile_options(indicators-test PRIVATE /source-charset:utf-8)
endif ()

target_link_libraries(indicators-test PRIVATE indica::indica)

# References:
# [How do I force cmake to include “-pthread” option during compilation?](https://stackoverflow.com/questions/5395309/how-do-i-force-cmake-to-include-pthread-option-during-compilation)
# [cmake and libpthread](https://stackoverflow.com/questions/1620918/cmake-and-libpthread)
