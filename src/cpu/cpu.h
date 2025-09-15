#pragma once

#include "src/memory/memory.h"
#include <cstdint>
#include <sys/types.h>

class cpu{

    //gameboy 16 bit registers
    private:
    uint16_t af_register, bc_register, de_register, hl_register;

    // 7	z	Zero flag
    // 6	n	Subtraction flag (BCD)
    // 5	h	Half Carry flag (BCD)
    // 4	c	Carry flag

    uint16_t program_counter, stack_pointer;
    //memory module

    public:
    cpu();

    void init();
    void reset();
    void run(memory &memory);

    private:
    void execute(uint8_t instruction, memory &memory);
    void inc_instr();
    uint8_t inc_8bit_rgstr(uint8_t rgstr);
    uint8_t dec_8bit_rgstr(uint8_t rgstr);
};
