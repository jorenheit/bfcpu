#include <gtest/gtest.h>
#include "computer.h"

TEST_F(Computer, Init) {
  reset();
  load("");
  run();

  int dp = dataPointerRegister.value();
  EXPECT_EQ(dp, STACK_SIZE);
  for (int i = 0; i != ram1.size(); ++i) {
    EXPECT_EQ(ram1.at(i), 0);
    EXPECT_EQ(ram2.at(i), 0);
  }
}

TEST_F(Computer, Plus) {
  reset();
  load("+");
  run();
  int flags = flagRegister.value();

  EXPECT_EQ(dataPointerRegister.relativeValue(), 0);
  EXPECT_EQ(dataRegister.value(), 1);
  EXPECT_EQ(dataRegister.zero(), false);
  EXPECT_EQ(dataRegister.carry(), false);
  EXPECT_EQ(!!(flags & 0b01), false);   // A
  EXPECT_EQ(!!(flags & 0b10), true);    // V
  
  EXPECT_EQ(ram1.at(STACK_SIZE), 0);
}

TEST_F(Computer, PlusOverflow) {
  reset();
  load(std::string(256, '+'));
  run();
  int flags = flagRegister.value();

  EXPECT_EQ(dataPointerRegister.relativeValue(), 0);
  EXPECT_EQ(dataRegister.value(), 0);
  EXPECT_EQ(dataRegister.carry(), true);
  EXPECT_EQ(dataRegister.zero(), true);
  EXPECT_EQ(!!(flags & 0b01), false);   // A
  EXPECT_EQ(!!(flags & 0b10), true);    // V
  EXPECT_EQ(ram1.at(STACK_SIZE), 0);
}

TEST_F(Computer, Minus) {
  reset();
  load("+-");
  run();
  int flags = flagRegister.value();

  EXPECT_EQ(dataPointerRegister.relativeValue(), 0);
  EXPECT_EQ(dataRegister.value(), 0);
  EXPECT_EQ(dataRegister.carry(), false);
  EXPECT_EQ(dataRegister.zero(), true);
  EXPECT_EQ(!!(flags & 0b01), false);   // A
  EXPECT_EQ(!!(flags & 0b10), true);    // V
  EXPECT_EQ(ram1.at(STACK_SIZE), 0);
}


TEST_F(Computer, MinusUnderflow) {
  reset();
  load("-");
  run();
  int flags = flagRegister.value();

  EXPECT_EQ(dataPointerRegister.relativeValue(), 0);
  EXPECT_EQ(dataRegister.value(), 0xff);
  EXPECT_EQ(dataRegister.carry(), true);
  EXPECT_EQ(dataRegister.zero(), false);
  EXPECT_EQ(!!(flags & 0b01), false);   // A
  EXPECT_EQ(!!(flags & 0b10), true);    // V
  EXPECT_EQ(ram1.at(STACK_SIZE), 0);
}

TEST_F(Computer, Right) {
  reset();
  load(">");
  run();

  int flags = flagRegister.value();
  
  EXPECT_EQ(dataPointerRegister.relativeValue(), 1);
  EXPECT_EQ(!!(flags & 0b01), true);   // A
  EXPECT_EQ(!!(flags & 0b10), false);  // V
}

TEST_F(Computer, Left) {
  reset();
  load("<");
  run();

  int flags = flagRegister.value();
  
  EXPECT_EQ(dataPointerRegister.relativeValue(), -1);
  EXPECT_EQ(!!(flags & 0b01), true);   // A
  EXPECT_EQ(!!(flags & 0b10), false);  // V
}

TEST_F(Computer, PlusAndStore) {
  reset();
  load("+>");
  run();
  int flags = flagRegister.value();

  EXPECT_EQ(dataPointerRegister.relativeValue(), 1);
  EXPECT_EQ(dataRegister.value(), 1);
  EXPECT_EQ(dataRegister.zero(), false);
  EXPECT_EQ(dataRegister.carry(), false);
  EXPECT_EQ(!!(flags & 0b01), true);   // A
  EXPECT_EQ(!!(flags & 0b10), false);    // V
  
  EXPECT_EQ(ram1.at(STACK_SIZE), 1);
}

TEST_F(Computer, LoopStartZero) {
  reset();
  load("+-[>");
  run();

  int flags = flagRegister.value();
  EXPECT_EQ(dataPointerRegister.relativeValue(), 0);
  EXPECT_EQ(dataRegister.value(), 0);
  EXPECT_EQ(loopRegister.value(), 1);
  EXPECT_EQ(!!(flags & 0b01), false);   // A
  EXPECT_EQ(!!(flags & 0b10), true);   // V
  EXPECT_EQ(stackPointerRegister.value(), 0);
  EXPECT_EQ(ram1.at(STACK_SIZE), 0);
}

TEST_F(Computer, LoopStartNonZero) {
  reset();
  load("+[>");
  run();

  int flags = flagRegister.value();
  EXPECT_EQ(dataPointerRegister.relativeValue(), 1);
  EXPECT_EQ(dataRegister.value(), 1);
  EXPECT_EQ(loopRegister.value(), 0);
  EXPECT_EQ(!!(flags & 0b01), true);   // A
  EXPECT_EQ(!!(flags & 0b10), false);   // V
  EXPECT_EQ(stackPointerRegister.value(), 1);
  EXPECT_EQ(ram1.at(STACK_SIZE), 1);
}

TEST_F(Computer, MultiplicationLoop) {
  reset();
  load("+++[>++<-]>+-"); // 3 * 2
  run();

  int flags = flagRegister.value();
  EXPECT_EQ(dataPointerRegister.relativeValue(), 1);
  EXPECT_EQ(dataRegister.value(), 6);
  EXPECT_EQ(loopRegister.value(), 0);
  EXPECT_EQ(!!(flags & 0b01), false);   // A
  EXPECT_EQ(!!(flags & 0b10), true);   // V
  EXPECT_EQ(stackPointerRegister.value(), 0);
  EXPECT_EQ(ram1.at(STACK_SIZE + 1), 6);
}

TEST_F(Computer, DoubleMultiplicationLoops) {
  reset();
  load("++++[>++<-]>[>+++<-]>+-"); // 4 * 2 * 3
  run();

  int flags = flagRegister.value();
  EXPECT_EQ(dataPointerRegister.relativeValue(), 2);
  EXPECT_EQ(dataRegister.value(), 24);
  EXPECT_EQ(loopRegister.value(), 0);
  EXPECT_EQ(!!(flags & 0b01), false);   // A
  EXPECT_EQ(!!(flags & 0b10), true);   // V
  EXPECT_EQ(stackPointerRegister.value(), 0);
  EXPECT_EQ(ram1.at(STACK_SIZE + 2), 24);
}
