#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>
#include <cassert>
#include <sstream>

std::string gen_bf(std::string const &str);


enum Opcode {
  NOP	      = 0x00,
  PLUS	      = 0x01,
  MINUS	      = 0x02,
  LEFT	      = 0x03,
  RIGHT	      = 0x04,
  IN          = 0x05,
  OUT	      = 0x06,
  LOOP_START  = 0x07,
  LOOP_END    = 0x08,
  RAND        = 0x09,
  PROG_START  = 0x0a,
  PROG_END    = 0x0b,
  LOAD_SLOT   = 0x0c,
  INIT        = 0x0d,
  INIT_FINISH = 0x0e,
  HLT         = 0x0f
};

struct Options
{
  bool halt;
  std::vector<std::string> inputFiles;
  std::ostream *outStream;
  bool echo;
  bool debug;
  bool rand;
  int maxDepth;
  bool allowUnbalanced;
  bool printFilename;
  int init;
};

void printHelp(std::string const &progName)
{
  std::cout << "Usage: " << progName << " [options] <file1, file2, ...>\n"
	    << "Options:\n"
	    << "-h, --help              Display this text.\n"
	    << "-H, --halt-enable       Interpret '!' as HLT in the BF code\n"
	    << "-r, --rand-enable       Interpret '?' as RAND in the BF code\n"
	    << "-g, --debug             Place a breakpoint (!) after each instruction.\n"
	    << "-e, --echo              Follow each input command (,) up by an output command (.) to echo keyboard input.\n"
	    << "-d, --max-depth         Maximum nesting depth of []-pairs.\n"
	    << "-p, --print-filename    Add BF code to print the source filename before the program starts.\n"
	    << "-z [N]                  Initialize N chunks of 256 bytes with zero\'s. Default: N = 1.\n"
	    << "-u, --allow-unbalanced-loops\n"
	    << "                        By default, the assembler will refuse to produce a program with unbalanced\n"
	    << "                        loops ([ and ] do not match). Using this option will allow for this to occur.\n"
	    << "-o [file, stdout]       Specify the output stream/file (default stdout).\n\n"
	    << "Example: " << progName << " -o program.bin program.bf\n";
}


std::pair<Options, int> parseCmdLine(int argc, char **argv)
{
  std::vector<std::string> const args{argv, argv + argc};
  
  Options opt;

  opt.halt = false;
  opt.init = 1;
  opt.outStream = &std::cout;
  opt.inputFiles = {};
  opt.echo = false;
  opt.debug = false;
  opt.maxDepth = 255;
  opt.allowUnbalanced = false;
  opt.rand = false;
  opt.printFilename = false;

  size_t idx = 1;
  
  while (idx < args.size())
  {
    if (args[idx] == "-h" || args[idx] == "--help")
    {
      return {opt, 1};
    }
    else if (args[idx] == "-H" || args[idx] == "--halt-enable")
    {
      opt.halt = true;
      ++idx;
    }
    else if (args[idx] == "-r" || args[idx] == "--rand-enable")
    {
      opt.rand = true;
      ++idx;
    }
    else if (args[idx] == "-e" || args[idx] == "--echo")
    {
      opt.echo = true;
      ++idx;
    }
    else if (args[idx] == "-g" || args[idx] == "--debug")
    {
      opt.debug = true;
      ++idx;
    }
    else if (args[idx] == "-u" || args[idx] == "--allow-unbalanced-loops")
    {
      opt.allowUnbalanced = true;
      ++idx;
    }
    else if (args[idx] == "-p" || args[idx] == "--print-filename")
    {
      opt.printFilename = true;
      ++idx;
    }
    else if (args[idx] == "-d" || args[idx] == "--max-depth")
    {
      if (idx == args.size() - 1)
      {
	std::cerr << "ERROR: No argument passed to option " << args[idx] << ".\n";
	return {opt, 1};
      }
      opt.maxDepth = std::stoi(args[idx + 1]);
      idx += 2;
    }
    else if (args[idx] == "-z")
    {
      if (idx == args.size() - 1)
      {
	std::cerr << "ERROR: No argument passed to option " << args[idx] << ".\n";
	return {opt, 1};
      }

      try {
	opt.init = std::stoi(args[idx + 1]);
	if (opt.init > 255) throw 0;
	idx += 2;
      }
      catch (...) {
	std::cerr << "ERROR: Argument to -z must be an integer <= 255.\n";
	return {opt, 1};
      }
    }
    else if (args[idx] == "-o")
    {
      if (idx == args.size() - 1)
      {
	std::cerr << "ERROR: No argument passed to option " << args[idx] << ".\n";
	return {opt, 1};
      }

      if (args[idx + 1] == "stdout")
      {
	idx += 2;
      }
      else
      {
	std::string const &fname = args[idx + 1];
	static std::ofstream file;
	file.open(fname);
	if (!file.good())
	{
	  std::cerr << "ERROR: could not open output-file " << fname << ".\n";
	  return {opt, 1};
	}

	opt.outStream = &file;
	idx += 2;
      }
    }
    else if (args[idx][0] == '-')
    {
      std::cerr << "ERROR: Unknown option " << args[idx] << ".\n";
      return {opt, 1};
    }
    else {
      // End of options -> rest of args are input files
      break;
    }
  }

  while (idx < args.size()) {
    opt.inputFiles.push_back(args[idx++]);
  }

  if (opt.inputFiles.empty()) {
    std::cerr << "ERROR: no input files.\n";
    return {opt, 1};
  }

  if (!opt.outStream || !opt.outStream->good()) {
    std::cerr << "ERROR: Output file not set. Use -o to specifiy stdout or a file.\n";
    return {opt, 1};
  }


  return {opt, 0};
}

std::vector<unsigned char> result;
void emit(Opcode opcode) {
  result.push_back(opcode);
}

int processFile(std::string const &filename, Options const &opt) {

  auto emit_opcodes = [&](std::istream &in) -> int {
    int nestingDepth = 0;  
    while (in) {
      char c = in.get();
      switch (c)
      {
      case '+': emit(PLUS);  break;
      case '-': emit(MINUS); break; 
      case '<': emit(LEFT);  break; 
      case '>': emit(RIGHT); break; 
      case '.': emit(OUT);   break;
      case ',': {
	emit(IN);
	if (opt.echo) emit(OUT);
	break;
      }
      case '[': {
	emit(LOOP_START);
	if (++nestingDepth > opt.maxDepth)
	{
	  std::cerr << "ERROR: Nesting depth exceeds maximum value (" << opt.maxDepth << ").\n"
		    << "You can change the maximum depth using the -d option.\n";
	  return 1;
	}
	break;
      }
      case ']': {
	emit(LOOP_END);
	--nestingDepth;
	break;
      }
      case '!': {
	if (opt.halt) emit(HLT);
	break;
      }
      case '?': {
	if (opt.rand) emit(RAND);
	break;
      }
      default:
	continue;
      }
	
      if (opt.debug) {
	emit(HLT);
      }
    }

    if (not opt.allowUnbalanced && nestingDepth != 0) {
      std::cerr << "ERROR: " << filename << " contains unbalanced loops. Use the -u option to ignore.\n";
      return 1;
    }
    
    return 0;
  };

  // Program Start
  emit(PROG_START);

  // Print program name
  if (opt.printFilename) {
    std::string bf = gen_bf(filename) + ">++++++++++.>";
    std::istringstream iss(bf);    
    if (emit_opcodes(iss)) return 1;
  }

  // Reached program -> wait for user to start
  emit(HLT);

  // Program body
  std::ifstream file(filename, std::ios::binary);
  if (!file.good()) {
    std::cerr << "ERROR: could not open input-file " << filename << ".\n";
    return 1;
  }
  
  if (emit_opcodes(file)) return 1;
  
  // Program tail
  emit(PROG_END);


  return 0;
}


//----------assemble_begin----------
int assemble(Options const &opt)
{
  // Start with a NOP for stability
  emit(NOP);

  // Zero init DATA
  for (int i = 0; i != opt.init; ++i) {
    emit(INIT);
  }
  
  // Signal that initialization is done and restore the DP back to its home position (0x0100)
  emit(INIT_FINISH);

  // Load program slot into memory
  emit(LOAD_SLOT);

  // Compile program(s) and append to result
  for (std::string file: opt.inputFiles) {
    int err = processFile(file, opt);
    if (err) return err;
  }
  
  // Pack nibbles together
  std::vector<unsigned char> packed;
  unsigned char buf = 0;

  for (size_t i = 0; i != result.size(); ++i) {
    unsigned char cmd = result[i];
    if ((i & 1) == 0) {
      buf = cmd;
      continue;
    }

    buf |= (cmd << 4);
    packed.push_back(buf);
  }

  // account for odd number of instructions
  if (result.size() & 1) {
    packed.push_back(buf);
  }
        
  // Write to file
  for (unsigned char byte: packed) {
    *(opt.outStream) << byte;
  }

  return 0;
}

int main(int argc, char **argv)
{
  auto [opt, ret] = parseCmdLine(argc, argv);

  if (ret == 1) {
    printHelp(argv[0]);
    return 1;
  }

  if (assemble(opt)) return 1;

  std::cout << "Successfully assembled " << opt.inputFiles.size() << " files:\n";
  for (size_t idx = 0; idx != opt.inputFiles.size(); ++idx) {
    std::cout << "Slot " << idx << ": " << opt.inputFiles[idx] << '\n';
  }
  
  return 0;
}
//----------assemble_end----------
