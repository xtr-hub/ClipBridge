#include <gtest/gtest.h>
#include <iostream>
#include <windows.h>
#include "core/path_processer.hpp"

TEST(PathProcesserTest, DefaultPath) {
    PathProcesser pp;
    std::cout << "DEFAULT_PATH: " << pp.default_path("test") << std::endl;
    SUCCEED();
}
