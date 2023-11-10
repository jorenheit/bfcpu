/*
For testing EEPROMS and banking system we need 20 control pins output:
    - 8 bits for CuR1
    - 8 bits for CuR2
    - 4 bits for Cur3 

BANK 1                       ADDRESS
0100 0000 0000 0000 0000     | 0
0100 0000 0000 0000 0001     | 1
0100 0000 0000 0000 0010     | 2
...                          | ...
0100 0bi                     | i
...                          | ...
=====================================
BANK 2                       ADDRESS
0100 0ba                     | a
=====================================
BANK 3                       ADDRESS
1100 0bc                     | c
*/

#include <fstream>
#include <vector>
#include <cassert>
#include <bitset>
#include <iostream>


/*
    EEPROM has 13 address lines, so 8192 cells. Number of cells is NOT
    divisible by three, thus bank 1 and 2 have 2731 cells whereas bank 3 has 2730
*/
constexpr unsigned int _N { 8192 };
constexpr unsigned int N1 { 2731 };
constexpr unsigned int N2 { 2730 }; 


typedef std::vector<std::bitset<20>> bank;

// get values for a bank
bank get_bank_vals(unsigned int N, unsigned int bank_n, unsigned int offset)
{
    bank vals;

    for (unsigned int i = offset; i < N + offset; ++i)
    {
        std::bitset<20> val{i};

        // e.g.
        // val          : 0000 0001 1010 1010 1010
        // bank_n       : 0000 0000 0000 0000 0011  (for bank 3 = 11)
        // bank_n << 18 : 1100 0000 0000 0000 0000
        // val | bank_n : 1100 0001 1010 1010 1010
        val |= std::bitset<20>{ bank_n } << 18;

        vals.emplace_back(val);
    }

    return vals;
}

// beautiful output
void print(std::vector<bank> &banks, unsigned int n = 5, unsigned int offset = 30)
{
    /*
        n - how many cells will be printed from each bank.
        offset - how far from the first cell of each bank.
    */
    assert(n >= 0 || "n must be a non-negative number.\n");
    assert(offset >= 0 || "offset must be a non-negative number.\n");
    assert(offset <= _N - N2 || "offset must be less than _N - N2.\n");

    std::cout << "BANK 1 \t\t\tADDRESS\n";
    for (unsigned int i = offset; i < offset + n; ++i)
        std::cout << banks[0][i] << '\t' << std::bitset<13>{ i } << '\n';
    std::cout << "=================================================================\n";

    std::cout << "BANK 2 \t\t\tADDRESS\n";
    for (unsigned int i = offset; i < offset + n; ++i)
        std::cout << banks[1][i] << '\t' << std::bitset<13>{ i } << '\n';
    std::cout << "=================================================================\n";

    std::cout << "BANK 3 \t\t\tADDRESS\n";
    for (unsigned int i = offset; i < offset + n; ++i)
        std::cout << banks[2][i] << '\t' << std::bitset<13>{ i } << '\n';
    std::cout << "=================================================================\n";
}


int main(int argc, char **argv)
{
    // get values for three banks
    std::vector<bank> banks{
        get_bank_vals(N1, 1, 0),
        get_bank_vals(N1, 2, N1),
        get_bank_vals(N2, 3, N1 + N1)
    };
 
    print(banks);

    // load banks into a binary file
    std::ofstream file;
    file.open("./output.bin", std::ios::binary);

    for (const auto &bank : banks)
        for (const auto &value : bank)
            file << value; 

    file.close();
    return EXIT_SUCCESS;
}

