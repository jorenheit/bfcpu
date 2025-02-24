#include <stdio.h>
#include <string.h>

#define MEM_SIZE 1000
#define JMP_TABLE_SIZE 256

unsigned char mem[MEM_SIZE];
int jmp_table[JMP_TABLE_SIZE];
int jmp_index = 0;


int main() {
  memset(mem, 0, MEM_SIZE);
  unsigned char *ptr = mem;
  char const *program = "++++++++++[>+++++++>++++++++"
                        "++>+++>+<<<<-]>++.>+.+++++++"
                        "..+++.>++.<<+++++++++++++++."
                        ">.+++.------.--------.>+.>.";

  int program_size = strlen(program);
  int index = 0;
  while (index < program_size) {
    switch (program[index]) {
    case '+': ++(*ptr); break;
    case '-': --(*ptr); break;
    case '<': ++ptr; break;
    case '>': --ptr; break;
    case '.': printf("%c", *ptr); break;
    case ',': (*ptr) = getc(stdin); break;
    case '[': {
      if (*ptr) jmp_table[jmp_index++] = index; // enter loop
      else { // find matching ']'
	int count = 1;
	while (count != 0) {
	  switch (program[++index]) {
	  case '[': ++count; break;
	  case ']': --count; break;
	  }
	}
      }
      break;
    }
    case ']': {
      if (*ptr) index = jmp_table[jmp_index--];
      break; // jump back
    }
    default: break; // ignore other characters
    }
    ++index;
  }
}
