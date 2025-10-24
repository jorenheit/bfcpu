#include <iostream>

int main(int argc, char **argv) {
  if (argc != 4) {
    std::cerr << "Syntax: " << argv[0] << " [#loops] [dp-start] [ip-start]\n";
    return 1;
  }

  try {
    int n = std::stoi(argv[1]);
    if (n <= 0) {
      std::cerr << "Expected a positive integer but got " << n << '\n';
      return 1;
    }
    int dpStart = std::stoi(argv[2]);
    if (dpStart < 0) {
      std::cerr << "Expected a positive integer but got " << dpStart << '\n';
      return 1;
    }
    int ipStart = std::stoi(argv[3]);
    if (ipStart < 0) {
      std::cerr << "Expected a positive integer but got " << ipStart << '\n';
      return 1;
    }

    if (dpStart > ipStart) {
      std::cerr << "ip-start must be less than dp-start\n";
      return 1;
    }

    // Move DP to start location
    for (int i = 0; i != dpStart; ++i) {
      std::cout << ">";
    }

    // Add instructions that do nothing until we reach ip-start
    for (int i = dpStart; i < ipStart; i += 2) {
      std::cout << "+-";
    }

    // Construct loops here
    std::cout << "++";
    for (int i = 0; i != n - 1; ++i) {
      std::cout << "[>++";
    }
    std::cout << "[-]";
    for (int i = 0; i != n - 1; ++i) {
      std::cout << "<[-]]";
    }
  }
  catch (...) {
    std::cerr << "Second argument is not an integer.\n";
    return 1;
  }

}

/*
  Example: 3 nested loops
++[>++[>++[-]<[-]]<[-]]
*/
