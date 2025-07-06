#pragma once

#include <cstdint>
#include <chrono>
#include <random>
#include <iostream>

const unsigned int VIDEO_WIDTH = 64;
const unsigned int VIDEO_HEIGHT = 32;


class Chip8 //Basic design of the system
{
    public:

        void LoadROM(char const* filename);
        Chip8();
        typedef void (Chip8::*Chip8func) (); //Chip8func -> Chip8 member function pointer that returns no value (void type)

        //instruction functions
        void OP_00E0(); //CLS = Clear display
        void OP_00EE(); //RET = return from CALL
        void OP_1nnn(); //JMP addr = Jumps to addr NNN in the opcode
        void OP_2nnn(); //CALL addr = Calls subroutine at NNN
        void OP_3xkk(); //SE Vx, byte = Skips instruction if  Vx = kk
        void OP_4xkk(); //SNE Vx, byte = Skips instruction if  Vx != kk
        void OP_5xy0(); //SE Vx, Vy = Skips instruction if  Vx = Vy
        void OP_6xkk(); //LD Vx, byte = Sets Vx = kk
        void OP_7xkk(); //ADD Vx, byte = Sets Vx = Vx + kk
        void OP_8xy0(); //LD Vx, Vy =  Sets Vx = Vy
        void OP_8xy1(); //OR Vx, Vy = Set Vx = Vx OR Vy
        void OP_8xy2(); //AND Vx, Vy = Set Vx = Vx AND Vy
        void OP_8xy3(); //XOR Vx, Vy = Set Vx = Vx XOR Vy
        void OP_8xy4(); //ADD Vx, Vy = Set Vx = Vx + Vy, if result > 255 VF is set to 1. Stores lower 8 bits
        void OP_8xy5(); //SUB Vx, Vy = Set Vx = Vx - Vy, if Vx > Vy VF = 1,  otherwise 0
        void OP_8xy6(); //SHR Vx = Set Vx SHR 1, if the least significant bit is 1, VF = 1. (saved in VF) Then Vx / 2
        void OP_8xy7(); //SUBN Vx, Vy = Set Vx = Vy - Vx, If Vy > Vx then VF = 1
        void OP_8xyE(); //SHL Vx {, Vy} = Set Vx = Vx SHL 1
        void OP_9xy0(); //SNE Vx, Vy = Skips instruction if Vx != Vy
        void OP_Annn(); //LD I, addr = Set I = nnn
        void OP_Bnnn(); //JP v0, addr = Jumps to nnn + V0
        void OP_Cxkk(); //RND Vx, byte = Vx = random byte AND kk
        void OP_Dxyn(); //DRW Vx, Vy, nibble = Displays n byte sprite starting at memory location I (index) at (Vx, Vy). Set VF = collision
        void OP_Ex9E(); //SKP Vx =  Skip next instruction if key with Vx value is pressed
        void OP_ExA1(); //SKNP Vx = Skip next instruction if key with Vx value is not pressed
        void OP_Fx07(); //LD Vx, DT = Set Vx = delay timer value
        void OP_Fx0A(); //LD Vx, K = Wait for a key press, store the value of the key in LD
        void OP_Fx15(); //LD DT, Vx = Set delay timer = Vx
        void OP_Fx18(); //LD ST, Vx = Set sound timer = Vx
        void OP_Fx1E(); //ADD I, Vx = Set I = I + Vx
        void OP_Fx29(); //LD F, Vx = Set I = location of sprite for digit Vx
        void OP_Fx33(); //LD B, Vx = Store BCD representation of Vx in memory locations I, I+1, I+2 
        void OP_Fx55(); //LD [I], Vx = Store registers V0 through Vx in memory starting at location I
        void OP_Fx65(); //LD Vx, [I] = Read registers V0 through Vx from memory starting at location I

        void Table0();
        void Table8();
        void TableE();
        void TableF();
        void OP_NULL();
        
        void Cycle();

        uint8_t KEYPAD[16]; //16 keys from 0 to F. We need it in main so I set it up as public
        uint32_t MEM_BUFFER[64*32]; //Video buffer, same as keypad





    private:
        uint8_t V[16]; //System registers, V[0xF] is reserved to info about current operation
        uint8_t MEM[4096]; //Memory address designed to each byte of the MM
        uint16_t I_REG; //index register
        uint16_t PC; //Program Counter
        uint16_t STACK[16]; //16 levels stack
        uint8_t SP; //Stack Pointer, points to the top of the stack
        uint8_t D_TIMER; //Delay timer that reduces its value at a rate of 60hz if D_TIMER > 0
        uint8_t S_TIMER; //Sound timer that reduces its value at a rate of 60hz if S_TIMER > 0
        uint16_t OPCODE; //2 Bytes opcodes
        std::default_random_engine randGen; //random values generator seeded with the system clock's time used for the random byte generator
        std::uniform_int_distribution<uint8_t> randByte {0,255}; //random class object, random value between 0 and 255 when given a randGen

        //Function pointer tables, each subtable serves to identify different opcodes that start with 0, 8, E, or F
        //We will use the opcode bytes as an index into the function table array for scalability purposes

        Chip8func table[0xF + 1];
        Chip8func table0[0xE + 1];
        Chip8func table8[0xE + 1];
        Chip8func tableE[0xE + 1];
        Chip8func tableF[0x65 + 1];
};