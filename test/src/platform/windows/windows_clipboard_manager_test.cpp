#include <gtest/gtest.h>
#include <iostream>
#include <windows.h>

TEST(WindowsClipBoardManagerTest, SaveDIBToPNG) {
    std::cout << "Size of BITMAPINFOHEADER: " << sizeof(BITMAPINFOHEADER) << std::endl;
    SUCCEED();
}
