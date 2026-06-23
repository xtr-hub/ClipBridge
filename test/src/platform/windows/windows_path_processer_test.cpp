#include <gtest/gtest.h>
#include <iostream>
#include <windows.h>
#include "windows_path_processer.hpp"

TEST(WindowsPathProcesserTest, DefaultPath){
    WindowsPathProcesser wpp;
    std::cout << "DEFAULT_PATH: " << wpp.default_path("test") << std::endl;
    SUCCEED();
}