//
// Created by rohan on 31/3/18.
//

#include "gtest/gtest.h"
#include "PassOne.h"
#include "PassTwo.h"
#include <iostream>
#include <fstream>

const std::string RELATIVE_PATH = "../assembler_tests/basic_assembler_tests/";

bool compareFiles(const std::string& p1, const std::string& p2);

TEST(basic_check, test_file_equal) {
    std::string intermediate = PassOne::run(RELATIVE_PATH + "EvenOddSeparator.asm");
    std::string objectFile = PassTwo::run(intermediate);

    std::string fullPath = realpath("EvenOddSeparator.txt", NULL);
    EXPECT_TRUE(compareFiles(objectFile, fullPath));
}

bool compareFiles(const std::string &p1, const std::string &p2) {
    std::ifstream f1(p1, std::ifstream::binary|std::ifstream::ate);
    std::ifstream f2(p2, std::ifstream::binary|std::ifstream::ate);

    if (f1.fail() || f2.fail()) {
        return false; //file problem
    }

    if (f1.tellg() != f2.tellg()) {
        return false; //size mismatch
    }

    //seek back to beginning and use std::equal to compare contents
    f1.seekg(0, std::ifstream::beg);
    f2.seekg(0, std::ifstream::beg);
    return std::equal(std::istreambuf_iterator<char>(f1.rdbuf()),
                      std::istreambuf_iterator<char>(),
                      std::istreambuf_iterator<char>(f2.rdbuf()));
}
