function single_value(value, filename)

ADDRESS_SIZE = 13;
ROM_SIZE = 2^ADDRESS_SIZE;
rom = value * ones(1, ROM_SIZE);
printf("Writing to file %s\n", filename);

fid = fopen(filename, "w+b");
for i = 1:ROM_SIZE
  fwrite(fid, rom(i));
endfor
fclose(fid);
