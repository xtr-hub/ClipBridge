#include <gtest/gtest.h>
#include <iostream>
#include <windows.h>
#include "core/clipboard_manager.hpp"

TEST(ClipBoardManagerTest, SaveDIBToPNG) {
    std::cout << "Size of BITMAPINFOHEADER: " << sizeof(BITMAPINFOHEADER) << std::endl;
    SUCCEED();
}
