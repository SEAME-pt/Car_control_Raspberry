#include <gtest/gtest.h>
#include "carControl.h"

// Test that parsingArgv parses --manual=false and --can=can1 correctly
TEST(ParsingTest, ParsesManualFalseAndCan1) {
    t_carControl cfg;
    cfg.canInterface = "can0";
    cfg.manual = true;
    cfg.exit = false;

    char* argv[] = {
        (char*)"prog",
        (char*)"--manual=false",
        (char*)"--can=can1"
    };
    int argc = 3;

    int ret = parsingArgv(argc, argv, &cfg);

    EXPECT_EQ(ret, 1);                    // parsing succeeded
    EXPECT_FALSE(cfg.manual);             // manual=false parsed
    EXPECT_EQ(cfg.canInterface, "can1");  // can interface set
    EXPECT_FALSE(cfg.exit);               // not an early-exit option
}

// Testing if parsingArgv parses --manual=true and --can=can0 correctly
TEST(ParsingTest, ParsesManualTrueAndCan0) {
    t_carControl cfg;
    cfg.canInterface = "can1";
    cfg.manual = true;
    cfg.exit = false;

    char* argv[] = {
        (char*)"prog",
        (char*)"--manual=true",
        (char*)"--can=can0"
    };
    int argc = 3;

    int ret = parsingArgv(argc, argv, &cfg);

    EXPECT_EQ(ret, 1);                    // parsing succeeded
    EXPECT_TRUE(cfg.manual);             // manual=true parsed
    EXPECT_EQ(cfg.canInterface, "can0");  // can interface set
    EXPECT_FALSE(cfg.exit);               // not an early-exit option
}

// Testing if parsingArgv gives error with badd boolean value
TEST(ParsingTest, ParsesManualBadValueAndCan0) {
    t_carControl cfg;
    cfg.canInterface = "can1";
    cfg.manual = true;
    cfg.exit = false;

    char* argv[] = {
        (char*)"prog",
        (char*)"--manual=badvalue",
        (char*)"--can=can0"
    };
    int argc = 3;

    int ret = parsingArgv(argc, argv, &cfg);

    EXPECT_EQ(ret, 1);                    // parsing succeeded
    EXPECT_TRUE(cfg.manual);             // manual=true parsed
    EXPECT_EQ(cfg.canInterface, "can0");  // can interface set
    EXPECT_FALSE(cfg.exit);               // not an early-exit option
}