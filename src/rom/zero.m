ADDRESS_SIZE = 13;
ROM_SIZE = 2^ADDRESS_SIZE;

rom = zeros(3, ROM_SIZE);
payloadStrings = {"01010101", "10101010", "11001100"};
payload = [0 0 0];
for i = 1:3
  payload(i) = bin2dec(payloadStrings{i});
endfor

for i = 1:ROM_SIZE
  addr = dec2bin(i - 1, ADDRESS_SIZE);
  rom(1, i) = bin2dec(bank_payload{bank + 1});
endfor


filename = "bank_test.bin";
printf("Writing to file %s\n", filename);
fid = fopen(filename, "w+b");
for i = 1:ROM_SIZE
  fwrite(fid, rom(i));
endfor

fclose(fid);
