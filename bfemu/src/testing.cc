#include <gtest/gtest.h>
#include <sstream>

#include "allmodules.h"
#include "computer.h"

TEST(PowerTest, OutputsAsExpected) {
  Power p;
  EXPECT_EQ(p.output(), 0b10);
}

TEST(PowerTest, NumberIOAsExpected) {
  Power p;
  EXPECT_EQ(p.numberOfOutputs(), 2);
  EXPECT_EQ(p.numberOfInputs(), 0);
}

TEST(ClockTest, HaltSignalStopsClock) {

  struct TestModule: Module {
    int value = 0;
    virtual void onClockRising() override {
      value = 42;
    };
    virtual int numberOfInputs() const override { return 0; }
    virtual int numberOfOutputs() const override { return 0; }
    
  };

  Module::init();
  Clock clc;
  TestModule test;
  clc.connectModule(test);
  clc.setHaltEnabled(true);

  EXPECT_EQ(test.value, 0);
  clc.pulse();
  EXPECT_EQ(test.value, 0);
  clc.setHaltEnabled(false);
  clc.pulse();
  EXPECT_EQ(test.value, 42);
}


TEST(RegisterTest, EnableOutput) {
  Module::init();
  Register reg;
  int const data = 0x1234;
  reg.setData(data);
  
  EXPECT_EQ(reg.output(Register::DATA_OUT), 0);
  reg.setOutputEnabled(true);
  EXPECT_EQ(reg.output(Register::DATA_OUT), data);
}

TEST(RegisterTest, LoadDataFromRegisterAIntoRegisterB) {
  Module::init();
  Register regA;
  Register regB;
  Clock clc;
  clc.connectModule(regA, regB);

  int const dataA = 0x1234;
  int const dataB = 0x4321;
  regA.setData(dataA);
  regB.setData(dataB);

  regA.setOutputEnabled(true);
  regB.setOutputEnabled(true);
  regB.connectInputTo(regA, Register::DATA_OUT, Register::DATA_IN);

  // First: without loadEnabled 
  clc.pulse();
  EXPECT_EQ(regB.output(Register::DATA_OUT), dataB);

  // Second: with loadEnabled
  regB.setLoadEnabled(true);
  clc.pulse();
  EXPECT_EQ(regB.output(Register::DATA_OUT), dataA);
}

TEST(ROMTest, OutputValuesByAddressFromRegister) {
  Module::init();
  Register regA; // address register
  ROM rom;

  regA.setOutputEnabled(true);
  rom.connectInputTo(regA, Register::DATA_OUT, RAM::ADDRESS_IN);

  
  unsigned char const romData[] = {42, 69, 112};
  rom.load(romData);

  for (int addr = 0; addr != 3; ++addr) {
    regA.setData(addr);
    EXPECT_EQ(rom.output(RAM::DATA_OUT), romData[addr]);
  }
}

TEST(RAMTest, OutputValuesByAddressFromRegister) {
  Module::init();
  Register regA; // address register
  RAM ram;

  ram.setOutputEnabled(true);
  regA.setOutputEnabled(true);
  ram.connectInputTo(regA, Register::DATA_OUT, RAM::ADDRESS_IN);

  
  unsigned char const ramData[] = {42, 69, 112};
  ram.load(ramData);

  for (int addr = 0; addr != 3; ++addr) {
    regA.setData(addr);
    EXPECT_EQ(ram.output(RAM::DATA_OUT), ramData[addr]);
  }
}

TEST(RAMTest, LoadValuesIntoRAM) {
  Module::init();
  Clock clc;
  Register regA; // address register
  Register regB; // value register
  RAM ram;

  clc.connectModule(regA, regB, ram);

  regA.setOutputEnabled(true);
  regB.setOutputEnabled(true);
  ram.setOutputEnabled(true);
  
  ram.connectInputTo(regA, Register::DATA_OUT, RAM::ADDRESS_IN);
  ram.connectInputTo(regB, Register::DATA_OUT_LOW, RAM::DATA_IN);
  
  unsigned char const initialRamData[] = {42, 69, 112};
  unsigned char const targetRamData[] = {24, 96, 211};
  ram.load(initialRamData);

  // First: without write-enable
  for (int addr = 0; addr != 3; ++addr) {
    regA.setData(addr);
    regB.setData(targetRamData[addr]);

    clc.pulse();
    EXPECT_EQ(ram.output(RAM::DATA_OUT), initialRamData[addr]);
  }

  // Next: with write-enable
  ram.setWriteEnabled(true);
  for (int addr = 0; addr != 3; ++addr) {
    regA.setData(addr);
    regB.setData(targetRamData[addr]);

    clc.pulse();
    EXPECT_EQ(ram.output(RAM::DATA_OUT), targetRamData[addr]);
  }
}

TEST(BusTest, ConnectSingleRegisterToBus) {
  Module::init();
  Bus bus;
  Register regA;
  regA.setOutputEnabled(true);

  int const data = 0x1234;
  regA.setData(data);
  
  bus.connectInputTo(regA, Register::DATA_OUT, Bus::FULL);
  EXPECT_EQ(bus.output(Bus::FULL), data);
}

TEST(BusTest, SendDataOverBusBetweenRegisters) {
  Module::init();
  Clock clc;
  Bus bus;
  Register regA;
  Register regB;
  clc.connectModule(regA, regB);

  bus.connectInputTo(regA, Register::DATA_OUT, Bus::FULL);
  regB.connectInputTo(bus, Bus::FULL, Register::DATA_IN);

  int const data = 0x1234;
  regA.setData(data);
  regA.setOutputEnabled(true);
  regB.setLoadEnabled(true);
  regB.setOutputEnabled(true);

  clc.pulse();
  EXPECT_EQ(regB.output(Register::DATA_OUT), data);
}

TEST(BinaryCounterTest, IncrementValue) {
  Module::init();
  Clock clc;
  BinaryCounter bc;
  clc.connectModule(bc);

  int const data = 0x1234;
  bc.setData(data);
  bc.setOutputEnabled(true);

  EXPECT_EQ(bc.output(BinaryCounter::DATA_OUT), data);
  clc.pulse();
  EXPECT_EQ(bc.output(BinaryCounter::DATA_OUT), data);
  bc.setCountEnabled(true);
  clc.pulse();
  EXPECT_EQ(bc.output(BinaryCounter::DATA_OUT), data + 1);
  EXPECT_EQ(bc.outputByIndex(BinaryCounter::CA), 0);
}

TEST(BinaryCounterTest, DecrementValue) {
  Module::init();
  Clock clc;
  BinaryCounter bc;
  clc.connectModule(bc);

  int const data = 0x6969;
  bc.setData(data);
  bc.setOutputEnabled(true);
  bc.setDecEnabled(true);
  
  EXPECT_EQ(bc.output(BinaryCounter::DATA_OUT), data);
  clc.pulse();
  EXPECT_EQ(bc.output(BinaryCounter::DATA_OUT), data);
  bc.setCountEnabled(true);
  clc.pulse();
  EXPECT_EQ(bc.output(BinaryCounter::DATA_OUT), data - 1);
  EXPECT_EQ(bc.outputByIndex(BinaryCounter::CA), 0);
}

TEST(BinaryCounterTest, Overflow) {
  Module::init();
  Clock clc;
  BinaryCounter bc;
  clc.connectModule(bc);

  bc.setData(0xffff);
  bc.setOutputEnabled(true);
  bc.setCountEnabled(true);

  clc.pulse();
  EXPECT_EQ(bc.output(BinaryCounter::DATA_OUT), 0);
  EXPECT_EQ(bc.outputByIndex(BinaryCounter::CA), true);
}

TEST(BinaryCounterTest, Underflow) {
  Module::init();
  Clock clc;
  BinaryCounter bc;
  clc.connectModule(bc);

  bc.setData(0);
  bc.setOutputEnabled(true);
  bc.setCountEnabled(true);
  bc.setDecEnabled(true);

  clc.pulse();
  EXPECT_EQ(bc.output(BinaryCounter::DATA_OUT), 0xffff);
  EXPECT_EQ(bc.outputByIndex(BinaryCounter::CA), true);
}


TEST(BinaryCounterTest, LoadDataFromBinaryCounterAIntoBinaryCounterB) {
  Module::init();
  BinaryCounter bcA;
  BinaryCounter bcB;
  Clock clc;
  clc.connectModule(bcA, bcB);

  int const dataA = 0x4242;
  int const dataB = 0x6969;
  bcA.setData(dataA);
  bcB.setData(dataB);

  bcA.setOutputEnabled(true);
  bcB.setOutputEnabled(true);
  bcB.connectInputTo(bcA, BinaryCounter::DATA_OUT, BinaryCounter::DATA_IN);

  // First: without loadEnabled 
  clc.pulse();
  EXPECT_EQ(bcB.output(BinaryCounter::DATA_OUT), dataB);

  // Second: with loadEnabled
  bcB.setLoadEnabled(true);
  clc.pulse();
  EXPECT_EQ(bcB.output(BinaryCounter::DATA_OUT), dataA);
}


TEST(ScreenTest, OutputCharactersToStream) {
  Module::init();
  Register reg;

  std::ostringstream oss;
  Screen scr(oss);
  
  scr.connectInputTo(reg, Register::DATA_OUT_LOW, Screen::DATA_IN);
  reg.setOutputEnabled(true);
  
  reg.setData('a');
  scr.refresh();
  reg.setData('b');
  scr.refresh();

  EXPECT_EQ(oss.str(), "ab");
}

TEST_F(Computer, Increment)
{
  unsigned char memory1[] = {/* stack: */ 0, 0, 0, 0, /* tape:  */ 'a', 'b', 'c', 'd'};
  unsigned char memory2[] = {/* stack: */ 0, 0, 0, 0, /* tape:  */ 0, 0, 0, 0};
  
  ram1.load(memory1);
  ram2.load(memory2);

  // Run
  unsigned char program[] = {
    Decoder::PROG_START,
    Decoder::PLUS,
    Decoder::PLUS,
    Decoder::PLUS,
    Decoder::HLT};
  
  rom.load(program);

  while (!clc.haltEnabled())
    clc.pulse();

  EXPECT_EQ(dataRegister.peek(BinaryCounter::DATA_OUT), 'a' + 3);
}

TEST_F(Computer, Decrement)
{
  unsigned char memory1[] = {/* stack: */ 0, 0, 0, 0, /* tape:  */ 'a', 'b', 'c', 'd'};
  unsigned char memory2[] = {/* stack: */ 0, 0, 0, 0, /* tape:  */ 0, 0, 0, 0};
  
  ram1.load(memory1);
  ram2.load(memory2);

  // Run
  unsigned char program[] = {
    Decoder::PROG_START,
    Decoder::MINUS,
    Decoder::MINUS,
    Decoder::MINUS,
    Decoder::HLT};
  rom.load(program);

  while (!clc.haltEnabled())
    clc.pulse();

  EXPECT_EQ(dataRegister.peek(BinaryCounter::DATA_OUT), 'a' - 3);
}

TEST_F(Computer, MovePointerRightWithoutChangedValue)
{
  unsigned char memory1[] = {/* stack: */ 0, 0, 0, 0, /* tape:  */ 'a', 'b', 'c', 'd'};
  unsigned char memory2[] = {/* stack: */ 0, 0, 0, 0, /* tape:  */ 0, 0, 0, 0};
  
  ram1.load(memory1);
  ram2.load(memory2);

  // Run
  unsigned char program[] = {
    Decoder::PROG_START,
    Decoder::RIGHT,
    Decoder::HLT
  };
  rom.load(program);

  while (!clc.haltEnabled())
    clc.pulse();

  EXPECT_EQ(dataPointerRegister.peek(Register::DATA_OUT), STACK_SIZE + 1);
  EXPECT_EQ(dataRegister.peek(Register::DATA_OUT), 'b');
  EXPECT_EQ(!!(flagRegister.peek(Register::DATA_OUT) & 1), true); // A flag
  EXPECT_EQ(!!(flagRegister.peek(Register::DATA_OUT) & 2), false); // V flag
}

TEST_F(Computer, MovePointerLeftAndDecrement)
{
  unsigned char memory1[] = {/* stack: */ 0, 0, 0, 0, /* tape:  */ 'a', 'b', 'c', 'd'};
  unsigned char memory2[] = {/* stack: */ 0, 0, 0, 0, /* tape:  */ 0, 0, 0, 0};
  
  ram1.load(memory1);
  ram2.load(memory2);

  dataPointerRegister.setData(STACK_SIZE + 3);
  dataRegister.setData(0);
  instructionPointerRegister.setData(0);
  flagRegister.setData(Decoder::A);

  
  // Run
  unsigned char program[] = {Decoder::LEFT, Decoder::LEFT, Decoder::LEFT, Decoder::MINUS}; // move from 'd' to 'a' and decrement
  rom.load(program);

  for (int i = 0; i != sizeof(program); ++i){
    clc.pulse();
  }

  EXPECT_EQ(dataPointerRegister.peek(Register::DATA_OUT), STACK_SIZE);
  EXPECT_EQ(dataRegister.peek(Register::DATA_OUT), 'a');
  EXPECT_EQ(!!(flagRegister.peek(Register::DATA_OUT) & Decoder::A), true);
  EXPECT_EQ(!!(flagRegister.peek(Register::DATA_OUT) & Decoder::V), false);

  clc.pulse();
  EXPECT_EQ(dataRegister.peek(Register::DATA_OUT), 'a' - 1);
  EXPECT_EQ(!!(flagRegister.peek(Register::DATA_OUT) & Decoder::A), false);
  EXPECT_EQ(!!(flagRegister.peek(Register::DATA_OUT) & Decoder::V), true);
}
