ADDRESS_SIZE = 13;
ROM_SIZE = 2^ADDRESS_SIZE;
BANK_BITS = [3 4];

rom = zeros(1, ROM_SIZE);
bank_payload = {"01010101", "10101010", "11001100", "11111111"};

for i = 1:ROM_SIZE
  addr = dec2bin(i - 1, ADDRESS_SIZE);
  bank = bin2dec([addr(ADDRESS_SIZE - max(BANK_BITS)), addr(ADDRESS_SIZE - min(BANK_BITS))]);
  rom(i) = bin2dec(bank_payload{bank + 1});
endfor


filename = "bank_test.bin";
printf("Writing to file %s\n", filename);
fid = fopen(filename, "w+b");
for i = 1:ROM_SIZE
  fwrite(fid, rom(i));
endfor

fclose(fid);
