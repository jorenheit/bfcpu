#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>

enum Instructions
{
    NOOP	= 0x00,
    PLUS	= 0x01,
    MINUS	= 0x02,
    LEFT	= 0x03,
    RIGHT	= 0x04,
    IN_BUF      = 0x05,
    IN_IM       = 0x06,
    OUT		= 0x07,
    LOOP_START	= 0x08,
    LOOP_END	= 0x09,
    HLT         = 0x0f
};

enum KeyboardInputMode
{
    IMMEDIATE,
    BUFFERED
};

enum InstructionSize
{
    NIBBLE,
    BYTE
};

struct Options
{
    KeyboardInputMode mode;
    std::istream *inStream;
    std::ostream *outStream;
    InstructionSize instructionSize;
    int maxDepth;
    bool allowUnbalanced;
};

void printHelp(std::string const &progName)
{
    std::cout << "Usage: " << progName << " [options] <file(.bf)>\n"
              << "Options:\n"
              << "-h, --help            Display this text.\n"
              << "-i, --immediate-input Assemble input commands (,) to immediate mode (\').\n"
	      << "-d, --max-depth       Maximum nesting depth of []-pairs.\n"
	      << "-u, --allow-unbalanced-loops\n"
	      << "                      By default, the assembler will refuse to produce a program with unbalanced\n"
	      << "                      loops ([ and ] do not match). Using this option will allow for this to occur.\n"
              << "-n, --nibble          Make every instruction 1 nibble long. The assembler will pack two\n"
              << "                      commands in every byte, where the low nibble is the first to be executed.\n"
              << "                      E.g. the byte 0x42 consists of the command 0x2 and 0x4 in that order.\n"
              << "-o [file, stdout]     Specify the output stream/file (default stdout).\n\n"
              << "Example: " << progName << " --immediate-input --nibble -o program.bin program.bf\n";
}


std::pair<Options, int> parseCmdLine(int argc, char **argv)
{
    std::vector<std::string> const args{argv, argv + argc};
    Options opt;

    opt.mode = BUFFERED;
    opt.inStream = &std::cin;
    opt.outStream = &std::cout;
    opt.instructionSize = BYTE;
    opt.maxDepth = 1<<8;
    opt.allowUnbalanced = false;

    std::string inputFile = "stdin";
    size_t idx = 1;
    while (idx < args.size())
    {
        if (args[idx] == "-h" || args[idx] == "--help")
        {
            return {opt, 1};
        }
        else if (args[idx] == "-i" || args[idx] == "--immediate-input")
        {
            opt.mode = IMMEDIATE;
            ++idx;
        }
        else if (args[idx] == "-n" || args[idx] == "--nibble")
        {
            opt.instructionSize = NIBBLE;
            ++idx;
        }
        else if (args[idx] == "-u" || args[idx] == "--allow-unbalanced-loops")
        {
            opt.allowUnbalanced = true;
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
        else if (idx == args.size() - 1)
        {
            inputFile = args.back();
            break;
        }
        else
        {
            std::cerr << "Unknown option " << args[idx] << ".\n";
            return {opt, 1};
        }
        
    }

    if (!opt.outStream || !opt.outStream->good())
    {
        std::cerr << "ERROR: Output file not set. Use -o to specifiy stdout or a file.\n";
        return {opt, 1};
    }

    if (inputFile != "stdin")
    {
        static std::ifstream file;
        file.open(inputFile, std::ios::binary);
        if (!file.good())
        {
            std::cerr << "ERROR: could not open input-file " << inputFile << ".\n";
            return {opt, 1};
        }

        opt.inStream = &file;
    }

    return {opt, 0};
}

int assemble(Options const &opt)
{
    std::istream &in  = *(opt.inStream);
    std::vector<unsigned char> result;
    int nestingDepth = 0;

    while (in)
    {
        char c = in.get();
        switch (c)
        {
        case '+': result.push_back(PLUS); break;
        case '-': result.push_back(MINUS); break; 
        case '<': result.push_back(LEFT); break; 
        case '>': result.push_back(RIGHT); break; 
        case '.': result.push_back(OUT); break; 
        case ',': result.push_back(opt.mode == BUFFERED ? IN_BUF : IN_IM); break; 

        case '[': {
	    result.push_back(LOOP_START);
	    if (++nestingDepth > opt.maxDepth)
	    {
		std::cerr << "ERROR: Nesting depth exceeds maximum value (" << opt.maxDepth << ").\n"
			  << "You can change the maximum depth using the -d option.\n";
		return 1;
	    }
	    break;
	}
        case ']': {
	    result.push_back(LOOP_END);
	    --nestingDepth;
	    break;
	}
        default:
            continue;
        }
    }


    if (not opt.allowUnbalanced && nestingDepth != 0)
    {
	std::cerr << "ERROR: Program contains unbalanced loops. Use the -u option to ignore.\n";
	return 1;
    }

    
    // Pack nibbles together
    if (opt.instructionSize == NIBBLE)
    {
        std::vector<unsigned char> packed;
        unsigned char buf = 0;

        for (size_t i = 0; i != result.size(); ++i)
        {
            unsigned char cmd = result[i];
            if ((i & 1) == 0)
            {
                buf = cmd;
                continue;
            }

            buf |= (cmd << 4);
            packed.push_back(buf);
        }

        // account for odd number of instructions
        if (result.size() & 1)
            packed.push_back(buf);
        
        result.swap(packed);
    }

    // Write to file
    for (unsigned char byte: result)
        *(opt.outStream) << byte;

    return 0;
}

int main(int argc, char **argv)
{
    auto [opt, ret] = parseCmdLine(argc, argv);

    if (ret == 1)
    {
        printHelp(argv[0]);
        return 1;
    }

    return assemble(opt);
}
