#include "chip8.h"
#include <fstream>
#include <cstring>

const unsigned int START_ADDRESS = 0x200; //0x000 to 0x1FF is reserved, so ROM gets loaded at 0x200 
const unsigned int FONTSET_SIZE = 80; //each font in memory is defined by 5 bytes in hex values than represent the pixels of the font
const unsigned int FONTSET_START_ADDRESS = 0x050;


uint16_t opcode;

uint8_t fontset[FONTSET_SIZE] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
	0x20, 0x60, 0x20, 0x20, 0x70, // 1
	0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
	0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
	0x90, 0x90, 0xF0, 0x10, 0x10, // 4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
	0xF0, 0x10, 0x20, 0x40, 0x40, // 7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
	0xF0, 0x90, 0xF0, 0x90, 0x90, // A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
	0xF0, 0x80, 0x80, 0x80, 0xF0, // C
	0xE0, 0x90, 0x90, 0x90, 0xE0, // D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
	0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

//Opcode function definition, we increment the PC value to PC + 2 in the CPU cycle function

void Chip8::OP_00E0() 
{
    std::memset(MEM_BUFFER, 0, sizeof(MEM_BUFFER)); //cstring functionxs

}

void Chip8::OP_00EE()
{
    SP--;
    PC = STACK[SP];
}

void Chip8::OP_1nnn()
{
    uint16_t address = (opcode & 0x0FFFu);
    PC = address;
}

void Chip8::OP_2nnn()
{
    uint16_t address = (opcode & 0x0FFFu);
    STACK[SP] = PC; //actually PC + 2, consecutive instruction after CALL
    SP++;
    PC = address;
}

void Chip8::OP_3xkk()
{
    uint8_t byte = (opcode & 0x00FFu) ;
    uint8_t x = (opcode & 0x0F00u) >> 8u ; //byte shift to the least significant bit
    if (V[x] == byte)
        PC = PC + 2;
}

void Chip8::OP_4xkk()
{
    uint8_t byte = (opcode & 0x00FFu) ;
    uint8_t x = (opcode & 0x0F00u) >> 8u ; //byte shift to the least significant bit
    if (V[x] != byte)
        PC = PC + 2;
}

void Chip8::OP_5xy0()
{
    uint8_t y = (opcode & 0x00F0u) >> 4u ;
    uint8_t x = (opcode & 0x0F00u) >> 8u ; //byte shift to the least significant bit
    if (V[x] == V[y])
        PC = PC + 2;
}

void Chip8::OP_6xkk()
{
    uint8_t byte = (opcode & 0x00FFu);
    uint8_t x = (opcode & 0x0F00u) >> 8u ; //byte shift to the least significant bit
    V[x] = byte;
}

void Chip8::OP_7xkk()
{
    uint8_t byte = (opcode & 0x00FFu);
    uint8_t x = (opcode & 0x0F00u) >> 8u ; //byte shift to the least significant bit
    V[x] += byte;
}

void Chip8::OP_8xy0()
{
    uint8_t y = (opcode & 0x00F0u) >> 4u;
    uint8_t x = (opcode & 0x0F00u) >> 8u ; //byte shift to the least significant bit
    V[x] = V[y];
}

void Chip8::OP_8xy1()
{
    uint8_t y = (opcode & 0x00F0u) >> 4u;
    uint8_t x = (opcode & 0x0F00u) >> 8u ; //byte shift to the least significant bit
    V[x] |= V[y];
}

void Chip8::OP_8xy2()
{
    uint8_t y = (opcode & 0x00F0u) >> 4u;
    uint8_t x = (opcode & 0x0F00u) >> 8u ; //byte shift to the least significant bit
    V[x] &= V[y];
}

void Chip8::OP_8xy3()
{
    uint8_t y = (opcode & 0x00F0u) >> 4u;
    uint8_t x = (opcode & 0x0F00u) >> 8u ; //byte shift to the least significant bit
    V[x] ^= V[y]; //Vx = Vx XOR Vy
}

void Chip8::OP_8xy4()
{
    uint8_t y = (opcode & 0x00F0u) >> 4u;
    uint8_t x = (opcode & 0x0F00u) >> 8u ; //byte shift to the least significant bit

    uint16_t sum = V[x] + V[y];

    if (sum > 255U)
        V[0xF] = 1;
    else
        V[0xF] = 0;
    
    V[x] = sum & 0xFF; 
}

void Chip8::OP_8xy5()
{
    uint8_t y = (opcode & 0x00F0u) >> 4u;
    uint8_t x = (opcode & 0x0F00u) >> 8u ; 

    if (V[x] > V[y])
        V[0xF] = 1;
    else
        V[0xF] = 0;
    
    V[x] -= V[y]; 
}

void Chip8::OP_8xy6()
{
    uint8_t x = (opcode & 0x0F00u) >> 8u ; //byte shift to the least significant bit

    //Saves LSB in VF
    
    V[0xF] = (V[x] & 0x1u);
    V[x] >>= 1;

}       

void Chip8::OP_8xy7()
{
    uint8_t y = (opcode & 0x00F0u) >> 4u;
    uint8_t x = (opcode & 0x0F00u) >> 8u ; 

    if (V[y] > V[x]) //Set VF NOT borrow
        V[0xF] = 1;
    else
        V[0xF] = 0;
    
    V[x] = V[y] - V[x]; 
}

void Chip8::OP_8xyE()
{
    uint8_t x = (opcode & 0x0F00u) >> 8u ; //byte shift to the least significant bit

    //Saves MSB in VF
    
    V[0xF] = (V[x] & 0x80u) >> 7u;
    V[x] <<= 1;

}   

void Chip8::OP_9xy0()
{
    uint8_t y = (opcode & 0x00F0u) >> 4u ;
    uint8_t x = (opcode & 0x0F00u) >> 8u ; //byte shift to the least significant bit
    if (V[x] != V[y])
        PC = PC + 2;
} 

void Chip8::OP_Annn()
{
    uint16_t address = (opcode & 0x0FFFu);
    I_REG = address;
}

void Chip8::OP_Bnnn()
{
	uint16_t address = (opcode & 0x0FFFu);

	PC = V[0] + address;
}

void Chip8::OP_Cxkk()
{
    uint8_t x = (opcode & 0x0F00u) >> 8u ; //byte shift to the least significant bit
    uint8_t byte = (opcode & 0x00FFu);

    V[x] = randByte(randGen) & byte;

}

void Chip8::OP_Dxyn()
{
    uint8_t x = (opcode & 0x0F00u) >> 8u;
    uint8_t y = (opcode & 0x00F0u) >> 4u;
    uint8_t height = (opcode & 0x000Fu);

    uint8_t x_pos = V[x] % VIDEO_WIDTH;
    uint8_t y_pos = V[y] % VIDEO_HEIGHT;

    V[0xF] = 0;

    for (uint8_t row = 0; row < height; ++row)
    {
        uint8_t spriteByte = MEM[I_REG + row];

        for (uint8_t col = 0; col < 8; ++col)
        {
            if ((x_pos + col) >= VIDEO_WIDTH || (y_pos + row) >= VIDEO_HEIGHT)
                continue;

            uint8_t spritePixel = spriteByte & (0x80u >> col);

            uint32_t& screenPixel = MEM_BUFFER[(y_pos + row) * VIDEO_WIDTH + (x_pos + col)];

            if (spritePixel)
            {
                if (screenPixel == 0xFFFFFFFF)
                {
                    V[0xF] = 1;
                }

                screenPixel ^= 0xFFFFFFFF;
            }
        }
    }
}

void Chip8::OP_Ex9E()
{
    uint8_t x = (opcode & 0x0F00u) >> 8u ; //byte shift to the least significant bit
    uint8_t key = V[x];
    if (KEYPAD[key])
        PC = PC + 2;
}

void Chip8::OP_ExA1()
{
    uint8_t x = (opcode & 0x0F00u) >> 8u ; //byte shift to the least significant bit
    uint8_t key = V[x];
    if (!KEYPAD[key])
        PC = PC + 2;
}

void Chip8::OP_Fx07()
{
    uint8_t x = (opcode & 0x0F00u) >> 8u ; //byte shift to the least significant bit

    V[x] = D_TIMER;
}

void Chip8::OP_Fx0A()
{
    uint8_t x = (opcode & 0x0F00u) >> 8u;

    for (uint8_t i = 0; i < 16; ++i)
    {
        if (KEYPAD[i]) //iterates over all keypad buttons until one is pressed
        {
            V[x] = i;
            return;
        }
    }

    PC -= 2; //repeats the same instruction repeatedly
}

void Chip8::OP_Fx15()
{
    uint8_t x = (opcode & 0x0F00u) >> 8u ; //byte shift to the least significant bit

    D_TIMER = V[x];
}

void Chip8::OP_Fx18()
{
    uint8_t x = (opcode & 0x0F00u) >> 8u;
    S_TIMER = V[x];
}

void Chip8::OP_Fx1E()
{
    uint8_t x = (opcode & 0x0F00u) >> 8u;

    I_REG += V[x];
}

void Chip8::OP_Fx29()
{
    uint8_t x = (opcode & 0x0F00u) >> 8u;
    uint8_t digit = V[x];

    I_REG = FONTSET_START_ADDRESS + (5 * digit);

}

void Chip8::OP_Fx33()
{
    uint8_t x = (opcode & 0x0F00u) >> 8u;
    uint8_t value = V[x];

    MEM[I_REG + 2] = value % 10; 
    value /= 10;

    MEM[I_REG + 1] = value % 10;
    value /= 10;

    MEM[I_REG] = value % 10;
    
}

void Chip8::OP_Fx55()
{
    uint8_t x = (opcode & 0x0F00u) >> 8u;

    for (uint8_t i = 0; i <= x ; i++)
    {
        MEM[I_REG + i] = V[i];
    }
}

void Chip8::OP_Fx65()
{
    uint8_t x = (opcode & 0x0F00u) >> 8u;

    for (uint8_t i = 0; i <= x ; i++)
    {
        V[i] =  MEM[I_REG + i];
    }

}




//Function pointer table definition

void Chip8::Table0()
{
    ((*this).*(table0[opcode & 0x000Fu])) (); // Insane sentence, but this executes the function returned by the member function
                                              // pointer of the current class object. The () is what executes the function,
                                              // the .* is a operand to get the pointer of the function member, but doesn't execute it.

                                              //An equivalent line of code would be (this->*table0[opcode & 0x000Fu]) ();
};

void Chip8::Table8()
{
    (this->*table8[opcode & 0x000Fu]) (); //All these functons serve as the second table for the repeated opcode nums.
};

void Chip8::TableE()
{
    (this->*tableE[opcode & 0x000Fu]) ();
};

void Chip8::TableF()
{
    (this->*tableF[opcode & 0x00FFu]) ();
};

void Chip8::OP_NULL()
{

};


Chip8::Chip8() : randGen(std::chrono::system_clock::now().time_since_epoch().count()) //constructor, also initializes randGen seed with internal clock
{
    PC = START_ADDRESS;
    SP = 0;
    opcode = 0;
    I_REG = 0;
    SP = 0;
    D_TIMER = 0;
    S_TIMER = 0;

    std::fill(std::begin(V), std::end(V), 0);
    std::fill(std::begin(MEM), std::end(MEM), 0);
    std::fill(std::begin(STACK), std::end(STACK), 0);
    std::fill(std::begin(KEYPAD), std::end(KEYPAD), 0);
    std::fill(std::begin(MEM_BUFFER), std::end(MEM_BUFFER), 0); //REALLY impotant to correctly initialize all sorts of data to avoid unexpected results.

    for (unsigned int i = 0; i < FONTSET_SIZE; i++)
    {
        MEM[FONTSET_START_ADDRESS + i] = fontset[i];
    }

    //POINTER TABLE!! 
    
    table[0x0] = &Chip8::Table0;
    table[0x1] = &Chip8::OP_1nnn;
    table[0x2] = &Chip8::OP_2nnn;
    table[0x3] = &Chip8::OP_3xkk;
    table[0x4] = &Chip8::OP_4xkk;
    table[0x5] = &Chip8::OP_5xy0;
    table[0x6] = &Chip8::OP_6xkk;
    table[0x7] = &Chip8::OP_7xkk;
    table[0x8] = &Chip8::Table8;
    table[0x9] = &Chip8::OP_9xy0;
    table[0xA] = &Chip8::OP_Annn;
    table[0xB] = &Chip8::OP_Bnnn;
    table[0xC] = &Chip8::OP_Cxkk;
    table[0xD] = &Chip8::OP_Dxyn;
    table[0xE] = &Chip8::TableE;
    table[0xF] = &Chip8::TableF; //master table is full, now we have to assign secondary tables (which won't be full)

    for (size_t i = 0; i <= 0xE ; i++)
    {
        table0[i] = &Chip8::OP_NULL;
        table8[i] = &Chip8::OP_NULL;
        tableE[i] = &Chip8::OP_NULL;
    }

    for (size_t i = 0; i <= 0x65 ; i++)
    {
        tableF[i] = &Chip8::OP_NULL;
    }

    table0[0x0] = &Chip8::OP_00E0;
    table0[0xE] = &Chip8::OP_00EE;

    table8[0x0] = &Chip8::OP_8xy0;
    table8[0x1] = &Chip8::OP_8xy1;
    table8[0x2] = &Chip8::OP_8xy2;
    table8[0x3] = &Chip8::OP_8xy3;
    table8[0x4] = &Chip8::OP_8xy4;
    table8[0x5] = &Chip8::OP_8xy5;
    table8[0x6] = &Chip8::OP_8xy6;
    table8[0x7] = &Chip8::OP_8xy7;
    table8[0xE] = &Chip8::OP_8xyE;

    tableE[0x1] = &Chip8::OP_ExA1;
    tableE[0xE] = &Chip8::OP_Ex9E;

    tableF[0x07] = &Chip8::OP_Fx07;
    tableF[0x0A] = &Chip8::OP_Fx0A;
    tableF[0x15] = &Chip8::OP_Fx15;
    tableF[0x18] = &Chip8::OP_Fx18;
    tableF[0x1E] = &Chip8::OP_Fx1E;
    tableF[0x29] = &Chip8::OP_Fx29;
    tableF[0x33] = &Chip8::OP_Fx33;
    tableF[0x55] = &Chip8::OP_Fx55;
    tableF[0x65] = &Chip8::OP_Fx65;

    



};

void Chip8::LoadROM(char const* filename)
{
    //open file as binary file, moves pointer at the end of file
    std::ifstream file(filename, std::ios::binary | std::ios::ate);

    if(file.is_open())
    {
        //calculates the size of the file and creates a buffer for allocating its contents
        std::streampos size = file.tellg();     //streampos = flexible data type for pointer positions in streams. tellg() returns current position (at the end, size)
        char* buffer = new char[size];          //size is unknow in compilation time -> we need to use pointers and dynamic memory

        file.seekg(0, std::ios::beg);           //changes current position from begining (0 is the offset, and we don't need one)
        file.read(buffer, size);                //reads al the data from the starting position to the indicated size into the buffer
        file.close();
        
        //LOAD the ROM contents into the system's memory from the adress 0x200
        for (std::size_t i = 0; i < size; i++)
        {
            MEM[START_ADDRESS + i] = buffer[i];
        }

        delete[] buffer;
    }

};

void Chip8::Cycle()
{
    opcode = (MEM[PC] << 8u | MEM[PC + 1]); //First bit gets shifted to upper OPCODE half

    PC += 2;
    
    std::cout << "Executing opcode: " << std::hex << opcode << " at PC: " << PC << " I_REG: " << I_REG << std::endl;
    (this->*table[(opcode & 0xF000) >> 12u]) ();

    if (D_TIMER > 0)
    {
        D_TIMER -= 1;
    }

    if (S_TIMER > 0)
    {
        S_TIMER -= 1;
    }
}
