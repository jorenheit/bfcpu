#include <iostream>
#include <cassert>
#include <bitset>
#include <fstream>

using namespace std;


const unsigned int TOTAL{ 8192 };
const unsigned int N    { 2730 }; 

typedef bitset<8> val_t;

val_t getValAt(unsigned int address);
void beautiful_print();


int main(int argc, char **argv)
{
    beautiful_print();

    // for debugging
    // cout << getValAt(8189) << '\n';
    // cout << getValAt(8190) << '\n';
    // cout << getValAt(8191) << '\n';
    // cout << getValAt(8192) << '\n';

    ofstream file;
    file.open("output.bin", std::ios::binary);

    for (unsigned int i = 0; i < TOTAL; ++i)
    {
        // for debugging 

        // unsigned long k = getValAt(i).to_ulong();
        // unsigned char t = static_cast<unsigned char>(k);

        // cout << "i  = " << i << '\n';
        // cout << ".  = " << getValAt(i) << '\n';
        // cout << "k  = " << k << '\n';
        // cout << "t  = " << t << '\n';
        // cout << "t' = " << (int)t << '\n';
        // cout << "========\n";
        
        file << static_cast<unsigned char>(getValAt(i).to_ulong());
    }
        

    file.close();
    return EXIT_SUCCESS;
}

val_t getValAt(unsigned int address)
{
    /*
        address = what address to get value in.

        if address is in [0, N-1],   the bank number is 1 == 0000 0001;
        if address is in [N, 2N-1],  the bank number is 2 == 0000 0010;
        if address is in [2N, TOTAL], the bank number is 3 == 0000 0011;
    */
    assert(address >= 0 && "Address must be a non-negative number");
    assert(address <= TOTAL && "Address must be <= than TOTAL");

    unsigned int bank;

    if (address >= 0 && address <= N-1)
        bank = 1;
    else if (address >= N && address <= 2*N-1)
        bank = 2;
    else if (address >= 2*N && address <= TOTAL)
        bank = 3; 

    val_t bank_bin( bank );

    /*
        temp = address transformed into binary number, that we will see in the bank cell.

        IF BANK = 1
            if address > 63, then temp = 0000 0000;
            if address <= 63 (e.g., 63),  then temp = 0011 1111;  
        IF BANK = 2
            address := address - N;
            if address > 63, then temp = 0000 0000;
            if address <= 63 (e.g., 63),  then temp = 0011 1111;  
        IF BANK = 3
            address := address - 2*N;
            if address > 63, then temp = 0000 0000;
            if address <= 63 (e.g., 63),  then temp = 0011 1111;  
    */
    val_t temp;

    switch (bank)
    {
    case 1:
        if (address <= 63)
            temp = val_t(address);
        break;
    case 2:
        if (address - N <= 63)
            temp = val_t(address - N);
        break;
    case 3:
        if (address - 2*N <= 63)
            temp = val_t(address - 2*N);
        break;
    }

    // bank = 3, bank_bin is | 0000 0011
    // temp = 64(in binary)  | 0011 1111
    // bank_bin << 6         | 1100 0000
    // val=bank_bin | temp   | 1111 1111
    val_t val = temp | (bank_bin << 6);

    return val;
}

void beautiful_print()
{
    /*
        BANK 1    ||  ADDRESS
        0100 0000 ||  0 0000 0000 0000 (0)
        0100 0001 ||  0 0000 0000 0001 (1)
        0100 0010 ||  0 0000 0000 0010 (2)
        ...       ||  ...
        0111 1111 ||  0 0000 0011 1111 (63)
        0100 0000 ||  0 0000 xxxx xxxx (< 2730)
        ===============================
        BANK 2    ||  ADDRESS
        1000 0000 ||  0 1010 1010 1010 (2730 + 0)
        1000 0001 ||  0 1010 1010 1011 (2730 + 1)
        1000 0010 ||  0 1010 1010 1100 (2730 + 2)
        ...       ||  ...
        1011 1111 ||  0 1010 1110 1001 (2730 + 63)
        1000 0000 ||  0 xxxx xxxx xxxx (< 2730 * 2)
        ===============================
        similar for BANK 3
    */

    for (unsigned int i = 1; i < 4; ++i)
    {
        cout << "BANK " << i << "\t||\t\t" << "ADDRESS\n";

        for (unsigned int j = 0; j < 3; ++j)
            cout << getValAt(j + N * (i - 1)) << "\t||\t" << bitset<13>(j + N * (i - 1)) << " (" << j + N * (i - 1) << ")\n";
        cout << "...\n";
        cout << getValAt(63 + N * (i - 1)) << "\t||\t" << bitset<13>(63 + N * (i - 1)) << " (" << 63 + N * (i - 1) << ")\n";
        cout << getValAt(70 + N * (i - 1)) << "\t||\t" << bitset<13>(70 + N * (i - 1)) << " < (" << N*i <<")\n";

        cout << "===================================================\n";
    }
}