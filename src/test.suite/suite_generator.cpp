#include <iostream>
#include <cassert>
#include <bitset>
#include <fstream>

using namespace std;


const unsigned int TOTAL{ 8192 };
const unsigned int N    { 2730 }; 

typedef bitset<8> val_t;
typedef bitset<13> address_t;

address_t mask_bank_1  (0b0000000000000);
address_t mask_bank_2  (0b0100000000000);
address_t mask_bank_3  (0b1000000000000);
address_t mask_residue (0b1100000000000);

// functions declarations
val_t getValAt_Arthur(unsigned int);
val_t getValAt(unsigned int);
void beautiful_print(val_t (*)(unsigned int));


int main(int argc, char **argv)
{
    beautiful_print(getValAt);

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

val_t getValAt_Arthur(unsigned int address)
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

val_t getValAt(unsigned int address)
{
    val_t bank_value(0b00000000);
    address_t address_value(address);

    // determine the bank number and set the corresponding address
    if ((address_value & mask_bank_1) == mask_bank_1)
    {
        bank_value |= val_t(0b00000000);

        if (address - mask_bank_1.to_ulong() < 64)
            bank_value |= val_t(address - mask_bank_1.to_ulong());
    }
    if ((address_value & mask_bank_2) == mask_bank_2)
    {
        bank_value |= val_t(0b01000000);

        if (address - mask_bank_2.to_ulong() < 64)
            bank_value |= val_t(address - mask_bank_2.to_ulong());
    }
    if ((address_value & mask_bank_3) == mask_bank_3)
    {
        bank_value |= val_t(0b10000000);

        if (address - mask_bank_3.to_ulong() < 64)
            bank_value |= val_t(address - mask_bank_3.to_ulong());
    }
    if ((address_value & mask_residue) == mask_residue)
        bank_value |= val_t(0b11000000);
    
    return bank_value;
}

void beautiful_print(val_t (*get_val_at)(unsigned int))
{
    /*
        BANK 1    ||  ADDRESS
        0000 0000 ||  00 00000 000000 (0)
        0000 0001 ||  00 00000 000001 (1)
        0000 0010 ||  00 00000 000010 (2)
        ...       ||  ...
        0011 1111 ||  00 00000 111111 (63)
        0000 0000 ||  00 xxxxx xxxxxx (<2048)
        ===============================
        BANK 2    ||  ADDRESS
        0100 0000 ||  01 00000 000000 (2048)
        1000 0001 ||  01 00000 000001 (2049)
        1000 0010 ||  01 00000 000010 (2050)
        ...       ||  ...
        1011 1111 ||  01 00000 111111 (2111)
        1000 0000 ||  01 xxxxx xxxxxx (<4096)
        ===============================
        similar for BANK 3
    */

    for (unsigned int i = 0; i < 3; ++i)
    {
        std::cout << "==============================================\n";
        std::cout << "BANK " << i + 1 << "\t\t||\t\t" << "ADDRESS" << '\n';
        for (unsigned int j = 0; j < 3; ++j)
        {
            unsigned int num{ j + i * 2048 };
            std::cout << get_val_at(num) << "\t||\t" << address_t(num) << '(' << address_t(num).to_ulong() << ")\n";
        }
        std::cout << "...\t\t||\t\t...\n";
        std::cout << get_val_at(63 + 2048 * i) << "\t||\t" << address_t(63 + 2048 * i) << '(' << address_t(63 + 2048 * i).to_ulong() << ")\n";
        std::cout << get_val_at(79 + 2048 * i) << "\t||\t" << address_t(79 + 2048 * i) << "(<" << 64 + 2048*i << ")\n";
    }

    std::cout << "========================================\n";
    std::cout << "RESIDUE " << "\t||\t\t" << "ADDRESS" << '\n';
    std::cout << "...\t\t||\t\t...\n";
    std::cout << get_val_at(7000) << "\t||\t" << address_t(7000) << "(<8192)\n";
}