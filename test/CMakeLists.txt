# 测试项目的 CMakeLists.txt

# 查找 GTest
find_package(GTest REQUIRED)

# 创建测试可执行文件
add_executable(ClipBridgeTests
    src/main.cpp
    src/clipboard_manager_test.cpp
)

# 链接头文件目录
target_include_directories(ClipBridgeTests PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/../include"
    "${GTEST_INCLUDE_DIRS}"
)

# 链接需要的库
target_link_libraries(ClipBridgeTests PRIVATE
    ClipBridgeCore
    GTest::gtest
    GTest::gtest_main
)

# 设置 C++ 标准
target_compile_features(ClipBridgeTests PRIVATE cxx_std_20)

if(MSVC)
    target_compile_definitions(ClipBridgeTests PRIVATE _UNICODE UNICODE)
endif()

# 启用测试
enable_testing()
include(GoogleTest)
gtest_discover_tests(ClipBridgeTests)
