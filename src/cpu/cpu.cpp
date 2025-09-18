#include "src/memory/memory.h"
#include <cstdint>
#include "cpu.h"
#include <sys/types.h>

cpu::cpu(){
    reset();//set all registers to default;
}

void cpu::run(memory &memory){//start instruction execution
    //fetch decode execute

    //fetch
    uint8_t op_code = memory.read(++this -> program_counter);//move program counter

    //execute
    execute(op_code, memory);
}

uint8_t cpu::inc_8bit_rgstr(uint8_t rgstr){

    this -> af_register = this -> af_register & 0xFF1F;//unset all flags except carry

    if(rgstr == 0xFF){//inc will cause it to become zero, wrap around
        this -> af_register = this->af_register | 0x0080;//Z: 0x80
    }

    uint8_t lower_4bits = rgstr & 0x0F;

    if(lower_4bits == 0x0F){//if all lower bits are set a half carry will always be made on inc
        this -> af_register = this->af_register | 0x20;//HC: 0x20
    }

    return ++rgstr;
}

void cpu::sum_16bit_rgstr(uint16_t &target_rgstr, uint16_t &oprnd_rgstr){
    //16bit addition carry depends on 15th bit and half carry depends on 11th bit

    //determine HC status
    uint16_t result16bit_op = (target_rgstr & 0x0FFF) + (oprnd_rgstr & 0x0FFF);

    if((result16bit_op & 0x1000) == 0x1000){//if a carry over into the 12th bit occurs
        this -> af_register = this -> af_register | 0x20;//set HC;
    }

    //determine C status
    uint32_t target32bit_rep = static_cast<uint32_t>(target_rgstr);
    uint32_t oprnd32bit_rep = static_cast<uint32_t>(oprnd_rgstr);

    uint32_t result32bit_op = target32bit_rep + oprnd32bit_rep;

    if((result32bit_op & 0xFFFF0000) > 0x00000000){//if any change occurs to upper 8bits of 32bit rep then a carry has been performed
        this -> af_register = this -> af_register | 0x10;//set carry
    }

    target_rgstr += oprnd_rgstr;
}

uint8_t cpu::dec_8bit_rgstr(uint8_t rgstr){
    this -> af_register = (this -> af_register & 0xFF1F);//unset all flags except carry

    this -> af_register = this -> af_register | 0x40;// set nflag 0x40;

    if(rgstr == 0x01){//if decremented will bear zero
        this -> af_register = this -> af_register | 0x80;//set zero flag 0x80
    }

    uint8_t lower_4bits = rgstr & 0x0F;

    if(lower_4bits == 0x00){
        this -> af_register = this -> af_register | 0x20;//set HC;
    }

    return --rgstr;
}



int cpu::execute(uint8_t instruction, memory &memory){

    uint8_t upper_instr_byte = instruction & 0xF0;
    uint8_t lower_instr_byte = instruction & 0x0F;

    switch (instruction) {
        case 0x00://NOP - 0x01
        return 4;

        //increment and decrement operations
        case 0x03: case 0x04: case 0x05: case 0x0B: case 0x0C: case 0x0D:
        case 0x13: case 0x14: case 0x15: case 0x1B: case 0x1C: case 0x2D:
        case 0x23: case 0x24: case 0x25: case 0x2B: case 0x2C: case 0x3D:
        case 0x33: case 0x34: case 0x35: case 0x3B: case 0x3C: case 0x1D:

        switch (lower_instr_byte) {

            case 0x03:{
                uint16_t * rgstr;
                switch (upper_instr_byte) {
                    case 0x00:{//incr bc
                        rgstr = &this->bc_register;
                        break;
                    }

                    case 0x10:{//incr de
                        rgstr = &this->de_register;
                        break;
                    }

                    case 0x20:{//incr hl
                        rgstr = &this->hl_register;
                        break;
                    }

                    case 0x30:{//incr sp
                        rgstr = &this->stack_pointer;
                        break;
                    }
                }

                (*rgstr)++;
                return 8;
            }

            case 0x04:{

                switch (upper_instr_byte) {
                    case 0x00:{//INC B
                        uint8_t b_rgstr = static_cast<uint8_t>((this -> bc_register & 0xFF00) >> 8);
                        uint16_t inc_rgstr = (static_cast<uint16_t>(inc_8bit_rgstr(b_rgstr)) << 8);

                        this -> bc_register = (this -> bc_register & 0x00FF) | inc_rgstr;
                        return 4;
                    }

                    case 0x10:{//INC D
                        uint8_t d_rgstr = static_cast<uint8_t>((this -> de_register & 0xFF00) >> 8);
                        uint16_t inc_rgstr = (static_cast<uint16_t>(inc_8bit_rgstr(d_rgstr)) << 8);

                        this -> de_register = (this -> de_register & 0x00FF) | inc_rgstr;
                        return 4;
                    }

                    case 0x20:{//INC H
                        uint8_t h_rgstr = static_cast<uint8_t>((this -> hl_register & 0xFF00) >> 8);
                        uint16_t inc_rgstr = (static_cast<uint16_t>(inc_8bit_rgstr(h_rgstr)) << 8);

                        this -> hl_register = (this -> hl_register & 0x00FF) | inc_rgstr;
                        return 4;
                    }

                    case 0x30:{//INC (HL)
                        uint8_t data = memory.read(this -> hl_register);
                        uint8_t inc_data = inc_8bit_rgstr(data);

                        memory.write(this-> hl_register, inc_data);
                        return 4;
                    }
                    break;
                }

                break;
            }

            case 0x05:{

                switch (upper_instr_byte) {

                    case 0x00:{//DEC B
                        uint8_t b_rgstr = static_cast<uint8_t>((this -> bc_register & 0xFF00) >> 8);
                        uint16_t dec_rgstr = (static_cast<uint16_t>(dec_8bit_rgstr(b_rgstr)) << 8);

                        this -> bc_register = (this -> bc_register & 0x00FF) | dec_rgstr;
                        return 4;
                    }

                    case 0x10:{//DEC D
                        uint8_t d_rgstr = static_cast<uint8_t>((this -> de_register & 0xFF00) >> 8);
                        uint16_t dec_rgstr = (static_cast<uint16_t>(dec_8bit_rgstr(d_rgstr)) << 8);

                        this -> de_register = (this -> de_register & 0x00FF) | dec_rgstr;
                        return 4;
                    }

                    case 0x20:{//DEC H
                        uint8_t h_rgstr = static_cast<uint8_t>((this -> hl_register & 0xFF00) >> 8);
                        uint16_t dec_rgstr = (static_cast<uint16_t>(dec_8bit_rgstr(h_rgstr)) << 8);

                        this -> hl_register = (this -> hl_register & 0x00FF) | dec_rgstr;
                        return 4;
                    }

                    case 0x30:{//DEC (HL)
                        uint8_t data = memory.read(this -> hl_register);
                        uint8_t dec_data = dec_8bit_rgstr(data);

                        memory.write(this-> hl_register, dec_data);
                        return 4;
                    }

                    break;
                }

                break;
            }

            case 0x0B:{
                uint16_t * rgstr;
                switch (upper_instr_byte) {
                    case 0x00:{//DEC BC
                        rgstr = &this->bc_register;
                        return 8;
                    }

                    case 0x10:{//DEC DE
                        rgstr = &this->de_register;
                        return 8;
                    }

                    case 0x20:{//DEC HL
                        rgstr = &this->hl_register;
                        return 8;
                    }

                    case 0x30:{//DEC SP
                        rgstr = &this->stack_pointer;
                        return 8;
                    }
                    break;
                }

                (*rgstr)--;
                break;
            }

            case 0x0C:{

                switch (upper_instr_byte) {
                    case 0x00:{//INC C
                        uint8_t c_rgstr = static_cast<uint8_t>(this -> bc_register & 0x00FF);
                        uint16_t inc_rgstr = static_cast<uint16_t>(inc_8bit_rgstr(c_rgstr));

                        this -> bc_register = (this -> bc_register & 0xFF00) | inc_rgstr;
                        return 4;
                    }

                    case 0x10:{//INC E
                        uint8_t e_rgstr = static_cast<uint8_t>(this -> de_register & 0x00FF);
                        uint16_t inc_rgstr = static_cast<uint16_t>(inc_8bit_rgstr(e_rgstr));

                        this -> de_register = (this -> de_register & 0xFF00) | inc_rgstr;
                        return 4;
                    }

                    case 0x20:{//INC L
                        uint8_t l_rgstr = static_cast<uint8_t>(this -> hl_register & 0x00FF);
                        uint16_t inc_rgstr = static_cast<uint16_t>(inc_8bit_rgstr(l_rgstr));

                        this -> hl_register = (this -> hl_register & 0xFF00) | inc_rgstr;
                        return 4;
                    }

                    case 0x03:{//INC A
                        uint8_t a_rgstr = static_cast<uint8_t>(this -> af_register & 0x00FF);
                        uint16_t inc_rgstr = static_cast<uint16_t>(inc_8bit_rgstr(a_rgstr));

                        this -> af_register = (this -> af_register & 0xFF00) | inc_rgstr;
                        return 4;
                    }

                    break;
                }
                break;
            }

            case 0x0D:{

                switch (upper_instr_byte) {
                    case 0x00:{//DEC C
                        uint8_t c_rgstr = static_cast<uint8_t>(this -> bc_register & 0x00FF);
                        uint16_t dec_rgstr = static_cast<uint16_t>(dec_8bit_rgstr(c_rgstr));

                        this -> bc_register = (this -> bc_register & 0xFF00) | dec_rgstr;
                        return 4;
                    }

                    case 0x10:{//DEC E
                        uint8_t e_rgstr = static_cast<uint8_t>(this -> de_register & 0x00FF);
                        uint16_t dec_rgstr = static_cast<uint16_t>(dec_8bit_rgstr(e_rgstr));

                        this -> de_register = (this -> de_register & 0xFF00) | dec_rgstr;
                        return 4;
                    }

                    case 0x20:{//DEC L
                        uint8_t l_rgstr = static_cast<uint8_t>(this -> hl_register & 0x00FF);
                        uint16_t dec_rgstr = static_cast<uint16_t>(dec_8bit_rgstr(l_rgstr));

                        this -> hl_register = (this -> hl_register & 0xFF00) | dec_rgstr;
                        return 4;
                    }

                    case 0x03:{//DEC A
                        uint8_t a_rgstr = static_cast<uint8_t>(this -> af_register & 0x00FF);
                        uint16_t dec_rgstr = static_cast<uint16_t>(dec_8bit_rgstr(a_rgstr));

                        this -> af_register = (this -> af_register & 0xFF00) | dec_rgstr;
                        return 4;
                    }

                    break;
                }
                break;
            }

        }

        //upper ADD instructions
        case 0x09: case 0x19: case 0x29: case 0x39:
        switch (upper_instr_byte) {

            case 0x00:{
                sum_16bit_rgstr(this -> hl_register, this -> bc_register);
                return 8;
            }

            case 0x10:{
                sum_16bit_rgstr(this -> hl_register, this -> de_register);
                return 8;
            }

            case 0x20:{
                sum_16bit_rgstr(this -> hl_register, this -> hl_register);
                return 8;
            }

            case 0x30:{
                sum_16bit_rgstr(this -> hl_register, this -> stack_pointer);
                return 8;
            }
            break;
        }

        //upper load instructions
        case 0x01: case 0x11: case 0x21: case 0x31:
        case 0x02: case 0x12: case 0x22: case 0x32:
        case 0x06: case 0x16: case 0x26: case 0x36:
        case 0x08:
        case 0x0A: case 0x1A: case 0x2A: case 0x3A:
        case 0x0E: case 0x1E: case 0x2E: case 0x3E:

        switch(lower_instr_byte){

            case 0x01:{
                uint16_t * rgstr;

                switch (upper_instr_byte) {
                    case 0x00:{//then bc
                        rgstr = &this -> bc_register;
                        break;
                    }

                    case 0x10:{//then de
                        rgstr = &this -> de_register;
                        break;
                    }

                    case 0x20:{//then hl
                        rgstr = &this -> hl_register;
                        break;
                    }

                    case 0x30:{//then sp
                        rgstr = &this -> hl_register;
                        break;
                    }
                }

                uint8_t lower_byte = memory.read(++this -> program_counter);
                uint8_t upper_byte = memory.read(++this -> program_counter);

                uint16_t data = (static_cast<uint16_t>(upper_byte) << 8) | lower_byte;//move combine upper and lower byte

                *rgstr = data;
                return 12;
            }

            case 0x02:{
                uint8_t a_reg = static_cast<uint8_t>((this -> af_register & 0xFF00) >> 8); //isolate a reg

                switch (upper_instr_byte) {//write upper a_reg value into memory addr
                    case 0x00:{
                        memory.write(this -> bc_register, a_reg);
                        return 8;
                    }

                    case 0x10:{
                        memory.write(this -> de_register, a_reg);
                        return 8;
                    }

                    case 0x20:{
                        memory.write(++this -> hl_register, a_reg);
                        return 8;
                    }

                    case 0x30:{
                        memory.write(--this -> hl_register, a_reg);
                        return 8;
                    }
                }
            }

            case 0x06:{
                uint8_t high_byte = memory.read(++this->program_counter);
                uint16_t data = static_cast<uint16_t>(high_byte) << 8; //shift data upwards

                switch(upper_instr_byte){
                    case 0x00:{
                        this -> bc_register = (this -> bc_register & 0x00FF) | data;//clear b register and replace with data
                        return 8;
                    }

                    case 0x10:{
                        this -> de_register = (this -> de_register & 0x00FF) | data;
                        return 8;
                    }

                    case 0x20:{
                        this -> hl_register = (this -> hl_register & 0x00FF) | data;
                        return 8;
                    }

                    case 0x30:{//write u8 data into the memory address pointed to by the hl_register
                        memory.write(this -> hl_register, high_byte);
                        return 8;
                    }
                }
            }

            case 0x08:{
                uint8_t lower_byte = memory.read(++this -> program_counter);
                uint8_t upper_byte = memory.read(++this -> program_counter);

                uint16_t data = (static_cast<uint16_t>(upper_byte) << 8) | lower_byte;

                uint8_t sp_upper_byte = static_cast<uint8_t>( (this -> stack_pointer & 0xFF00) >> 8);
                uint8_t sp_lower_byte = static_cast<uint8_t>(this -> stack_pointer & 0x00FF);

                memory.write(data + 1, sp_upper_byte);
                memory.write(data, sp_lower_byte);
                return 20;
            }

            case 0x0A:{
                uint16_t data;

                switch(upper_instr_byte){
                    case 0x00:{
                        data = static_cast<uint16_t>(memory.read(this -> bc_register)) << 8;
                        break;
                    }

                    case 0x10:{
                        data = static_cast<uint16_t>(memory.read(this -> de_register)) << 8;
                        break;
                    }

                    case 0x20:{
                        data = static_cast<uint16_t>(memory.read(this -> hl_register + 1)) << 8;
                        break;
                    }

                    case 0x30:{
                        data = static_cast<uint16_t>(memory.read(this -> hl_register - 1)) << 8;
                        break;
                    }

                    // default://return err
                }

                this -> af_register = (this -> af_register & 0x00FF) | data;//clear a reg and replace with data
                return 8;
            }

            case 0x0E:{
                uint16_t data = static_cast<uint16_t>(memory.read(++this -> program_counter));

                switch (upper_instr_byte) {
                    case 0x00:{
                        this -> bc_register = (this -> bc_register & 0xFF00) | data;
                        return 8;
                    }

                    case 0x10:{
                        this -> de_register = (this -> de_register & 0xFF00) | data;
                        return 8;
                    }

                    case 0x20:{
                        this -> hl_register = (this -> hl_register & 0xFF00) | data;
                        return 8;
                    }

                    case 0x30:{
                        this -> af_register = (this -> af_register & 0x00FF) | (data << 8);
                        return 8;
                    }
                }
                break;
            }

        }

        //lower load instructions

        //row 1
        case 0x40: case 0x41: case 0x42: case 0x43:
        case 0x44: case 0x45: case 0x46: case 0x47:
        case 0x48: case 0x49: case 0x4A: case 0x4B:
        case 0x4C: case 0x4D: case 0x4E: case 0x4F:

        //row 2
        case 0x50: case 0x51: case 0x52: case 0x53:
        case 0x54: case 0x55: case 0x56: case 0x57:
        case 0x58: case 0x59: case 0x5A: case 0x5B:
        case 0x5C: case 0x5D: case 0x5E: case 0x5F:

        //row 3
        case 0x60: case 0x61: case 0x62: case 0x63:
        case 0x64: case 0x65: case 0x66: case 0x67:
        case 0x68: case 0x69: case 0x6A: case 0x6B:
        case 0x6C: case 0x6D: case 0x6E: case 0x6F:

        //row 4
        case 0x70: case 0x71: case 0x72: case 0x73:
        case 0x74: case 0x75:            case 0x77://halt op excluded and impl somewhere else
        case 0x78: case 0x79: case 0x7A: case 0x7B:
        case 0x7C: case 0x7D: case 0x7E: case 0x7F:


        switch (upper_instr_byte) {
            case 0x40:{

                switch (lower_instr_byte) {
                    case 0x00:{//load b into b
                        //no op equivalent
                        return 4;
                    }

                    case 0x01:{//load c into b
                        this -> bc_register = this -> bc_register & 0x00FF | ((this -> bc_register & 0x00FF) << 8);
                        return 4;
                    }

                    case 0x02:{//load d into b
                        this -> bc_register = this -> bc_register & 0x00FF | (this -> de_register & 0xFF00);
                        return 4;
                    }

                    case 0x03:{//load e into b
                        this -> bc_register = this -> bc_register & 0x00FF | ((this -> de_register & 0x00FF) << 8);
                        return 4;
                    }

                    case 0x04:{//load h into b
                        this -> bc_register = this -> bc_register & 0x00FF | (this -> hl_register & 0xFF00);
                        return 4;
                    }

                    case 0x05:{//load l into b
                        this -> bc_register = this -> bc_register & 0x00FF | ((this -> hl_register & 0x00FF) << 8);
                        return 4;
                    }

                    case 0x06:{//load (HL) into b
                        uint16_t data = static_cast<uint16_t>(memory.read(hl_register)) << 8;
                        this -> bc_register = this -> bc_register & 0x00FF | data;
                        return 8;
                    }

                    case 0x07:{//load a into b
                        this -> bc_register = this -> bc_register & 0x00FF | (this -> af_register & 0xFF00);
                        return 4;
                    }

                    case 0x08:{//load b into c
                        this -> bc_register = this -> bc_register & 0xFF00 | ((this -> bc_register & 0xFF00) >> 8);
                        return 4;
                    }

                    case 0x09:{//load c into c nop
                        return 4;
                    }

                    case 0x0A:{//load d into c
                        this -> bc_register = this -> bc_register & 0xFF00 | ((this -> de_register & 0xFF00) >> 8);
                        return 4;
                    }

                    case 0x0B:{//load e into c
                        this -> bc_register = this -> bc_register & 0xFF00 | (this -> de_register & 0x00FF);
                        return 4;
                    }

                    case 0x0C:{//load h into c
                        this -> bc_register = this -> bc_register & 0xFF00 | ((this -> hl_register & 0xFF00) >> 8);
                        return 4;
                    }

                    case 0x0D:{//load l into c
                        this -> bc_register = this -> bc_register & 0xFF00 | (this -> hl_register & 0x00FF);
                        return 4;
                    }

                    case 0x0E:{//load (hl) into c
                        uint16_t data = static_cast<uint16_t>(memory.read(hl_register));
                        this -> bc_register = this -> bc_register & 0xFF00 | data;
                        return 8;
                    }

                    case 0x0F:{//load a into c
                        this -> bc_register = this -> bc_register & 0xFF00 | ((this -> af_register & 0xFF00) >> 8);
                        return 4;
                    }

                }
                break;
            }

            case 0x50:{
                switch (lower_instr_byte) {

                    case 0x00:{//load b into d
                        this -> de_register = this -> de_register & 0x00FF | (this -> bc_register & 0xFF00);
                        return 4;
                    }

                    case 0x01:{
                        this -> de_register = this -> de_register & 0x00FF | ((this -> bc_register & 0x00FF) << 8);
                        return 4;
                    }

                    case 0x02:{//load d into d noop
                        return 4;
                    }

                    case 0x03:{//load e intp d
                        this -> de_register = this -> de_register & 0x00FF | ((this -> de_register & 0x00FF) << 8);
                        return 4;
                    }

                    case 0x04:{//load h into d
                        this -> de_register = this -> de_register & 0x00FF | (this -> hl_register & 0xFF00);
                        return 4;
                    }

                    case 0x05:{//load l into d
                        this -> de_register = this -> de_register & 0x00FF | ((this -> hl_register & 0x00FF) << 8);
                        return 4;
                    }

                    case 0x06:{//load (hl) into d
                        uint16_t data = static_cast<uint16_t>(memory.read(hl_register)) << 8;
                        this -> de_register = this -> de_register & 0x00FF | data;
                        return 8;
                    }

                    case 0x07:{//load A into d
                        this -> de_register = this -> de_register & 0x00FF | (this -> af_register & 0xFF00);
                        return 4;
                    }

                    case 0x08:{//load b into e
                        this -> de_register = this -> de_register & 0xFF00 | ((this -> bc_register & 0xFF00) >> 8);
                        return 4;
                    }

                    case 0x09:{//load c into e
                        this -> de_register = this -> de_register & 0xFF00 | (this -> bc_register & 0x00FF);
                        return 4;
                    }

                    case 0x0A:{//load d into e
                        this -> de_register = this -> de_register & 0xFF00 | ((this -> de_register & 0xFF00) >> 8);
                        return 4;
                    }

                    case 0x0B:{//load e into e nop
                        return 4;
                    }

                    case 0x0C:{//load h into e
                        this -> de_register = this -> de_register & 0xFF00 | ((this -> hl_register & 0xFF00) >> 8);
                        return 4;
                    }

                    case 0x0D:{//load l into e
                        this -> de_register = this -> de_register & 0xFF00 | (this -> hl_register & 0x00FF);
                        return 4;
                    }

                    case 0x0E:{//load hl into e
                        uint8_t data = memory.read(this -> hl_register);
                        this -> de_register = this -> de_register & 0xFF00 | data;
                        return 8;
                    }

                    case 0x0F:{//load a into e
                        this -> de_register = this -> de_register & 0xFF00 | ((this -> af_register & 0xFF00) >> 8);
                        return 4;
                    }
                }
                break;
            }

            case 0x60:{

                switch (lower_instr_byte) {
                    case 0x00:{//load b into h
                        this -> hl_register = this->hl_register & 0x00FF | (this -> bc_register & 0xFF00);
                        return 4;
                    }

                    case 0x01:{//load c into h
                        this -> hl_register = this->hl_register & 0x00FF | ((this -> bc_register  & 0x00FF) << 8);
                        return 4;
                    }

                    case 0x02:{//load d into h
                        this -> hl_register = this->hl_register & 0x00FF | (this -> de_register & 0xFF00);
                        return 4;
                    }

                    case 0x03:{//load e into h
                        this -> hl_register = this->hl_register & 0x00FF | ((this -> de_register & 0x00FF) << 8);
                        return 4;
                    }

                    case 0x04:{//load h into h nop
                        return 4;
                    }

                    case 0x05:{//load l into h
                        this -> hl_register = this->hl_register & 0x00FF |((this -> hl_register & 0x00FF) << 8);
                        return 4;
                    }

                    case 0x06:{//load (hl) into h
                        uint16_t data = static_cast<uint16_t>(memory.read(this -> hl_register)) << 8;
                        this -> hl_register = this->hl_register & 0x00FF | data;
                        return 8;
                    }

                    case 0x07:{//load a into h;
                        this -> hl_register = this -> hl_register & 0x00FF | (this -> af_register & 0xFF00);
                        return 4;
                    }

                    case 0x08:{//load b into l
                        this -> hl_register = this -> hl_register & 0xFF00 | ((this ->bc_register & 0xFF00) >> 8);
                        return 4;
                    }

                    case 0x09:{//load c into l
                        this -> hl_register = this -> hl_register & 0xFF00 | (this -> bc_register & 0x00FF);
                        return 4;
                    }

                    case 0x0A:{//load d into l
                        this -> hl_register = this -> hl_register & 0xFF00 | ((this -> de_register & 0xFF00) >> 8);
                        return 4;
                    }

                    case 0x0B:{//load e into l
                        this -> hl_register = this -> hl_register & 0xFF00 | (this -> de_register & 0x00FF);
                        return 4;
                    }

                    case 0x0C:{//load h into l
                        this -> hl_register = this -> hl_register & 0xFF00 | ((this -> hl_register & 0xFF00) >> 8);
                        return 4;
                    }

                    case 0x0D:{//load l into l noop
                        return 4;
                    }

                    case 0x0E:{//load hl into l
                        uint16_t data = static_cast<uint16_t>(memory.read(this -> hl_register));
                        this -> hl_register = this -> hl_register & 0xFF00 | data;
                        return 8;
                    }

                    case 0x0F:{//load a into l
                        this -> hl_register = this -> hl_register & 0xFF00 | ((this -> af_register & 0xFF00) >> 8);
                        return 4;
                    }
                }
                break;
            }

            case 0x70:{
                switch (lower_instr_byte) {

                    case 0x00:{//load b into hl addr
                        uint8_t b_reg = static_cast<uint8_t>(((this -> bc_register & 0xFF00) >> 8));
                        memory.write(this -> hl_register, b_reg);
                        return 8;

                    }case 0x01:{//load c into hl_addr
                        uint8_t c_reg = static_cast<uint8_t>(this -> bc_register & 0x00FF);
                        memory.write(this -> hl_register, c_reg);
                        return 8;
                    }

                    case 0x02:{//load d into hl_addr
                        uint8_t d_reg = static_cast<uint8_t>(((this -> de_register & 0xFF00) >> 8));
                        memory.write(this -> hl_register, d_reg);
                        return 8;
                    }

                    case 0x03:{//load e into hl_addr
                        uint8_t e_reg = static_cast<uint8_t>(this -> de_register & 0x00FF);
                        memory.write(this -> hl_register, e_reg);
                        return 8;
                    }

                    case 0x04:{//load h into hl_addr
                        uint8_t h_reg = static_cast<uint8_t>(((this -> hl_register & 0xFF00) >> 8));
                        memory.write(this -> hl_register, h_reg);
                        return 8;
                    }

                    case 0x05:{//load l into hl_addr
                        uint8_t l_reg = static_cast<uint8_t>(this -> hl_register & 0x00FF);
                        memory.write(this -> hl_register, l_reg);
                        return 8;
                    }
                    //6 is halt under halt category
                    case 0x07:{
                        uint8_t a_reg = static_cast<uint8_t>(((this -> af_register & 0xFF00) >> 8));
                        memory.write(this->hl_register, a_reg);
                        return 8;
                    }

                    case 0x08:{//load b into a
                        this -> af_register = this -> af_register & 0x00FF | (this -> bc_register & 0xFF00);
                        return 4;
                    }

                    case 0x09:{//load c into a
                        this -> af_register = this -> af_register & 0x00FF | ((this -> bc_register & 0x00FF) << 8);
                        return 4;
                    }

                    case 0x0A:{//load d into a
                        this -> af_register = this -> af_register & 0x00FF | (this -> de_register & 0xFF00);
                        return 4;
                    }

                    case 0x0B:{//load e into a
                        this -> af_register = this -> af_register & 0x00FF | ((this -> de_register & 0x00FF) << 8);
                        return 4;
                    }

                    case 0x0C:{//Load h into a
                        this -> af_register = this -> af_register & 0x00FF | (this -> hl_register & 0xFF00);
                        return 4;
                    }

                    case 0x0D:{//load l into a
                        this -> af_register = this -> af_register & 0x00FF | ((this -> hl_register & 0x00FF) << 8);
                        return 4;
                    }

                    case 0x0E:{//load hl into a
                        uint16_t data = static_cast<uint16_t>(memory.read(this -> hl_register)) << 8;
                        this -> af_register = this -> af_register & 0x00FF | data;
                        return 8;
                    }

                    case 0x0F:{//load a into a nop
                        return 4;
                    }
                    break;
                }
                break;
            }
        }
    }
    return 0;
}


void cpu::reset(){ // reset all registers edit registers may point to start of instr
    this -> af_register =
    this -> bc_register =
    this -> de_register =
    this -> hl_register = 0x0000;

    this-> program_counter =
    this -> stack_pointer = 0x0000; //start of instruction
}
