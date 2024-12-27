function create_rom_test(filename_base)

  filename = [filename_base, ".bin"];
  printf("Writing to file %s\n", filename);
  fid = fopen(filename, "w+b");

  nBytes = 2^13;
  for i = 1:nBytes
    fwrite(fid, mod(i-1,256), "uchar");
  endfor

  fclose(fid);

endfunction

