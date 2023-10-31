function table = create_rom()
  
  nBits = 3;
  table = zeros(2^nBits, 1);
  
  pattern = getPattern("PLUS", 2, "0x10");
  bits = getBits("LD_D", "EN_D", "CR", "OE_RAM");
  table = addInstruction(getPattern("PLUS", 1, "x01x"), getBits("INC(D)", "VE", "LD_F"));
  
endfunction

function table = addInstruction(table, pattern, value)
  N = size(table, 1);
    
  % Pattern is a string, e.g. 0011 00xx x100, where x can be either 0 or 1.
  % Characters other than 0, 1 or x are removed from the pattern.
  % The pattern is padded (from the left) with zeros to match the number of
  % bits required to index the entire table (size N).
  pattern(~ismember(pattern, "01x")) = [];
  nBits = ceil(log(N)/log(2));
  if length(pattern) > nBits
    error("Pattern too long for table-size");
  endif
  pattern = [char('0' + zeros(1, nBits - length(pattern))), pattern];


  % Loop over entire address space and add "value" to the table where the 
  % address is a match to the pattern.
  for addr = 0:N-1
    if isMatch(addr, pattern)
      table(addr + 1) = value;
    endif
  endfor
endfunction

function result = isMatch(address, pattern)
  addressBits = dec2bin(address);
  if length(addressBits) > length(pattern)
    error("Lengths do not match");  
  endif
  
  diff = length(pattern) - length(addressBits);
  addressBits = [char('0' + zeros(1,  diff)), addressBits];
  
  for i = 1:length(pattern)
    if addressBits(i) ~= pattern(i) && pattern(i) ~= 'x'
      result = false;
      return;
    endif
  endfor
  
  result = true;
  return;
endfunction


function value = getBits(cmd, cycle, varargin)
  
  signals = {
    "RS0",
    "RS1",
    "RS2",
    "INC",
    "DEC",
    "EN_D",
    "EN_DP",
    "EN_IP",
    "EN_SP",
    "OE_RAM",
    "WE_RAM",
    "LD_D",
    "LD_IP",
    "LD_I",
    "LD_F",
    "CR"
  };
  
  
  value = 0;
  for i = 1:length(varargin)
    sig = varargin{i};
    idx = find(ismember(signals, sig)) - 1;
    value += 2^idx;
  endfor
  
endfunction

function pattern = getPattern(cmd, cycle, flags)
  
  commands = {
    "NOOP",
    "PLUS",
    "MINUS",
    "LEFT",
    "RIGHT",
    "IN",
    "OUT",
    "LOOP_START",
    "LOOP_END",
    "HLT",
    "ERR"
  };
  commandBits = 4;
  cycleBits = 3;
  
  if strcmp(cmd, "ANY")
    cmd = "xxxx";
  else
    cmd = dec2bin(find(ismember(commands, cmd)));
    cmd = [char('0' + zeros(1, commandBits - length(cmd))), cmd];
  endif
  
  cycle = dec2bin(cycle);
  cycle = [char('0' + zeros(1, cycleBits - length(cycle))), cycle];
  
  pattern = [cmd, cycle, flags];
  
endfunction  
  
