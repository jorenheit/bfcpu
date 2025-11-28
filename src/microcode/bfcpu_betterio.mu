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
  CLR_DP
  EN_SP

  OE_RAM
  WE_RAM
  EN_IN
  EN_OUT
  EN_V
  EN_A
  LD_FBI
  LD_FA

  EN_IP
  LD_IP
  EN_D
  LD_D
  CR
  CLR_K
  END  
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
  PROG_START    = 0x0a  
  PROG_END      = 0x0b  
  LOAD_SLOT     = 0x0c  
  INIT          = 0x0d
  INIT_FINISH   = 0x0e
  HALT          = 0x0f
}


[macros] {
  # Register Select  
  R_D  = RS0
  R_DP = RS1
  R_SP = RS0, RS1
  R_IP = RS2   
  R_LS = RS0, RS2

  # Load/store to RAM
  LOAD_D   = LD_D, OE_RAM
  STORE_D  = EN_D, WE_RAM
  LOAD_IP  = LD_IP, EN_SP, OE_RAM
  STORE_IP = EN_IP, EN_SP, WE_RAM

  # Set A and V flags
  SET_V  = EN_V, LD_FA
  SET_A  = EN_A, LD_FA
  SET_VA = EN_V, EN_A, LD_FA  
  CLR_VA = LD_FA  

  # Modify Data
  INC_D = INC, R_D, SET_V
  DEC_D = DEC, R_D, SET_V

  # Modify Data Pointer
  INC_DP = INC, R_DP, SET_A
  DEC_DP = DEC, R_DP, SET_A

  # Loops
  INC_SP = INC, R_SP
  DEC_SP = DEC, R_SP

  INC_LS = INC, R_LS
  DEC_LS = DEC, R_LS  

  # Move to next instruction
  NEXT = INC, R_IP, CR

  # RNG
  EN_RNG = EN_IN, EN_OUT  
}  

[microcode] {

  # Most opcodes do the same thing in cycle 0: load the new instruction into I and latch flags into FB  
  NOP:0:()                      -> LD_FBI
  PLUS:0:()                     -> LD_FBI
  MINUS:0:()                    -> LD_FBI
  LEFT:0:()                     -> LD_FBI
  RIGHT:0:()                    -> LD_FBI
  LOOP_START:0:()               -> LD_FBI
  LOOP_END:0:()                 -> LD_FBI
  INIT:0:()                     -> LD_FBI
  INIT_FINISH:0:()              -> LD_FBI
  LOAD_SLOT:0:()                -> LD_FBI
  PROG_START:0:()               -> LD_FBI
  PROG_END:0:()                 -> LD_FBI
  HALT:0:()                     -> LD_FBI


  PLUS:1:(A=0,S=0)              -> INC_D
  PLUS:2:(A=0,S=0)              -> NEXT
  PLUS:1:(A=1,S=0)              -> LOAD_D
  PLUS:2:(A=1,S=0)              -> INC_D
  PLUS:3:(A=1,S=0)              -> NEXT
  PLUS:1:(S=1)                  -> NEXT

  MINUS:1:(A=0,S=0)             -> DEC_D
  MINUS:2:(A=0,S=0)             -> NEXT
  MINUS:1:(A=1,S=0)             -> LOAD_D
  MINUS:2:(A=1,S=0)             -> DEC_D
  MINUS:3:(A=1,S=0)             -> NEXT
  MINUS:1:(S=1)                 -> NEXT

  LEFT:1:(V=0,S=0)              -> DEC_DP
  LEFT:2:(V=0,S=0)              -> NEXT
  LEFT:1:(V=1,S=0)              -> STORE_D
  LEFT:2:(V=1,S=0)              -> DEC_DP
  LEFT:3:(V=1,S=0)              -> NEXT
  LEFT:1:(S=1)                  -> NEXT

  RIGHT:1:(V=0,S=0)             -> INC_DP
  RIGHT:2:(V=0,S=0)             -> NEXT
  RIGHT:1:(V=1,S=0)             -> STORE_D
  RIGHT:2:(V=1,S=0)             -> INC_DP
  RIGHT:3:(V=1,S=0)             -> NEXT
  RIGHT:1:(S=1)                 -> NEXT
  
  LOOP_START:1:(A=0,Z=1,S=0)    -> INC_LS
  LOOP_START:2:(A=0,Z=1,S=0)    -> NEXT
  LOOP_START:1:(A=0,Z=0,S=0)    -> INC_SP
  LOOP_START:2:(A=0,Z=0,S=0)    -> STORE_IP
  LOOP_START:3:(A=0,Z=0,S=0)    -> NEXT
  LOOP_START:1:(A=1,S=0)        -> LOAD_D, CLR_VA, CR
  LOOP_START:1:(S=1)            -> INC_LS
  LOOP_START:2:(S=1)            -> NEXT

  LOOP_END:1:(A=0,Z=1,S=0)      -> DEC_SP
  LOOP_END:2:(A=0,Z=1,S=0)      -> NEXT
  LOOP_END:1:(A=0,Z=0,S=0)      -> LOAD_IP
  LOOP_END:2:(A=0,Z=0,S=0)      -> NEXT
  LOOP_END:1:(A=1,S=0)          -> LOAD_D, CLR_VA, CR
  LOOP_END:1:(S=1)              -> DEC_LS
  LOOP_END:2:(S=1)              -> NEXT

  OUT:0:(A=0)                   -> LD_FBI, EN_D
  OUT:0:(A=1)                   -> LD_FBI, OE_RAM   
  OUT:1:(S=1)                   -> NEXT
  OUT:1:(A=0,S=0)               -> EN_OUT, EN_D
  OUT:1:(A=1,S=0)               -> EN_OUT, OE_RAM
  OUT:2:(K=0,A=0,S=0)           -> EN_D, CR
  OUT:2:(K=0,A=1,S=0)           -> OE_RAM, CR  
  OUT:2:(K=1,S=0)               -> CLR_K, NEXT  

  # All "normal" states behave like any instruction @ cycle 0  
  IN:0:(A=0,V=0)                -> LD_FBI
  IN:0:(A=0,V=1)                -> LD_FBI
  IN:0:(A=1,V=0)                -> LD_FBI
  IN:1:(S=1)                    -> NEXT  

  # Special state (A=V=1) to indicate we're in a busy-loop, waiting for the IO peripheral -> keep EN_IN high  
  IN:0:(A=1,V=1,S=0)            -> EN_IN

  # If this is the first time (A and V not simultaneously 1), keep EN_IN asserted and set both A and V before resetting the CC
  IN:1:(A=0,V=0,S=0)            -> EN_IN, SET_VA
  IN:2:(A=0,V=0,S=0)            -> EN_IN, LD_FBI, CR  
  IN:1:(A=0,V=1,S=0)            -> EN_IN, SET_VA
  IN:2:(A=0,V=1,S=0)            -> EN_IN, LD_FBI, CR  
  IN:1:(A=1,V=0,S=0)            -> EN_IN, SET_VA
  IN:2:(A=1,V=0,S=0)            -> EN_IN, LD_FBI, CR  

  # All subsequent iterations: check K if peripheral is ready
  # If K = 0 -> keep EN_IN asserted
  IN:1:(K=0,A=1,V=1,S=0)        -> EN_IN, CR

  # If K = 1 -> Load value from the bus, set V and move on
  IN:1:(K=1,A=1,V=1,S=0)        -> LD_D, SET_V
  IN:2:(K=1,A=1,V=1,S=0)        -> CLR_K, LD_FBI, NEXT

  # Same algorithm for RNG  
  RAND:0:(A=1,V=1,S=0)          -> EN_RNG
  RAND:1:(A=0,V=0,S=0)          -> EN_RNG, SET_VA
  RAND:2:(A=0,V=0,S=0)          -> EN_RNG, LD_FBI, CR  
  RAND:1:(A=0,V=1,S=0)          -> EN_RNG, SET_VA
  RAND:2:(A=0,V=1,S=0)          -> EN_RNG, LD_FBI, CR  
  RAND:1:(A=1,V=0,S=0)          -> EN_RNG, SET_VA
  RAND:2:(A=1,V=0,S=0)          -> EN_RNG, LD_FBI, CR  
  RAND:1:(K=0,A=1,V=1,S=0)      -> EN_RNG, CR
  RAND:1:(K=1,A=1,V=1,S=0)      -> LD_D, SET_V
  RAND:2:(K=1,A=1,V=1,S=0)      -> CLR_K, LD_FBI, NEXT


  # HOUSEKEEPING   
  NOP:1:()                      -> NEXT
  HALT:1:(S=0)                  -> HLT
  HALT:2:(S=0)                  -> NEXT
  HALT:1:(S=1)                  -> NEXT  
  
  INIT:1:(K=0)                  -> CR # Wait for IO module to set K (bus is safe to use)  
  INIT:1:(K=1, Z=1)             -> STORE_D, INC, R_LS
  INIT:2:(K=1, Z=1)             -> LD_FBI, INC, R_DP
  INIT:3:(K=1, Z=1, S=1)        -> CR
  INIT:3:(K=1, Z=1, S=0)        -> NEXT

  INIT_FINISH:1:(K=1)           -> CLR_DP, CLR_K, NEXT

  LOAD_SLOT:1:(K=0)             -> CR
  LOAD_SLOT:1:(K=1)             -> LD_D, INC, R_LS
  LOAD_SLOT:2:(K=1)             -> CLR_K, NEXT  # write slot to D and go into skip mode

  PROG_START:1:(Z=0, S=1)       -> DEC, R_D
  PROG_START:1:(Z=1, S=1)       -> DEC, R_LS
  PROG_START:2:(S=1)            -> NEXT

  PROG_END:1:(S=0)              -> END, HLT
  PROG_END:1:(S=1)              -> NEXT  

  catch                         -> ERR, HLT
}



