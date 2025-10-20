# This file can be compiled with Mugen, see https://github.com/jorenheit/mugen.

[rom] { 8192 x 8 x 3 }

[address] {
  cycle:   3
  opcode:  4
  flags:   K, V, A, S, Z
}

[signals] {
  HLT
  RS0
  RS1
  RS2
  INC
  DEC
  DPR
  EN_SP

  OE_RAM
  WE_RAM
  EN_IN
  EN_OUT
  SET_V
  SET_A
  LD_FBI
  LD_FA

  EN_IP
  LD_IP
  EN_D
  LD_D
  CR
  CLR_K
  EMPTY
  ERR  
}

[opcodes] {
  NOP           = 0x00
  PLUS          = 0x01
  MINUS         = 0x02
  LEFT          = 0x03
  RIGHT         = 0x04
  IN            = 0x05
  OUT           = 0x06
  LOOP_START    = 0x07
  LOOP_END      = 0x08
  RAND          = 0x09
  WAIT_EXT      = 0x0c  
  INIT          = 0x0d
  HOME          = 0x0e
  HALT          = 0x0f
}

# Resgister Select:
# D  = RS0
# DP = RS1
# SP = RS0, RS1
# IP = RS2
# LS = RS0, RS2

[microcode] {

  NOP:0:xxxxx           -> LD_FBI
  PLUS:0:xxxxx          -> LD_FBI
  MINUS:0:xxxxx         -> LD_FBI
  LEFT:0:xxxxx          -> LD_FBI
  RIGHT:0:xxxxx         -> LD_FBI
  IN:0:xxxxx            -> LD_FBI
  OUT:0:xx0xx           -> LD_FBI, EN_D     # when OUT is spinning, EN_D must be kept high
  OUT:0:xx1xx           -> LD_FBI, OE_RAM   # OE_RAM in this case, for the same reason
  LOOP_START:0:xxxxx    -> LD_FBI
  LOOP_END:0:xxxxx      -> LD_FBI
  RAND:0:xxxxx          -> LD_FBI
  WAIT_EXT:0:xxxxx      -> LD_FBI
  INIT:0:xxxxx          -> LD_FBI
  HOME:0:xxxxx          -> LD_FBI
  HALT:0:xxxxx          -> LD_FBI

  PLUS:1:xx00x          -> INC, RS0, SET_V, LD_FA
  PLUS:2:xx00x          -> INC, RS2, CR
  PLUS:1:xx10x          -> LD_D, OE_RAM
  PLUS:2:xx10x          -> INC, RS0, SET_V, LD_FA  
  PLUS:3:xx10x          -> INC, RS2, CR
  PLUS:1:xxx1x          -> INC, RS2, CR

  MINUS:1:xx00x         -> DEC, RS0, SET_V, LD_FA
  MINUS:2:xx00x         -> INC, RS2, CR
  MINUS:1:xx10x         -> LD_D, OE_RAM
  MINUS:2:xx10x         -> DEC, RS0, SET_V, LD_FA
  MINUS:3:xx10x         -> INC, RS2, CR
  MINUS:1:xxx1x         -> INC, RS2, CR

  LEFT:1:x0x0x          -> DEC, RS1, SET_A, LD_FA  
  LEFT:2:x0x0x          -> INC, RS2, CR
  LEFT:1:x1x0x          -> EN_D, WE_RAM
  LEFT:2:x1x0x          -> DEC, RS1, SET_A, LD_FA 
  LEFT:3:x1x0x          -> INC, RS2, CR
  LEFT:1:xxx1x          -> INC, RS2, CR

  RIGHT:1:x0x0x         -> INC, RS1, SET_A, LD_FA
  RIGHT:2:x0x0x         -> INC, RS2, CR
  RIGHT:1:x1x0x         -> EN_D, WE_RAM
  RIGHT:2:x1x0x         -> INC, RS1, SET_A, LD_FA
  RIGHT:3:x1x0x         -> INC, RS2, CR
  RIGHT:1:xxx1x         -> INC, RS2, CR

  LOOP_START:1:xx001    -> INC, RS0, RS2
  LOOP_START:2:xx001    -> INC, RS2, CR
  LOOP_START:1:xx000    -> INC, RS0, RS1
  LOOP_START:2:xx000    -> WE_RAM, EN_SP, EN_IP
  LOOP_START:3:xx000    -> INC, RS2, CR
  LOOP_START:1:xx10x    -> OE_RAM, LD_D, LD_FA, CR
  LOOP_START:1:xxx1x    -> INC, RS0, RS2
  LOOP_START:2:xxx1x    -> INC, RS2, CR

  LOOP_END:1:xx001      -> DEC, RS0, RS1
  LOOP_END:2:xx001      -> INC, RS2, CR
  LOOP_END:1:xx000      -> EN_SP, OE_RAM, LD_IP
  LOOP_END:2:xx000      -> INC, RS2, CR
  LOOP_END:1:xx10x      -> OE_RAM, LD_D, LD_FA, CR
  LOOP_END:1:xxx1x      -> DEC, RS0, RS2
  LOOP_END:2:xxx1x      -> INC, RS2, CR

  OUT:1:xxx1x           -> INC, RS2, CR
  OUT:1:xx00x           -> EN_OUT, EN_D
  OUT:1:xx10x           -> EN_OUT, OE_RAM
  OUT:2:0x00x           -> EN_D, CR
  OUT:2:0x10x           -> OE_RAM, CR  
  OUT:2:1xx0x           -> CLR_K, INC, RS2, CR  

  IN:1:xxx1x            -> INC, RS2, CR  
  IN:1:xxx0x            -> EN_IN
  IN:2:0xx0x            -> CR
  IN:2:1xx0x            -> LD_D, SET_V, LD_FA
  IN:3:1xx0x            -> CLR_K, INC, RS2, CR

  RAND:1:xxx1x          -> INC, RS2, CR  
  RAND:1:0xx0x          -> EN_IN, EN_OUT
  RAND:2:0xx0x          -> CR
  RAND:1:1xx0x          -> LD_D, SET_V, LD_FA
  RAND:2:1xx0x          -> CLR_K, INC, RS2, CR

  WAIT_EXT:1:0xxxx      -> CR
  WAIT_EXT:1:1xxxx      -> CLR_K, INC, RS2, CR

  INIT:1:xxxx1          -> EN_D, WE_RAM, INC, RS0, RS2
  INIT:2:xxxx1          -> LD_FBI, INC, RS1
  INIT:3:xxx01          -> INC, RS2, CR
  INIT:3:xxx11          -> CR
  
  NOP:1:xxxxx           -> INC, RS2, CR
  HALT:1:xxxxx          -> HLT
  HALT:2:xxxxx          -> INC, RS2, CR

  HOME:1:xxxxx          -> DPR, INC, RS2, CR

  catch                 -> ERR, HLT
}



