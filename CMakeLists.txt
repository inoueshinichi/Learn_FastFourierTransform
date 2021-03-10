cmake_minimum_required(VERSION 3.0.0)
project(Fourier VERSION 0.1.0)

# include(CTest)
# enable_testing()

# C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
message(STATUS "CMAKE_CXX_STANDARD: ${CMAKE_CXX_STANDARD}")

find_package(Python3 COMPONENTS Development NumPy)

add_executable(test_maptlotlibcpp test_matplotlibcpp.cpp)
target_include_directories(test_maptlotlibcpp PRIVATE ${Python3_INCLUDE_DIRS} ${Python3_NumPy_INCLUDE_DIRS})
target_link_libraries(test_maptlotlibcpp PRIVATE Python3::Python Python3::NumPy)


set(TARGET_NAME Fourier)
add_executable(${TARGET_NAME} 
    main.cpp
    fourier.hpp
    fft_policy.hpp
)
target_include_directories(${TARGET_NAME} PRIVATE ${Python3_INCLUDE_DIRS} ${Python3_NumPy_INCLUDE_DIRS})
target_link_libraries(${TARGET_NAME} PRIVATE Python3::Python Python3::NumPy)


# コンパイラオプション
# https://cmake.org/cmake/help/latest/command/target_compile_options.html?highlight=target_compile_options
target_compile_options(${TARGET_NAME} PRIVATE
    # gcc
    $<$<CXX_COMPILER_ID:GNU>:-Wall -pedantic --pedantic-errors>
    $<$<AND:$<CXX_COMPILER_ID:GNU>,$<CONFIG:Debug>>:-O0 -g>
    $<$<AND:$<CXX_COMPILER_ID:GNU>,$<CONFIG:Release>>:-O3>
    $<$<AND:$<CXX_COMPILER_ID:GNU>,$<CONFIG:MinSizeRel>>:-Os>
    $<$<AND:$<CXX_COMPILER_ID:GNU>,$<CONFIG:RelWithDebgInfo>>:-O2 -g>
    # clang & apple clang
    $<$<CXX_COMPILER_ID:Clang,AppleClang>:-Wall --pedantic-errors>
    $<$<AND:$<CXX_COMPILER_ID:Clang,AppleClang>,$<CONFIG:Debug>>:-O0 -g>
    $<$<AND:$<CXX_COMPILER_ID:Clang,AppleClang>,$<CONFIG:Release>>:-O3>
    $<$<AND:$<CXX_COMPILER_ID:Clang,AppleClang>,$<CONFIG:MinSizeRel>>:-Os>
    $<$<AND:$<CXX_COMPILER_ID:Clang,AppleClang>,$<CONFIG:RelWithDebgInfo>>:-O2 -g>
    # msvc
    $<$<CXX_COMPILER_ID:MSVC>:/GR /EHsc /W4 /utf-8 /Zc:__cplusplus /bigobj> # /GR /EHsc /utf-8 /Zc:__cplusplus /bigobj
    $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:Debug>>:/Ob0 /Od /MDd /Zi /RTC1>
    $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:Release>>:/Ob2 /O2 /MD>
    $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:MinSizeRel>>:/O1 /MD>
    $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:RelWithDebgInfo>>:/Od /MDd /Zi /RTC1>
)

# 定義済みマクロ
# https://cmake.org/cmake/help/latest/command/target_compile_definitions.html?highlight=target_compile_definitions
target_compile_definitions(${TARGET_NAME} PRIVATE
    # gcc
    $<$<CXX_COMPILER_ID:GNU>:>
    $<$<AND:$<CXX_COMPILER_ID:GNU>,$<CONFIG:Debug>>:>
    $<$<AND:$<CXX_COMPILER_ID:GNU>,$<CONFIG:Release>>:NDEBUG>
    $<$<AND:$<CXX_COMPILER_ID:GNU>,$<CONFIG:MinSizeRel>>:NDEBUG>
    $<$<AND:$<CXX_COMPILER_ID:GNU>,$<CONFIG:RelWithDebgInfo>>:>
    # clang & apple clang
    $<$<CXX_COMPILER_ID:Clang>:>
    $<$<AND:$<CXX_COMPILER_ID:Clang>,$<CONFIG:Debug>>:>
    $<$<AND:$<CXX_COMPILER_ID:Clang>,$<CONFIG:Release>>:NDEBUG>
    $<$<AND:$<CXX_COMPILER_ID:Clang>,$<CONFIG:MinSizeRel>>:NDEBUG>
    $<$<AND:$<CXX_COMPILER_ID:Clang>,$<CONFIG:RelWithDebgInfo>>:>
    # msvc
    $<$<CXX_COMPILER_ID:MSVC>:WIN32 _WINDOWS>
    $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:Debug>>:>
    $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:Release>>:NDEBUG>
    $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:MinSizeRel>>:NDEBUG>
    $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:RelWithDebgInfo>>:>
)

# set(CPACK_PROJECT_NAME ${PROJECT_NAME})
# set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
# include(CPack)
