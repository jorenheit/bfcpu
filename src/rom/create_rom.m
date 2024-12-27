function create_rom(filename_base)
  
  % D  = 000 - none
  % DP = 001 - RS0
  % SP = 010 - RS1
  % F  = 011 - RS0 | RS1
  % I  = 100 - RS2
  % IP = 101 - RS0 | RS2
  % LS = 110 - RS1 | RS2
  
  table = {
  
  {"ANY",   0, "xxxx"}, {"EN_IP", "LD_I"};
  
  {"PLUS",  1, "x01x"}, {"INC", "VE", "LD_F"};
  {"PLUS",  2, "x01x"}, {"INC", "RS0", "RS2", "CR"};
  {"PLUS",  1, "x11x"}, {"EN_DP", "LD_D", "OE_RAM", "LD_F", "CR"};
  {"PLUS",  1, "xx0x"}, {"INC", "RS0", "RS2", "CR"};
  
  {"MINUS", 1, "x01x"}, {"DEC", "VE", "LD_F"};
  {"MINUS", 2, "x01x"}, {"INC", "RS0", "RS2", "CR"};
  {"MINUS", 1, "x11x"}, {"EN_DP", "LD_D", "OE_RAM", "LD_F", "CR"}; 
  {"MINUS", 1, "xx0x"}, {"INC", "RS0", "RS2", "CR"};
  
  {"RIGHT", 1, "0x1x"}, {"INC", "RS0", "AE", "LD_F"};
  {"RIGHT", 2, "0x1x"}, {"INC", "RS0", "RS2", "CR"};
  {"RIGHT", 1, "1x1x"}, {"EN_D", "EN_DP", "WE_RAM", "LD_F", "CR"};
  {"RIGHT", 1, "xx0x"}, {"INC", "RS0", "RS2", "CR"};
    
  {"LEFT",  1, "0x1x"}, {"DEC", "RS0", "AE", "LD_F"};
  {"LEFT",  2, "0x1x"}, {"INC", "RS0", "RS2", "CR"};
  {"LEFT",  1, "1x1x"}, {"EN_D", "EN_DP", "WE_RAM", "LD_F", "CR"};
  {"LEFT",  1, "xx0x"}, {"INC", "RS0", "RS2", "CR"};
  
  {"LOOP_START", 1, "x011"}, {"INC", "RS1", "RS2"};
  {"LOOP_START", 2, "x011"}, {"INC", "RS0", "RS2", "CR"};
  {"LOOP_START", 1, "x010"}, {"INC", "RS1"};
  {"LOOP_START", 2, "x010"}, {"WE_RAM", "EN_SP", "EN_IP"};
  {"LOOP_START", 3, "x010"}, {"INC", "RS0", "RS2", "CR"};
  {"LOOP_START", 1, "x11x"}, {"EN_DP", "LD_D", "OE_RAM"};
  {"LOOP_START", 2, "x11x"}, {"LD_F", "CR"};
  {"LOOP_START", 1, "xx0x"}, {"INC", "RS1", "RS2"};
  {"LOOP_START", 2, "xx0x"}, {"INC", "RS0", "RS2", "CR"};
  
  {"LOOP_END", 1, "x011"}, {"DEC", "RS1"};
  {"LOOP_END", 2, "x011"}, {"INC", "RS0", "RS2", "CR"};
  {"LOOP_END", 1, "x010"}, {"EN_SP", "OE_RAM", "LD_IP"};
  {"LOOP_END", 2, "x010"}, {"INC", "RS0", "RS2", "CR"};
  {"LOOP_END", 1, "x11x"}, {"EN_DP", "OE_RAM", "LD_D"};
  {"LOOP_END", 2, "x11x"}, {"LD_F", "CR"};
  {"LOOP_END", 1, "xx0x"}, {"DEC", "RS1", "RS2"};
  {"LOOP_END", 2, "xx0x"}, {"INC", "RS0", "RS2", "CR"};
  
  {"OUT", 1, "x01x"}, {"PRE", "EN_D", "INC", "RS0", "RS2", "CR"};
  {"OUT", 1, "x11x"}, {"EN_DP", "OE_RAM", "LD_D"}; 
  {"OUT", 2, "x11x"}, {"PRE", "EN_D", "INC", "RS0", "RS2", "CR"};
  {"OUT", 1, "xx0x"}, {"INC", "RS0", "RS2", "CR"};
  
  % TBD: input
  };
  
 
  printf("Building Table ...\n");
  
  nBits = 11; % Number of address bits = 4 (cmd) + 4 (flags) + 3 (cycle)
  result = NaN * ones(2^nBits, 1);
  for i = 1:size(table, 1)
    result = addInstruction(result, getPattern(table{i, 1}), getBits(table{i, 2}));
  endfor
  
  % Fill remaining addresses with the ERR instruction
  for i = 1:size(result, 1)
    if isnan(result(i))
      result(i) = getBits({"ERR", "CR"});
    endif
  endfor
  
  nRom = ceil((log(max(result))/log(2)) / 8);
  for i = 1:nRom
    filename = [filename_base, num2str(i), ".bin"];
    printf("Writing to file %s\n", filename);
    fid = fopen(filename, "w+b");
    current = bitand(result, 0xff);
    result = bitshift(result, -8);
    
    fwrite(fid, current, "uchar");
    fclose(fid);
  endfor
  
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
      i = addr + 1;
      if ~isnan(table(i))
        error("Duplicate patterns")
      endif
      table(i) = value;
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


function value = getBits(args)
  
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
    "CR",
    "VE",
    "AE",
    "PRE",
    "HLT",
    "ERR"    
  };
  
  
  value = 0;
  for i = 1:length(args)
    sig = args{i};
    idx = find(ismember(signals, sig)) - 1;
    if numel(idx) ~= 1
      error("Invalid signal");
    endif
    value += 2^idx;
  endfor
  
endfunction

function pattern = getPattern(state)
  
  cmd = state{1};
  cycle = state{2};
  flags = state{3};
  
  commands = {
    "NOOP",
    "PLUS",
    "MINUS",
    "LEFT",
    "RIGHT",
    "IN_BUF",
    "IN_IM",
    "OUT",
    "LOOP_START",
    "LOOP_END"
  };
  commandBits = 4;
  cycleBits = 3;
  
  if strcmp(cmd, "ANY")
    cmd = "xxxx";
  else
    cmd = dec2bin(find(ismember(commands, cmd)) - 1, commandBits);
  endif
  cycle = dec2bin(cycle, cycleBits);
  pattern = [cmd, cycle, flags];
  
endfunction  
  
