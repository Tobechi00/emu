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
    int execute(uint8_t instruction, memory &memory);
    void inc_instr();

    uint8_t inc_8bit_rgstr(uint8_t rgstr);

    uint8_t dec_8bit_rgstr(uint8_t rgstr);

    //logic op methods
    void add_16bit_rgstr(uint16_t &target_rgstr, uint16_t &oprnd_rgstr);
    uint8_t add_8bit_rgstr(uint8_t target_rgstr, uint8_t oprnd_rgstr);
    uint8_t sub_8bit_rgstr(uint8_t target_rgstr, uint8_t oprnd_rgstr);
    uint8_t adc_8bit_rgstr(uint8_t target_rgstr, uint8_t oprnd_rgstr);
    uint8_t sbc_8bit_rgstr(uint8_t target_rgstr, uint8_t oprnd_rgstr);
    uint8_t and_8bit_rgstr(uint8_t target_rgstr, uint8_t oprnd_rgstr);
    uint8_t xor_8bit_rgstr(uint8_t target_rgstr, uint8_t oprnd_rgstr);
    uint8_t or_8bit_rgstr(uint8_t target_rgstr, uint8_t oprnd_rgstr);
    void cp_8bit_rgstr(uint8_t target_rgstr, uint8_t oprnd_rgstr);
    void ret_instr(memory &memory);
    void jp_instr(memory &memory);
    void rst_instr(memory &memory, uint8_t p_val);


    uint8_t get_a_reg();
    uint8_t get_b_reg();
    uint8_t get_c_reg();
    uint8_t get_d_reg();
    uint8_t get_e_reg();
    uint8_t get_h_reg();
    uint8_t get_l_reg();

    //flag setters
    void set_z_flag();
    void set_n_flag();
    void set_hc_flag();
    void set_c_flag();

    uint8_t get_z_flag();
    uint8_t get_n_flag();
    uint8_t get_hc_flag();
    uint8_t get_c_flag();

    void set_a_reg(uint8_t val);

    void unset_c_flag();
    void unset_all_flags();
};
