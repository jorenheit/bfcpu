#include <stdio.h>
#include <string.h>

#define MEM_SIZE 1000
#define JMP_TABLE_SIZE 256

void bfint(char const *);

int main(int argc, char **argv) {
  /* char const *program = // hello world, wikipedia */
  /*   "++++++++[>++++[>++>+++>+++>+<<<<-]>+>+>->>+[<]<-]>>.>---.+++++++..+++.>>.<-.<.+++.------.--------.>>+.>++."; */

  bfint(argv[1]);
}

//----------bfint_begin----------
void bfint(char const *program) {
  unsigned char mem[MEM_SIZE];
  memset(mem, 0, MEM_SIZE);
  unsigned char *ptr = mem;

  int jmp_table[JMP_TABLE_SIZE];
  int jmp_index = 0;
  
  int program_size = strlen(program);
  int index = 0;

  while (index < program_size) {
    switch (program[index]) {      
    case '+': ++(*ptr); break;
    case '-': --(*ptr); break;
    case '<': --ptr; break;
    case '>': ++ptr; break;
    case '.': putchar(*ptr); break;
    case ',': (*ptr) = getchar(); break;
    case '[': {
      if (*ptr) jmp_table[jmp_index++] = index;
      else { 
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
      --jmp_index;
      if (*ptr) index = jmp_table[jmp_index++];
      break; 
    }}
    ++index;
  }
}
//----------bfint_end----------
