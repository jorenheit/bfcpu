function result = create_rom(filename)

  ## D  = 001 - RS0
  ## DP = 010 - RS1
  ## SP = 011 - RS0 | RS1
  ## IP = 100 - RS2
  ## LS = 101 - RS0 | RS2

  table = {

	   {"ANY",   0, "xxxx"}, {"LD_I"};

	   {"PLUS",  1, "x00x"}, {"INC", "RS0", "VE", "LD_F"};
	   {"PLUS",  2, "x00x"}, {"INC", "RS2", "CR"};
	   {"PLUS",  1, "x10x"}, {"LD_D", "OE_RAM", "LD_F", "CR"};
	   {"PLUS",  1, "xx1x"}, {"INC", "RS2", "CR"};

	   {"MINUS", 1, "x00x"}, {"DEC", "RS0", "VE", "LD_F"};
	   {"MINUS", 2, "x00x"}, {"INC", "RS2", "CR"};
	   {"MINUS", 1, "x10x"}, {"LD_D", "OE_RAM", "LD_F", "CR"};
	   {"MINUS", 1, "xx1x"}, {"INC", "RS2", "CR"};

     	   {"LEFT",  1, "0x0x"}, {"DEC", "RS1", "AE", "LD_F"};
	   {"LEFT",  2, "0x0x"}, {"INC", "RS2", "CR"};
	   {"LEFT",  1, "1x0x"}, {"EN_D", "WE_RAM", "LD_F", "CR"};
	   {"LEFT",  1, "xx1x"}, {"INC", "RS2", "CR"};

	   {"RIGHT", 1, "0x0x"}, {"INC", "RS1", "AE", "LD_F"};
	   {"RIGHT", 2, "0x0x"}, {"INC", "RS2", "CR"};
	   {"RIGHT", 1, "1x0x"}, {"EN_D", "WE_RAM", "LD_F", "CR"};
	   {"RIGHT", 1, "xx1x"}, {"INC", "RS2", "CR"};

	   {"LOOP_START", 1, "x001"}, {"INC", "RS0", "RS2"};
	   {"LOOP_START", 2, "x001"}, {"INC", "RS2", "CR"};
	   {"LOOP_START", 1, "x000"}, {"INC", "RS0", "RS1"};
	   {"LOOP_START", 2, "x000"}, {"WE_RAM", "EN_SP", "EN_IP"};
	   {"LOOP_START", 3, "x000"}, {"INC", "RS2", "CR"};
	   {"LOOP_START", 1, "x10x"}, {"LD_D", "OE_RAM"};
	   {"LOOP_START", 2, "x10x"}, {"LD_F", "CR"};
	   {"LOOP_START", 1, "xx1x"}, {"INC", "RS0", "RS2"};
	   {"LOOP_START", 2, "xx1x"}, {"INC", "RS2", "CR"};

	   {"LOOP_END", 1, "x001"}, {"DEC", "RS0", "RS1"};
	   {"LOOP_END", 2, "x001"}, {"INC", "RS0", "RS2", "CR"};
	   {"LOOP_END", 1, "x000"}, {"EN_SP", "OE_RAM", "LD_IP"};
	   {"LOOP_END", 2, "x000"}, {"INC", "RS2", "CR"};
	   {"LOOP_END", 1, "x10x"}, {"OE_RAM", "LD_D"};
	   {"LOOP_END", 2, "x10x"}, {"LD_F", "CR"};
	   {"LOOP_END", 1, "xx1x"}, {"DEC", "RS0", "RS2"};
	   {"LOOP_END", 2, "xx1x"}, {"INC", "RS2", "CR"};

	   {"OUT", 1, "x00x"}, {"EN_OUT", "EN_D", "INC", "RS2", "CR"};
	   {"OUT", 1, "x10x"}, {"OE_RAM", "LD_D"};
	   {"OUT", 2, "x10x"}, {"EN_OUT", "EN_D", "INC", "RS2", "CR"};
	   {"OUT", 1, "xx1x"}, {"INC", "RS2", "CR"};

	   {"IN_BUF", 1, "xx0x"}, {"EN_IN", "LD_D"};
	   {"IN_BUF", 2, "xx0x"}, {"LD_I"};
	   {"IN_BUF", 3, "xx00"}, {"VE", "LD_F", "INC", "RS2", "CR"};
	   {"IN_BUF", 3, "0x01"}, {"CR"};
	   {"IN_BUF", 1, "xx1x"}, {"INC", "RS2",  "CR"};

	   {"IN_IM", 1, "xx0x"}, {"EN_IN", "LD_D", "VE", "LD_F"};
	   {"IN_IM", 2, "xx0x"}, {"INC", "RS2", "CR"};
	   {"IN_IM", 1, "xx1x"}, {"INC", "RS2", "CR"};

	   {"NOP", 1, "xxxx"}, {"INC", "RS2",  "CR"};
	   {"HALT", 1, "xxxx"}, {"HLT"};
           {"HALT", 2, "xxxx"}, {"INC", "RS2",  "CR"};

	   {"INIT", 1, "xxx1"}, {"EN_D", "WE_RAM", "INC", "RS0", "RS2"};
	   {"INIT", 2, "xxx1"}, {"LD_I", "INC", "RS1"};
	   {"INIT", 3, "xx01"}, {"INC", "RS2", "CR"};
	   {"INIT", 3, "xx11"}, {"CR"};

	   {"HOME", 1, "xxxx"}, {"DPR", "INC", "RS2", "CR"};

	   };




  printf("Building Table ...\n");

  %% Number of address bits = 3 (cycle) + 2 (bank) + 3 (cycle) + 4 (instruction)
  nBits = 13;
  result = zeros(2^nBits, 1);
  for addr = 1:numel(result)
    printf("%d/%d\n", addr, numel(result))
    for j = 1:size(table, 1)
      [bank, match] = isMatch(table{j,1}, addr - 1);
      if match
	      result(addr) = getBits(table{j,2}, bank);
        break
      endif
    endfor

    if !match
      result(addr) = getBits({"ERR", "HLT"}, bank);
    endif
  endfor

  printf("Writing to file %s\n", filename);
  fid = fopen(filename, "w+b");
  fwrite(fid, result, "uchar");
  fclose(fid);


endfunction


function [bank, match] = isMatch(state, addr)

  match = false;

  commands = {
	      "NOP",           % 0x00
	      "PLUS",          % 0x01
	      "MINUS",         % 0x02
	      "LEFT",          % 0x03
	      "RIGHT",         % 0x04
	      "IN_BUF",        % 0x05
	      "IN_IM",         % 0x06
	      "OUT",           % 0x07
	      "LOOP_START",    % 0x08
	      "LOOP_END",      % 0x09
	      "--",            % 0x0a
	      "--",            % 0x0b
	      "--",            % 0x0c
	      "INIT",          % 0x0d
	      "HOME",          % 0x0e
	      "HALT",          % 0x0f
	   };

  addr_backup = addr;

  cycle = bitand(addr, 7);
  addr = bitshift(addr, -3);
  bank = bitand(addr, 3);
  addr = bitshift(addr, -2);
  cmd = bitand(addr, 15);
  addr = bitshift(addr, -4);
  flags = bitand(addr, 15);
  addr = bitshift(addr, -4);

  if (addr != 0)
    error("Address format??  %s", dec2bin(addr_backup));
  endif


  ## Check if command matches
  if !strcmp(state{1}, "ANY") && !strcmp(state{1}, commands{cmd + 1})
    return
  endif

  ## Command has matched, check cycle
  if state{2} != cycle
    return
  endif

  ## Cycle has matched, check flags
  flagPattern = state{3};
  flags = dec2bin(flags, numel(flagPattern))(end:-1:1);  %TODO: fix order of flags permanently
  if !all(ismember(flagPattern, "01x"))
    error("flag pattern contains illegal symbols: %s", flagPattern);
  endif

  if numel(flags) != numel(flagPattern)
    error("flag size mismatch: %s and %s", flags, flagPattern);
  endif

  for i = 1:numel(flags)
    if flagPattern(i) != flags(i) && flagPattern(i) != 'x'
      return
    endif
  endfor

  ## Flags match
  match = true;
  return

endfunction

function result = getBits(signals, bank)

  opcodes = {
	     %% BANK 0
	     "HLT",
	     "RS0",
	     "RS1",
	     "RS2",
	     "INC",
	     "DEC",
	     "DPR"
	     "EN_SP",
	     %% BANK 1
	     "OE_RAM",
	     "WE_RAM",
	     "EN_IN",
	     "EN_OUT",
	     "VE",
	     "AE",
	     "LD_I",
	     "LD_F",
	     %% BANK 2
	     "EN_IP",
	     "LD_IP",
	     "EN_D",
	     "LD_D",
	     "CR",
	     "ERR",
    };


  result = 0;
  for i = 1:length(signals)
    sig = signals{i};
    idx = find(ismember(opcodes, sig)) - 1;
    if numel(idx) ~= 1
      error("Invalid signal %s", sig);
    endif

    if floor(idx / 8) != bank
      continue
    endif

    result += 2^(idx - 8 * bank);
  endfor
endfunction
