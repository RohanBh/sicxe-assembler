//
// Created by rohan on 31/3/18.
//

#include "gtest/gtest.h"
#include "PassOne.h"
#include "PassTwo.h"
#include "Commons.h"

const std::string PROJECT_ROOT = "../../../";
const std::string TEST_ROOT = PROJECT_ROOT + "/assembler_tests/basic_assembler_tests/";

bool compareFiles(const std::string &p1, const std::string &p2);

TEST(basic_check, test_optab) {
    EXPECT_EQ(initOpTab(PROJECT_ROOT), 63);
    EXPECT_EQ(initSymTab(PROJECT_ROOT), 9);
}

TEST(basic_check, test_even_odd_separator) {
    OPTAB.clear();
    SYMTAB.clear();
    initOpTab(PROJECT_ROOT);
    initSymTab(PROJECT_ROOT);

    std::string intermediate = createIntermediate(TEST_ROOT + "/EvenOddSeparator.asmb");
    std::string objectFile = createObjectFile(intermediate);
    EXPECT_TRUE(compareFiles(objectFile, TEST_ROOT + "EvenOddSeparator.txt"));
}

TEST(basic_check, test_ll_beck_example) {
    OPTAB.clear();
    SYMTAB.clear();
    initOpTab(PROJECT_ROOT);
    initSymTab(PROJECT_ROOT);

    std::string intermediate = createIntermediate(TEST_ROOT + "ll_beck_example.asmb");
    std::string objectFile = createObjectFile(intermediate);
    EXPECT_TRUE(compareFiles(objectFile, TEST_ROOT + "ll_beck_example.txt"));
}

bool compareFiles(const std::string &p1, const std::string &p2) {
    std::ifstream f1(p1, std::ifstream::binary | std::ifstream::ate);
    std::ifstream f2(p2, std::ifstream::binary | std::ifstream::ate);

    if (f1.fail() || f2.fail()) {
        std::cerr << "Can not open test file\n";
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
