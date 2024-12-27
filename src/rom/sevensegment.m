NOP = 0;
PLUS = 1;
MINUS = 2;
LEFT = 3;
RIGHT = 4;
IN1 = 5;
IN2 = 6;
OUT = 7;
LOOP_START = 8;
LOOP_END = 9;
HLT = 15;

N = 16;

map = zeros(N, 1);

map(NOP + 1)        = bin2dec("11111100");
map(PLUS + 1)       = bin2dec("11101110");
map(MINUS + 1)      = bin2dec("10110110");
map(LEFT + 1)       = bin2dec("00001110");
map(RIGHT + 1)      = bin2dec("01100010");
map(IN1 + 1)        = bin2dec("11100110");
map(IN2 + 1)        = bin2dec("11100111");
map(OUT + 1)        = bin2dec("11000110");
map(LOOP_START + 1) = bin2dec("10011100");
map(LOOP_END + 1)   = bin2dec("11110000");
map(HLT + 1)        = bin2dec("01101110");

filename = "7seg.bin";
printf("Writing to file %s\n", filename);
fid = fopen(filename, "w+b");
for i = 1:N
  fwrite(fid, map(i));
endfor

fclose(fid);
