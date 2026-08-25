cmake_minimum_required(VERSION 3.14)
project(algo-lib VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ============================================================================
# 核心：header-only 接口库
# 使用方式（第三方项目）：
#   add_subdirectory(algo-lib)  或  target_link_libraries(app PRIVATE algo_lib)
# 之后即可 #include "algo/ds/fenwick.hpp" 等
# ============================================================================
add_library(algo_lib INTERFACE)
target_include_directories(algo_lib INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)
target_compile_features(algo_lib INTERFACE cxx_std_17)

# 头文件含中文注释（UTF-8 无 BOM），MSVC 需显式指定源字符集，
# 否则在中文代码页(936)下会报 C4819 / 解析错误。
target_compile_options(algo_lib INTERFACE $<$<CXX_COMPILER_ID:MSVC>:/utf-8>)

# 可选：构建示例与冒烟测试（默认开启）
option(ALGO_LIB_BUILD_EXAMPLES "Build demo and smoke test" ON)

if(ALGO_LIB_BUILD_EXAMPLES)
    add_executable(demo examples/demo.cpp)
    target_link_libraries(demo PRIVATE algo_lib)
    target_compile_options(demo PRIVATE
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra>)

    # smoke 测试：包含全部头文件 + 少量正确性断言，兼作"全库可编译"检查
    add_executable(smoke tests/smoke.cpp)
    target_link_libraries(smoke PRIVATE algo_lib)
    target_compile_options(smoke PRIVATE
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra>)

    enable_testing()
    add_test(NAME smoke COMMAND smoke)
endif()

# 安装：头文件安装到 include/ 目录
install(DIRECTORY include/ DESTINATION include)
