#include "src/memory/memory.h"
#include <cstdint>
#include "cpu.h"
#include <sys/types.h>

#define Z_FLAG   0x0080;
#define HC_FLAG  0x0020;
#define N_FLAG   0x0040;
    
    

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

    this -> af_register = this -> af_register & 0xFF10;//unset Z N HC


    if(rgstr == 0xFF){//inc will cause it to become zero, wrap around
        this -> af_register = this->af_register | Z_FLAG;
    }

    uint8_t lower_byte = rgstr & 0x0F;

    if(lower_byte == 0x0F){//if all lower bits are set or the three last lower bits a half carry will always be made on inc
        this -> af_register = this->af_register | HC_FLAG;
    }

    return ++rgstr;
}

// uint8_t cpu::dec_8bit_rgstr(uint8_t rgstr){
//     this -> af_register = this -> af_register & 0xFF10;//unset Z N HC

//     this -> af_register = this -> af_register | N_FLAG;

//     if(rgstr == 0x00){//dec will cause it to become 0xFF, wrap around
//         this -> af_register = this->af_register | Z_FLAG;//Z: 0x80
//     }

//     if(rgstr == 0x10){ rework 
//         this -> af_register = this->af_register | HC_FLAG;//HC: 0x20
//     }
    

//     return --rgstr;
// }

void cpu::execute(uint8_t instruction, memory &memory){

    uint8_t upper_instr_byte = instruction & 0xF0;
    uint8_t lower_instr_byte = instruction & 0x0F;

    switch (instruction) {
        case 0x00://NOP - 0x01
        break;

        //increment and decrement op
        case 0x03: case 0x04: case 0x05: case 0x0B: case 0x0C: case 0x0D:
        case 0x13: case 0x14: case 0x15: case 0x1B: case 0x1C: case 0x2D:
        case 0x23: case 0x24: case 0x25: case 0x2B: case 0x2C: case 0x3D:
        case 0x33: case 0x34: case 0x35: case 0x3B: case 0x3C: case 0x1D:

        switch (lower_instr_byte) {
            uint16_t * rgstr;

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
                break;
            }

            case 0x04:{

                switch (upper_instr_byte) {
                    case 0x00:{//INC B
                        uint8_t b_reg = static_cast<uint8_t>((this -> bc_register & 0xFF00) >> 8);
                        uint8_t inc_rgstr = inc_8bit_rgstr(b_reg);

                        this -> bc_register = (this -> bc_register & 0x00FF) | static_cast<uint16_t>(inc_rgstr) << 8;
                        break;
                    }

                    case 0x10:{//INC D
                        uint8_t d_reg = static_cast<uint8_t>((this -> de_register & 0xFF00) >> 8);
                        uint8_t inc_rgstr = inc_8bit_rgstr(d_reg);

                        this -> de_register = (this -> de_register & 0x00FF) | static_cast<uint16_t>(inc_rgstr) << 8;
                        break;
                    }

                    case 0x20:{//INC H
                        uint8_t h_reg = static_cast<uint8_t>((this -> hl_register & 0xFF00) >> 8);
                        uint8_t inc_rgstr = inc_8bit_rgstr(h_reg);

                        this -> hl_register = (this -> hl_register & 0x00FF) | static_cast<uint16_t>(inc_rgstr) << 8;
                        break;
                    }

                    case 0x30:{//INC HL
                        uint8_t data = memory.read(this -> hl_register);
                        uint8_t inc_data = inc_8bit_rgstr(data);

                        memory.write(this -> hl_register, inc_data);
                        break;
                    }
                }
                break;
            }

            case 0x05:{

                switch(upper_instr_byte){
                    case 0x00:{

                    }

                    case 0x10:{

                    }

                    case 0x20:{

                    }

                    case 0x30:{

                    }
                }
                break;
            }

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
                break;
            }

            case 0x02:{
                uint8_t a_reg = static_cast<uint8_t>((this -> af_register & 0xFF00) >> 8); //isolate a reg

                switch (upper_instr_byte) {//write upper a_reg value into memory addr
                    case 0x00:{
                        memory.write(this -> bc_register, a_reg);
                        break;
                    }

                    case 0x10:{
                        memory.write(this -> de_register, a_reg);
                        break;
                    }

                    case 0x20:{
                        memory.write(++this -> hl_register, a_reg);
                        break;
                    }

                    case 0x30:{
                        memory.write(--this -> hl_register, a_reg);
                        break;
                    }
                }
            }

            case 0x06:{
                uint8_t high_byte = memory.read(++this->program_counter);
                uint16_t data = static_cast<uint16_t>(high_byte) << 8; //shift data upwards

                switch(upper_instr_byte){
                    case 0x00:{
                        this -> bc_register = (this -> bc_register & 0x00FF) | data;//clear b register and replace with data
                        break;
                    }

                    case 0x10:{
                        this -> de_register = (this -> de_register & 0x00FF) | data;
                        break;
                    }

                    case 0x20:{
                        this -> hl_register = (this -> hl_register & 0x00FF) | data;
                        break;
                    }

                    case 0x30:{//write u8 data into the memory address pointed to by the hl_register
                        memory.write(this -> hl_register, high_byte);
                        break;
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
                break;
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
                break;
            }

            case 0x0E:{
                uint16_t data = static_cast<uint16_t>(memory.read(++this -> program_counter));

                switch (upper_instr_byte) {
                    case 0x00:{
                        this -> bc_register = (this -> bc_register & 0xFF00) | data;
                        break;
                    }

                    case 0x10:{
                        this -> de_register = (this -> de_register & 0xFF00) | data;
                    }

                    case 0x20:{
                        this -> hl_register = (this -> hl_register & 0xFF00) | data;
                    }

                    case 0x30:{
                        this -> af_register = (this -> af_register & 0x00FF) | (data << 8);
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
        case 0x74: case 0x75:            case 0x77://halt op excluded an impl somewhere else
        case 0x78: case 0x79: case 0x7A: case 0x7B:
        case 0x7C: case 0x7D: case 0x7E: case 0x7F:


        switch (upper_instr_byte) {
            case 0x40:{

                switch (lower_instr_byte) {
                    case 0x00:{//load b into b
                        //no op equivalent
                        break;
                    }

                    case 0x01:{//load c into b
                        this -> bc_register = this -> bc_register & 0x00FF | ((this -> bc_register & 0x00FF) << 8);
                        break;
                    }

                    case 0x02:{//load d into b
                        this -> bc_register = this -> bc_register & 0x00FF | (this -> de_register & 0xFF00);
                        break;
                    }

                    case 0x03:{//load e into b
                        this -> bc_register = this -> bc_register & 0x00FF | ((this -> de_register & 0x00FF) << 8);
                        break;
                    }

                    case 0x04:{//load h into b
                        this -> bc_register = this -> bc_register & 0x00FF | (this -> hl_register & 0xFF00);
                        break;
                    }

                    case 0x05:{//load l into b
                        this -> bc_register = this -> bc_register & 0x00FF | ((this -> hl_register & 0x00FF) << 8);
                        break;
                    }

                    case 0x06:{//load (HL) into b
                        uint16_t data = static_cast<uint16_t>(memory.read(hl_register)) << 8;
                        this -> bc_register = this -> bc_register & 0x00FF | data;
                        break;
                    }

                    case 0x07:{//load a into b
                        this -> bc_register = this -> bc_register & 0x00FF | (this -> af_register & 0xFF00);
                        break;
                    }

                    case 0x08:{//load b into c
                        this -> bc_register = this -> bc_register & 0xFF00 | ((this -> bc_register & 0xFF00) >> 8);
                        break;
                    }

                    case 0x09:{//load c into c nop
                        break;
                    }

                    case 0x0A:{//load d into c
                        this -> bc_register = this -> bc_register & 0xFF00 | ((this -> de_register & 0xFF00) >> 8);
                        break;
                    }

                    case 0x0B:{//load e into c
                        this -> bc_register = this -> bc_register & 0xFF00 | (this -> de_register & 0x00FF);
                        break;
                    }

                    case 0x0C:{//load h into c
                        this -> bc_register = this -> bc_register & 0xFF00 | ((this -> hl_register & 0xFF00) >> 8);
                        break;
                    }

                    case 0x0D:{//load l into c
                        this -> bc_register = this -> bc_register & 0xFF00 | (this -> hl_register & 0x00FF);
                        break;
                    }

                    case 0x0E:{//load (hl) into c
                        uint16_t data = static_cast<uint16_t>(memory.read(hl_register));
                        this -> bc_register = this -> bc_register & 0xFF00 | data;
                        break;
                    }

                    case 0x0F:{//load a into c
                        this -> bc_register = this -> bc_register & 0xFF00 | ((this -> af_register & 0xFF00) >> 8);
                        break;
                    }

                }
                break;
            }

            case 0x50:{
                switch (lower_instr_byte) {

                    case 0x00:{//load b into d
                        this -> de_register = this -> de_register & 0x00FF | (this -> bc_register & 0xFF00);
                        break;
                    }

                    case 0x01:{
                        this -> de_register = this -> de_register & 0x00FF | ((this -> bc_register & 0x00FF) << 8);
                        break;
                    }

                    case 0x02:{//load d into d noop
                        break;
                    }

                    case 0x03:{//load e intp d
                        this -> de_register = this -> de_register & 0x00FF | ((this -> de_register & 0x00FF) << 8);
                        break;
                    }

                    case 0x04:{//load h into d
                        this -> de_register = this -> de_register & 0x00FF | (this -> hl_register & 0xFF00);
                        break;
                    }

                    case 0x05:{//load l into d
                        this -> de_register = this -> de_register & 0x00FF | ((this -> hl_register & 0x00FF) << 8);
                        break;
                    }

                    case 0x06:{//load (hl) into d
                        uint16_t data = static_cast<uint16_t>(memory.read(hl_register)) << 8;
                        this -> de_register = this -> de_register & 0x00FF | data;
                        break;
                    }

                    case 0x07:{//load A into d
                        this -> de_register = this -> de_register & 0x00FF | (this -> af_register & 0xFF00);
                        break;
                    }

                    case 0x08:{//load b into e
                        this -> de_register = this -> de_register & 0xFF00 | ((this -> bc_register & 0xFF00) >> 8);
                        break;
                    }

                    case 0x09:{//load c into e
                        this -> de_register = this -> de_register & 0xFF00 | (this -> bc_register & 0x00FF);
                        break;
                    }

                    case 0x0A:{//load d into e
                        this -> de_register = this -> de_register & 0xFF00 | ((this -> de_register & 0xFF00) >> 8);
                        break;
                    }

                    case 0x0B:{//load e into e nop
                        break;
                    }

                    case 0x0C:{//load h into e
                        this -> de_register = this -> de_register & 0xFF00 | ((this -> hl_register & 0xFF00) >> 8);
                        break;
                    }

                    case 0x0D:{//load l into e
                        this -> de_register = this -> de_register & 0xFF00 | (this -> hl_register & 0x00FF);
                        break;
                    }

                    case 0x0E:{//load hl into e
                        uint8_t data = memory.read(this -> hl_register);
                        this -> de_register = this -> de_register & 0xFF00 | data;
                        break;
                    }

                    case 0x0F:{//load a into e
                        this -> de_register = this -> de_register & 0xFF00 | ((this -> af_register & 0xFF00) >> 8);
                        break;
                    }
                }
                break;
            }

            case 0x60:{

                switch (lower_instr_byte) {
                    case 0x00:{//load b into h
                        this -> hl_register = this->hl_register & 0x00FF | (this -> bc_register & 0xFF00);
                        break;
                    }

                    case 0x01:{//load c into h
                        this -> hl_register = this->hl_register & 0x00FF | ((this -> bc_register  & 0x00FF) << 8);
                        break;
                    }

                    case 0x02:{//load d into h
                        this -> hl_register = this->hl_register & 0x00FF | (this -> de_register & 0xFF00);
                        break;
                    }

                    case 0x03:{//load e into h
                        this -> hl_register = this->hl_register & 0x00FF | ((this -> de_register & 0x00FF) << 8);
                        break;
                    }

                    case 0x04:{//load h into h nop
                        break;
                    }

                    case 0x05:{//load l into h
                        this -> hl_register = this->hl_register & 0x00FF |((this -> hl_register & 0x00FF) << 8);
                        break;
                    }

                    case 0x06:{//load (hl) into h
                        uint16_t data = static_cast<uint16_t>(memory.read(this -> hl_register)) << 8;
                        this -> hl_register = this->hl_register & 0x00FF | data;
                        break;
                    }

                    case 0x07:{//load a into h;
                        this -> hl_register = this -> hl_register & 0x00FF | (this -> af_register & 0xFF00);
                        break;
                    }

                    case 0x08:{//load b into l
                        this -> hl_register = this -> hl_register & 0xFF00 | ((this ->bc_register & 0xFF00) >> 8);
                        break;
                    }

                    case 0x09:{//load c into l
                        this -> hl_register = this -> hl_register & 0xFF00 | (this -> bc_register & 0x00FF);
                        break;
                    }

                    case 0x0A:{//load d into l
                        this -> hl_register = this -> hl_register & 0xFF00 | ((this -> de_register & 0xFF00) >> 8);
                        break;
                    }

                    case 0x0B:{//load e into l
                        this -> hl_register = this -> hl_register & 0xFF00 | (this -> de_register & 0x00FF);
                        break;
                    }

                    case 0x0C:{//load h into l
                        this -> hl_register = this -> hl_register & 0xFF00 | ((this -> hl_register & 0xFF00) >> 8);
                        break;
                    }

                    case 0x0D:{//load l into l noop
                        break;
                    }

                    case 0x0E:{//load hl into l
                        uint16_t data = static_cast<uint16_t>(memory.read(this -> hl_register));
                        this -> hl_register = this -> hl_register & 0xFF00 | data;
                        break;
                    }

                    case 0x0F:{//load a into l
                        this -> hl_register = this -> hl_register & 0xFF00 | ((this -> af_register & 0xFF00) >> 8);
                        break;
                    }
                }
                break;
            }

            case 0x70:{
                switch (lower_instr_byte) {

                    case 0x00:{//load b into hl addr
                        uint8_t b_reg = static_cast<uint8_t>(((this -> bc_register & 0xFF00) >> 8));
                        memory.write(this -> hl_register, b_reg);
                        break;

                    }case 0x01:{//load c into hl_addr
                        uint8_t c_reg = static_cast<uint8_t>(this -> bc_register & 0x00FF);
                        memory.write(this -> hl_register, c_reg);
                        break;
                    }

                    case 0x02:{//load d into hl_addr
                        uint8_t d_reg = static_cast<uint8_t>(((this -> de_register & 0xFF00) >> 8));
                        memory.write(this -> hl_register, d_reg);
                        break;
                    }

                    case 0x03:{//load e into hl_addr
                        uint8_t e_reg = static_cast<uint8_t>(this -> de_register & 0x00FF);
                        memory.write(this -> hl_register, e_reg);
                        break;
                    }

                    case 0x04:{//load h into hl_addr
                        uint8_t h_reg = static_cast<uint8_t>(((this -> hl_register & 0xFF00) >> 8));
                        memory.write(this -> hl_register, h_reg);
                        break;
                    }

                    case 0x05:{//load l into hl_addr
                        uint8_t l_reg = static_cast<uint8_t>(this -> hl_register & 0x00FF);
                        memory.write(this -> hl_register, l_reg);
                        break;
                    }
                    //6 is halt under halt category
                    case 0x07:{
                        uint8_t a_reg = static_cast<uint8_t>(((this -> af_register & 0xFF00) >> 8));
                        memory.write(this->hl_register, a_reg);
                        break;
                    }

                    case 0x08:{//load b into a
                        this -> af_register = this -> af_register & 0x00FF | (this -> bc_register & 0xFF00);
                        break;
                    }

                    case 0x09:{//load c into a
                        this -> af_register = this -> af_register & 0x00FF | ((this -> bc_register & 0x00FF) << 8);
                        break;
                    }

                    case 0x0A:{//load d into a
                        this -> af_register = this -> af_register & 0x00FF | (this -> de_register & 0xFF00);
                        break;
                    }

                    case 0x0B:{//load e into a
                        this -> af_register = this -> af_register & 0x00FF | ((this -> de_register & 0x00FF) << 8);
                        break;
                    }

                    case 0x0C:{//Load h into a
                        this -> af_register = this -> af_register & 0x00FF | (this -> hl_register & 0xFF00);
                        break;
                    }

                    case 0x0D:{//load l into a
                        this -> af_register = this -> af_register & 0x00FF | ((this -> hl_register & 0x00FF) << 8);
                        break;
                    }

                    case 0x0E:{//load hl into a
                        uint16_t data = static_cast<uint16_t>(memory.read(this -> hl_register)) << 8;
                        this -> af_register = this -> af_register & 0x00FF | data;
                        break;
                    }

                    case 0x0F:{//load a into a nop
                        break;
                    }
                    break;
                }
                break;
            }
        }

        case

        break;
    }
}


void cpu::reset(){ // reset all registers edit registers may point to start of instr
    this -> af_register =
    this -> bc_register =
    this -> de_register =
    this -> hl_register = 0x0000;

    this-> program_counter =
    this -> stack_pointer = 0x0000; //start of instruction
}
