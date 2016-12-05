#include <iostream>
#include "fixed_point_compiler.h"
#include "gtest/gtest.h"

TEST(simple, first) { EXPECT_EQ(1 + 1, 2); }

TEST(simple, mul) {
  Program prog;
  VarR x = prog.load("x", Range(0, 100), Scale(4), "x");
  VarR y = prog.load("y", Range(0, 1024), Scale(3), "y");
  VarR z = prog.mul(x, y, "z");
  environment env;
  env["x"] = 1 << 2;   /* 0.25 */
  env["y"] = 100 << 3; /* 100 */
  int64_t actual = prog.eval(env);
  double value = z->scale().value_of(actual);
  EXPECT_EQ(value, 0.25 * 100);
}

TEST(simple, add) {
  Program prog;
  VarR x = prog.load("x", Range(0, 100), Scale(4), "x");
  VarR y = prog.load("y", Range(0, 1024), Scale(3), "y");
  VarR z = prog.add(x, y, "z");
  environment env;
  env["x"] = 1 << 2;   /* 0.25 */
  env["y"] = 100 << 3; /* 100 */
  int64_t actual = prog.eval(env);
  double value = z->scale().value_of(actual);
  EXPECT_EQ(value, 0.25 + 100);
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
/* vim: set ts=2 sw=2:  */
