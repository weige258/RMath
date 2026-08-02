cmake_minimum_required(VERSION 3.20)
project(RMath LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# SIMD detection and flags (可选，避免向所有消费者强制透传 -mavx2/-mfma)
option(RMATH_ENABLE_SIMD "Enable SIMD code generation for the rmath interface target" ON)
include(CheckCXXCompilerFlag)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
    check_cxx_compiler_flag("-mavx2" COMPILER_SUPPORTS_AVX2)
    check_cxx_compiler_flag("-mavx" COMPILER_SUPPORTS_AVX)
    check_cxx_compiler_flag("-msse4.2" COMPILER_SUPPORTS_SSE42)
    check_cxx_compiler_flag("-msse2" COMPILER_SUPPORTS_SSE2)

    if(COMPILER_SUPPORTS_AVX2)
        set(RMATH_SIMD_FLAGS -mavx2 -mfma)
    elseif(COMPILER_SUPPORTS_AVX)
        set(RMATH_SIMD_FLAGS -mavx)
    elseif(COMPILER_SUPPORTS_SSE42)
        set(RMATH_SIMD_FLAGS -msse4.2)
    elseif(COMPILER_SUPPORTS_SSE2)
        set(RMATH_SIMD_FLAGS -msse2)
    endif()
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    check_cxx_compiler_flag("/arch:AVX2" COMPILER_SUPPORTS_AVX2)
    check_cxx_compiler_flag("/arch:AVX" COMPILER_SUPPORTS_AVX)

    if(COMPILER_SUPPORTS_AVX2)
        set(RMATH_SIMD_FLAGS "/arch:AVX2")
    elseif(COMPILER_SUPPORTS_AVX)
        set(RMATH_SIMD_FLAGS "/arch:AVX")
    else()
        # SSE2 is enabled by default on x86_64 for MSVC
        set(RMATH_SIMD_FLAGS "")
    endif()
endif()

message(STATUS "RMath SIMD flags: ${RMATH_SIMD_FLAGS}")

# Interface library for the header-only math library
add_library(rmath INTERFACE)
target_include_directories(rmath INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)

# Apply SIMD flags to the interface target (仅当启用时透传)
if(RMATH_ENABLE_SIMD AND RMATH_SIMD_FLAGS)
    target_compile_options(rmath INTERFACE ${RMATH_SIMD_FLAGS})
endif()

# Test executable
add_executable(rmath_test test.cpp)
target_link_libraries(rmath_test PRIVATE rmath)

# Enable additional warnings
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
    target_compile_options(rmath_test PRIVATE -Wall -Wextra -Wpedantic)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    target_compile_options(rmath_test PRIVATE /W4)
endif()

# CTest integration
enable_testing()
add_test(NAME RMathTest COMMAND rmath_test)

# Install rules (header-only interface library)
include(GNUInstallDirs)
install(TARGETS rmath EXPORT RMathTargets)
install(FILES rmath.hpp vec.hpp mat.hpp range.hpp simd_utils.hpp math_functions.hpp
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT RMathTargets
        FILE RMathTargets.cmake
        NAMESPACE RMath::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/RMath)