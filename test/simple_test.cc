#include <iostream>
#include "gtest/gtest.h"

TEST(simple, first) { EXPECT_EQ(1 + 1, 2); }

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
/* vim: set ts=2 sw=2:  */
