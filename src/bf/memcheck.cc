#include <iostream>

int main(int argc, char **argv) {
  if (argc != 2) {
    std::cerr << "Provide (only) the number of cells to check as an integer parameter.\n";
    return 1;
  }

  try {
    int n = std::stoi(argv[1]);
    if (n <= 0) {
      std::cerr << "Expected a positive integer but got " << n << '\n';
    }

    for (int i = 0; i != n; ++i) {
      std::cout << ".>";
    }
  }
  catch (...) {
    std::cerr << "Second argument is not an integer.\n";
    return 1;
  }

}
