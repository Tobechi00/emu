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


//GETTERS AND SETTERS//
uint8_t cpu::get_a_reg(){
    return static_cast<uint8_t>((this-> af_register & 0xFF00) >> 8);
}

uint8_t cpu::get_b_reg(){
    return static_cast<uint8_t>((this -> bc_register & 0xFF00) >> 8);
}

uint8_t cpu::get_c_reg(){
    return static_cast<uint8_t>(this -> bc_register);
}

uint8_t cpu::get_d_reg(){
    return static_cast<uint8_t>((this -> de_register & 0xFF00) >> 8);
}

uint8_t cpu::get_e_reg(){
    return static_cast<uint8_t>(this -> de_register);
}

uint8_t cpu::get_h_reg(){
    return static_cast<uint8_t>((this -> hl_register & 0xFF00) >> 8);
}

uint8_t cpu::get_l_reg(){
    return static_cast<uint8_t>(this -> hl_register);
}

void cpu::set_z_flag(){
    this -> af_register = this ->   af_register | 0x0080;
}

void cpu::set_n_flag(){
    this -> af_register = this->   af_register | 0x0040;
}

void cpu::set_hc_flag(){
    this -> af_register = this->  af_register | 0x0020;//set hc
}

void cpu::set_c_flag(){
    this -> af_register = this ->  af_register | 0x0010;//set carry
}

void cpu::unset_c_flag(){
    this -> af_register = this ->   af_register & 0xFFEF;
}

void cpu::unset_all_flags(){
    this -> af_register = this -> af_register & 0xFF0F;
}

void cpu::set_a_reg(uint8_t val){
    this -> af_register = (this -> af_register & 0x00FF) |   (static_cast<uint16_t>(val) << 8);
}
//----------------------//

//HELPER METHODS//
uint8_t cpu::inc_8bit_rgstr(uint8_t rgstr){

    this -> af_register = this -> af_register & 0xFF1F;//unset all flags except carry

    if(rgstr == 0xFF){//inc will cause it to become zero, wrap around
        set_z_flag();//Z: 0x80
    }

    uint8_t lower_4bits = rgstr & 0x0F;

    if(lower_4bits == 0x0F){//if all lower bits are set a half carry will always be made on inc
        set_hc_flag();//HC: 0x20
    }

    return ++rgstr;
}

void cpu::add_16bit_rgstr(uint16_t &target_rgstr, uint16_t &oprnd_rgstr){
    //16bit addition carry depends on 15th bit and half carry depends on 11th bit

    //determine HC status
    uint16_t result16bit_op = (target_rgstr & 0x0FFF) + (oprnd_rgstr & 0x0FFF);

    if((result16bit_op & 0x1000) == 0x1000){//if a carry over into the 12th bit occurs
        set_hc_flag();//set HC;
    }

    //determine C status
    uint32_t target32bit_rep = static_cast<uint32_t>(target_rgstr);
    uint32_t oprnd32bit_rep = static_cast<uint32_t>(oprnd_rgstr);

    uint32_t result32bit_op = target32bit_rep + oprnd32bit_rep;

    if((result32bit_op & 0xFFFF0000) > 0x00000000){//if any chang occurs to upper 8bits of 32bit rep then a carry has been performed
        set_c_flag();//set carry
    }

    target_rgstr += oprnd_rgstr;
}

uint8_t cpu::add_8bit_rgstr(uint8_t target_rgstr, uint8_t oprnd_rgstr){
    unset_all_flags();//unset all flags including negative

    //max possible values
    uint16_t max_8bit_int = static_cast<uint16_t>(255);
    uint8_t max_4bit_int = static_cast<uint8_t>(15);

    //check for half carry
    uint8_t sum_4bit = (target_rgstr & 0x0F) + (oprnd_rgstr & 0x0F);

    if(sum_4bit  > max_4bit_int){//if sum is greater than 15 a carry occurred
        set_hc_flag();//set hc
    }

    //check for carry
    uint16_t sum_16bit = static_cast<uint16_t>(target_rgstr + oprnd_rgstr);

    if(sum_16bit > max_8bit_int){//if sum is greater than 255 a carry occurred
        set_c_flag();//set c
    }

    uint8_t sum_8bit_rep = static_cast<uint8_t>(sum_16bit);//convert to 8 bit rep

    if(sum_8bit_rep == 0x00){//if 8bit rep is empty
        set_z_flag();//set zero
    }

    return sum_8bit_rep;
}

uint8_t cpu::sub_8bit_rgstr(uint8_t target_rgstr, uint8_t oprnd_rgstr){
    //unset all and set n flag
    unset_all_flags();
    set_n_flag();

    //HC check
    // extract bit 3 from each
    uint8_t target_lower4 = target_rgstr & 0x0F;
    uint8_t oprnd_lower4 = oprnd_rgstr & 0x0F;

    uint8_t val = target_rgstr - oprnd_rgstr;

    //check hc
    if(target_lower4 < oprnd_lower4){
        set_hc_flag();//set HC
    }

    if(target_rgstr < oprnd_rgstr){
         set_c_flag();//set C
    }

    //check zero
    if(val == 0x00){
        set_z_flag();//set Z;
    }

    return val;
}

uint8_t cpu::sbc_8bit_rgstr(uint8_t target_rgstr, uint8_t oprnd_rgstr){
    uint8_t carry_flag = static_cast<uint8_t>(this -> af_register & 0x0010);//extract carry flag
    // this -> af_register = (this -> af_register & 0xFF0F) | 0x0040;;
    //unset all and set n flag
    unset_all_flags();
    set_n_flag();

    uint8_t temp_oprnd_rgstr = oprnd_rgstr;


    if(carry_flag == 0x10){//if carry flag set
        temp_oprnd_rgstr += 0x01;
    }

    uint8_t target_lower4 = target_rgstr & 0x0F;
    uint8_t oprnd_lower4 = temp_oprnd_rgstr & 0x0F;

    //check hc
    if(target_lower4 < oprnd_lower4){
        set_hc_flag();//set HC
    }

    if(target_rgstr < temp_oprnd_rgstr){
         set_c_flag();//set C
    }


    uint8_t val = target_rgstr - temp_oprnd_rgstr;

    //check zero
    if(val == 0x00){
        set_z_flag();//set Z;
    }

    return val;
}

uint8_t cpu::and_8bit_rgstr(uint8_t target_rgstr, uint8_t oprnd_rgstr){
    // this -> af_register = (this -> af_register & 0xFF0F) | 0x0020;//unset all and set HC
    unset_all_flags();
    set_hc_flag();

    uint8_t val = target_rgstr & oprnd_rgstr;

    if(val == 0x00){
        set_z_flag();//set Z
    }

    return val;
}


uint8_t cpu::xor_8bit_rgstr(uint8_t target, uint8_t oprnd_rgstr){
    unset_all_flags();

    uint8_t val = target ^ oprnd_rgstr;

    if(val == 0x00){
        set_z_flag();//set Z
    }
    return val;
}

uint8_t cpu::or_8bit_rgstr(uint8_t target, uint8_t oprnd_rgstr){
    unset_all_flags();

    uint8_t val = target | oprnd_rgstr;

    if(val == 0x00){
        set_z_flag();
    }

    return val;
}


uint8_t cpu::adc_8bit_rgstr(uint8_t target_rgstr, uint8_t oprnd_rgstr){
    uint8_t carry_flag = static_cast<uint8_t>(this -> af_register & 0x0010);//get carry

    if(carry_flag == 0x10){//if carry flag convert to 1
        carry_flag = 0x01;
    }else{
        carry_flag = 0x00;
    }

    this -> af_register = this -> af_register & 0xFF0F;//unset all flags

    //max possible values
    uint16_t max_8bit_int = static_cast<uint16_t>(255);
    uint8_t max_4bit_int = static_cast<uint8_t>(15);

    //check for half carry
    uint8_t sum_4bit = (target_rgstr & 0x0F) + (oprnd_rgstr & 0x0F) + carry_flag;

    if(sum_4bit  > max_4bit_int){//if sum is greater than 15 a carry occurred
        set_hc_flag();//set hc
    }

    //check for carry
    uint16_t sum_16bit = static_cast<uint16_t>(target_rgstr + oprnd_rgstr + carry_flag);

    if(sum_16bit > max_8bit_int){//if sum is greater than 255 a carry occurred
        set_c_flag();//set c
    }

    uint8_t sum_8bit_rep = static_cast<uint8_t>(sum_16bit);//convert to 8 bit rep

    if(sum_8bit_rep == 0x00){//if 8bit rep is empty
        set_z_flag();//set zero
    }

    return sum_8bit_rep;
}


uint8_t cpu::dec_8bit_rgstr(uint8_t rgstr){
    this -> af_register = (this -> af_register & 0xFF1F);//unset all flags except carry

    set_n_flag();// set nflag 0x40;

    if(rgstr == 0x01){//if decremented will bear zero
        set_z_flag();//set zero flag 0x80
    }

    uint8_t lower_4bits = rgstr & 0x0F;

    if(lower_4bits == 0x00){
        set_hc_flag();//set HC;
    }

    return --rgstr;
}

void cpu::cp_8bit_rgstr(uint8_t target_rgstr, uint8_t oprnd_rgstr){
    //unset all and set n flag
    unset_all_flags();
    set_n_flag();

    //HC check
    // extract bit 3 from each
    uint8_t target_lower4 = target_rgstr & 0x0F;
    uint8_t oprnd_lower4 = oprnd_rgstr & 0x0F;

    uint8_t val = target_rgstr - oprnd_rgstr;

    //check hc
    if(target_lower4 < oprnd_lower4){
        set_hc_flag();//set HC
    }

    if(target_rgstr < oprnd_rgstr){
         set_c_flag();//set C
    }

    //check zero
    if(val == 0x00){
        set_z_flag();//set Z;
    }
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
                        uint16_t inc_rgstr = (static_cast<uint16_t>(inc_8bit_rgstr(get_b_reg())) << 8);

                        this -> bc_register = (this -> bc_register & 0x00FF) | inc_rgstr;
                        return 4;
                    }

                    case 0x10:{//INC D
                        uint16_t inc_rgstr = (static_cast<uint16_t>(inc_8bit_rgstr(get_d_reg())) << 8);

                        this -> de_register = (this -> de_register & 0x00FF) | inc_rgstr;
                        return 4;
                    }

                    case 0x20:{//INC H
                        uint16_t inc_rgstr = (static_cast<uint16_t>(inc_8bit_rgstr(get_h_reg())) << 8);

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
                        uint16_t dec_rgstr = (static_cast<uint16_t>(dec_8bit_rgstr(get_b_reg())) << 8);

                        this -> bc_register = (this -> bc_register & 0x00FF) | dec_rgstr;
                        return 4;
                    }

                    case 0x10:{//DEC D
                        uint16_t dec_rgstr = (static_cast<uint16_t>(dec_8bit_rgstr(get_b_reg())) << 8);

                        this -> de_register = (this -> de_register & 0x00FF) | dec_rgstr;
                        return 4;
                    }

                    case 0x20:{//DEC H
                        uint16_t dec_rgstr = (static_cast<uint16_t>(dec_8bit_rgstr(get_h_reg())) << 8);

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
                        uint16_t inc_rgstr = static_cast<uint16_t>(inc_8bit_rgstr(get_c_reg()));

                        this -> bc_register = (this -> bc_register & 0xFF00) | inc_rgstr;
                        return 4;
                    }

                    case 0x10:{//INC E
                        uint16_t inc_rgstr = static_cast<uint16_t>(inc_8bit_rgstr(get_e_reg()));

                        this -> de_register = (this -> de_register & 0xFF00) | inc_rgstr;
                        return 4;
                    }

                    case 0x20:{//INC L
                        uint16_t inc_rgstr = static_cast<uint16_t>(inc_8bit_rgstr(get_l_reg()));

                        this -> hl_register = (this -> hl_register & 0xFF00) | inc_rgstr;
                        return 4;
                    }

                    case 0x03:{//INC A
                        uint16_t inc_rgstr = static_cast<uint16_t>(inc_8bit_rgstr(get_a_reg()));

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
                        uint16_t dec_rgstr = static_cast<uint16_t>(dec_8bit_rgstr(get_c_reg()));

                        this -> bc_register = (this -> bc_register & 0xFF00) | dec_rgstr;
                        return 4;
                    }

                    case 0x10:{//DEC E
                        uint16_t dec_rgstr = static_cast<uint16_t>(dec_8bit_rgstr(get_e_reg()));

                        this -> de_register = (this -> de_register & 0xFF00) | dec_rgstr;
                        return 4;
                    }

                    case 0x20:{//DEC L
                        uint8_t l_rgstr = static_cast<uint8_t>(this -> hl_register & 0x00FF);
                        uint16_t dec_rgstr = static_cast<uint16_t>(dec_8bit_rgstr(get_l_reg()));

                        this -> hl_register = (this -> hl_register & 0xFF00) | dec_rgstr;
                        return 4;
                    }

                    case 0x03:{//DEC A
                        uint16_t dec_rgstr = static_cast<uint16_t>(dec_8bit_rgstr(get_a_reg()));

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
                add_16bit_rgstr(this -> hl_register, this -> bc_register);
                return 8;
            }

            case 0x10:{
                add_16bit_rgstr(this -> hl_register, this -> de_register);
                return 8;
            }

            case 0x20:{
                add_16bit_rgstr(this -> hl_register, this -> hl_register);
                return 8;
            }

            case 0x30:{
                add_16bit_rgstr(this -> hl_register, this -> stack_pointer);
                return 8;
            }
            break;
        }

        //RRCA
        case 0x0F:{//rotates a reg by one bit

            //unset zero negative hc and c
            this -> af_register = this -> af_register & 0xFF0F;

            uint8_t a_reg = get_a_reg();

            uint8_t bit_0 = a_reg & 0x01;//get zeroth bit
            a_reg = a_reg >> 1; //shift to the right once

            if(bit_0 == 0x01){//if a one was carried out
                a_reg = a_reg | 0x80;//set bit 7;
                set_c_flag(); //set carry
            }

            //set a_reg back into af_register
            this -> af_register = (this -> af_register & 0x00FF) | (static_cast<uint16_t>(a_reg) << 8); //set new get_a_reg()

            return 4;
        }

        //RRA - 0x1F
        case 0x1F:{//rotates a reg by one bit and uses carry flag as 9th bit
            //unset zero negative hc keep carry
            this -> af_register = this -> af_register & 0xFF1F;

            uint8_t a_reg = get_a_reg();

            uint8_t bit_0 = a_reg & 0x01;//get zeroth bit
            a_reg = a_reg >> 1; //shift to the right once

            if((this -> af_register & 0x0010) == 0x0010){// if carry flag is set
                a_reg = a_reg | 0x80; //carry flag value is wrapped into bit 7;
            }//else bit 7 is left as zero


            if(bit_0 == 0x01){//if a one was carried out of bit zero its moved into the carry flag
                set_c_flag(); //set carry
            }else{
                this -> af_register = this -> af_register & 0xFFEF;
            }

            //set a_reg back into af_register
            this -> af_register = (this -> af_register & 0x00FF) | (static_cast<uint16_t>(a_reg) << 8); //set new get_a_reg()

            return 4;
        }

        case 0x2F:{//CPL flip all the bits of register A
            this -> af_register =  (this -> af_register ^ 0xFF00) | 0x0060;//flip then set n and hc

            return 4;
        }

        case 0x3F:{// filp carry flag
            this -> af_register = (this -> af_register & 0xFF9F); // unset n and hc

            uint16_t mask = 0x0010;

            uint16_t c_flag = af_register & mask;

            if(c_flag == 0x0000){//if carry flag not set
                set_c_flag();//set carry
            }else{
                this -> af_register = this -> af_register & 0xFFEF;//unset carry
            }

            return 4;
        }


        case 0x27:{//DAA
            //todo
        }

        case 0x37:{//SCF set carry flag
            set_c_flag();
            return 4;
        }


        // all JR instructions
        case 0x18: case 0x20: case 0x28: case 0x30:
        case 0x38:

        switch(lower_instr_byte){
            case 0x00:{
                switch (upper_instr_byte) {
                    case 0x20:{//0x20 JR NZ
                        bool is_zero_set = (this -> af_register & 0x0080) == 0x0080;

                        if(!is_zero_set){
                            int8_t data = static_cast<int8_t>(memory.read(++program_counter));
                            this -> program_counter += data;

                            return 12;
                        }

                        return 8;
                    }

                    case 0x30:{ //JR NC,i8 - 0x30
                        bool is_carry_set = (this -> af_register & 0x0010) == 0x0010;

                        if(!is_carry_set){
                            int8_t data = static_cast<int8_t>(memory.read(++program_counter));
                            this -> program_counter += data;

                            return 12;
                        }

                        return 8;
                    }
                }
            }

            case 0x08:{
                switch (upper_instr_byte) {//
                    case 0x10:{//0x18 JR i8
                        int8_t data = static_cast<int8_t>(memory.read(++program_counter));
                        this -> program_counter += data;

                        return 12;
                    }

                    case 0x20:{//0x28 JR Z

                        bool is_zero_set = (this -> af_register & 0x0080) == 0x0080;

                        if(is_zero_set){
                            int8_t data = static_cast<int8_t>(memory.read(++program_counter));
                            this -> program_counter += data;

                            return 12;
                        }

                        return 8;
                    }

                    case 0x30:{//0x38 JR C
                        bool is_carry_set = (this -> af_register & 0x0010) == 0x0010;

                        if(is_carry_set){
                            int8_t data = static_cast<int8_t>(memory.read(++program_counter));
                            this -> program_counter += data;

                            return 12;
                        }

                        return 8;
                    }
                }
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
                return 12;
            }

            case 0x02:{

                switch (upper_instr_byte) {//write upper get_a_reg() value into memory addr
                    case 0x00:{
                        memory.write(this -> bc_register, get_a_reg());
                        return 8;
                    }

                    case 0x10:{
                        memory.write(this -> de_register, get_a_reg());
                        return 8;
                    }

                    case 0x20:{
                        memory.write(++this -> hl_register, get_a_reg());
                        return 8;
                    }

                    case 0x30:{
                        memory.write(--this -> hl_register, get_a_reg());
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
                uint8_t data;

                switch(upper_instr_byte){
                    case 0x00:{
                        data = memory.read(this -> bc_register);
                        break;
                    }

                    case 0x10:{
                        data = memory.read(this -> de_register);
                        break;
                    }

                    case 0x20:{
                        data = memory.read(this -> hl_register + 1);
                        break;
                    }

                    case 0x30:{
                        data = memory.read(this -> hl_register - 1);
                        break;
                    }

                    // default://return err
                }
                set_a_reg(data);
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
                        memory.write(this -> hl_register, get_b_reg());
                        return 8;

                    }case 0x01:{//load c into hl_addr
                        memory.write(this -> hl_register, get_c_reg());
                        return 8;
                    }

                    case 0x02:{//load d into hl_addr
                        memory.write(this -> hl_register, get_d_reg());
                        return 8;
                    }

                    case 0x03:{//load e into hl_addr
                        memory.write(this -> hl_register, get_e_reg());
                        return 8;
                    }

                    case 0x04:{//load h into hl_addr
                        memory.write(this -> hl_register, get_h_reg());
                        return 8;
                    }

                    case 0x05:{//load l into hl_addr
                        memory.write(this -> hl_register, get_l_reg());
                        return 8;
                    }

                    //6 is halt under halt category: TODO

                    case 0x07:{
                        memory.write(this->hl_register, get_a_reg());
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

        //ALL LOWER ADD
        case 0x80: case 0x81: case 0x82: case 0x83: case 0x84:
        case 0x85: case 0x86: case 0x87:
        switch (lower_instr_byte) {

            case 0x00:{// ADD AB
                uint8_t sum = add_8bit_rgstr(get_a_reg(), get_b_reg());

                set_a_reg(sum);

                return 4;
            }

            case 0x01:{//ADD AC
                uint8_t sum = add_8bit_rgstr(get_a_reg(), get_c_reg());

                set_a_reg(sum);

                return 4;
            }

            case 0x02:{//ADD AD
                uint8_t sum = add_8bit_rgstr(get_a_reg(), get_d_reg());

                set_a_reg(sum);

                return 4;
            }

            case 0x03:{//ADD AE
                uint8_t sum = add_8bit_rgstr(get_a_reg(), get_e_reg());

                set_a_reg(sum);

                return 4;
            }

            case 0x04:{//ADD AH
                uint8_t sum = add_8bit_rgstr(get_a_reg(), get_h_reg());

                set_a_reg(sum);

                return 4;
            }

            case 0x05:{//ADD AL
                uint8_t sum = add_8bit_rgstr(get_a_reg(), get_l_reg());

                set_a_reg(sum);

                return 4;
            }

            case 0x06:{//ADD A HL
                uint8_t data = memory.read(this -> hl_register);

                uint8_t sum = add_8bit_rgstr(get_a_reg(), data);

                set_a_reg(sum);

                return 8;
            }

            case 0x07:{//ADD AA
                uint8_t sum = add_8bit_rgstr(get_a_reg(), get_a_reg());

                set_a_reg(sum);

                return 4;
            }
        }

        //ALL ADC
        case 0x88: case 0x89: case 0x8A: case 0x8B: case 0x8C: case 0x8D: case 0x8E:
        case 0x8F:
        switch (lower_instr_byte) {
            case 0x08:{//ADC AB
                uint8_t sum = adc_8bit_rgstr(get_a_reg(), get_b_reg());

                set_a_reg(sum);

                return 4;
            }

            case 0x09:{//ADC AC
                uint8_t sum = adc_8bit_rgstr(get_a_reg(), get_c_reg());

                set_a_reg(sum);

                return 4;
            }

            case 0x0A:{//ADC AD
                uint8_t sum = adc_8bit_rgstr(get_a_reg(), get_d_reg());

                set_a_reg(sum);

                return 4;
            }

            case 0x0B:{//ADC AE
                uint8_t sum = adc_8bit_rgstr(get_a_reg(), get_e_reg());

                set_a_reg(sum);

                return 4;
            }

            case 0x0C:{//ADC AH
                uint8_t sum = adc_8bit_rgstr(get_a_reg(), get_h_reg());

                set_a_reg(sum);

                return 4;
            }

            case 0x0D:{//ADC AL
                uint8_t sum = adc_8bit_rgstr(get_a_reg(), get_l_reg());

                set_a_reg(sum);

                return 4;
            }

            case 0x0E:{//ADC A HL
                uint8_t data = memory.read(this -> hl_register);

                uint8_t sum = adc_8bit_rgstr(get_a_reg(), data);

                set_a_reg(sum);

                return 8;
            }

            case 0x0F:{//ADC AA
                uint8_t sum = adc_8bit_rgstr(get_a_reg(), get_a_reg());

                set_a_reg(sum);

                return 4;
            }
        }

        //all SUB
        case 0x90: case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x96:
        case 0x97:
        switch (lower_instr_byte) {

            case 0x00:{//SUB AB
                uint8_t sub = sub_8bit_rgstr(get_a_reg(), get_b_reg());

                set_a_reg(sub);

                return 4;
            }

            case 0x01:{//SUB AC
                uint8_t sub = sub_8bit_rgstr(get_a_reg(), get_c_reg());

                set_a_reg(sub);

                return 4;
            }

            case 0x02:{//SUB AD
                uint8_t sub = sub_8bit_rgstr(get_a_reg(), get_d_reg());

                set_a_reg(sub);

                return 4;
            }

            case 0x03:{//SUB AE
                uint8_t sub = sub_8bit_rgstr(get_a_reg(), get_e_reg());

                set_a_reg(sub);

                return 4;
            }

            case 0x04:{//SUB AH
                uint8_t sub = sub_8bit_rgstr(get_a_reg(), get_h_reg());

                set_a_reg(sub);

                return 4;
            }

            case 0x05:{//SUB AL
                uint8_t sub = sub_8bit_rgstr(get_a_reg(), get_l_reg());

                set_a_reg(sub);

                return 4;
            }

            case 0x06:{//SUB A (HL)
                uint8_t data = memory.read(this -> hl_register);

                uint8_t sub = sub_8bit_rgstr(get_a_reg(), data);

                set_a_reg(sub);

                return 8;
            }

            case 0x07:{//SUB AA
                uint8_t sub = sub_8bit_rgstr(get_a_reg(), get_a_reg());

                set_a_reg(sub);

                return 4;
            }

        }

        //all SBC
        case 0x98: case 0x99: case 0x9A: case 0x9B: case 0x9C:
        case 0x9D: case 0x9E: case 0x9F:

            switch (lower_instr_byte) {
                case 0x08:{//SBC AB
                    uint8_t sbc = sbc_8bit_rgstr(get_a_reg(), get_b_reg());

                    set_a_reg(sbc);

                    return 4;
                }

                case 0x09:{//SBC AC
                    uint8_t sbc = sbc_8bit_rgstr(get_a_reg(), get_c_reg());

                    set_a_reg(sbc);

                    return 4;
                }

                case 0x0A:{//SBC AD
                    uint8_t sbc = sbc_8bit_rgstr(get_a_reg(), get_d_reg());

                    set_a_reg(sbc);

                    return 4;
                }

                case 0x0B:{//SBC AE
                    uint8_t sbc = sbc_8bit_rgstr(get_a_reg(), get_e_reg());

                    set_a_reg(sbc);

                    return 4;
                }

                case 0x0C:{//SBC AH
                    uint8_t sbc = sbc_8bit_rgstr(get_a_reg(), get_h_reg());

                    set_a_reg(sbc);

                    return 4;
                }

                case 0x0D:{//SBC AL
                    uint8_t sbc = sbc_8bit_rgstr(get_a_reg(), get_l_reg());

                    set_a_reg(sbc);

                    return 4;
                }

                case 0x0E:{//SBC A HL
                    uint8_t data = memory.read(this -> hl_register);

                    uint8_t sbc = sbc_8bit_rgstr(get_a_reg(), data);

                    set_a_reg(sbc);

                    return 8;
                }

                case 0x0F:{//SBC AA
                    uint8_t sbc = sbc_8bit_rgstr(get_a_reg(), get_a_reg());

                    set_a_reg(sbc);

                    return 4;
                }

            }

        //all AND
        case 0xA0: case 0xA1: case 0xA2: case 0xA3:
        case 0xA4: case 0xA5: case 0xA6: case 0xA7:
        switch (lower_instr_byte) {
            case 0x00:{//AND B
                uint8_t and_res = and_8bit_rgstr(get_a_reg(), get_b_reg());
                set_a_reg(and_res);

                return 4;
            }

            case 0x01:{//AND C
                uint8_t and_res = and_8bit_rgstr(get_a_reg(), get_c_reg());
                set_a_reg(and_res);

                return 4;
            }

            case 0x02:{//AND D
                uint8_t and_res = and_8bit_rgstr(get_a_reg(), get_d_reg());
                set_a_reg(and_res);

                return 4;
            }

            case 0x03:{//AND E
                uint8_t and_res = and_8bit_rgstr(get_a_reg(), get_e_reg());
                set_a_reg(and_res);

                return 4;
            }

            case 0x04:{//AND H
                uint8_t and_res = and_8bit_rgstr(get_a_reg(), get_h_reg());
                set_a_reg(and_res);

                return 4;
            }

            case 0x05:{//AND L
                uint8_t and_res = and_8bit_rgstr(get_a_reg(), get_l_reg());
                set_a_reg(and_res);

                return 4;
            }

            case 0x06:{//AND HL
                uint8_t data = memory.read(this -> hl_register);

                uint8_t and_res = and_8bit_rgstr(get_a_reg(), data);
                set_a_reg(and_res);

                return 8;
            }

            case 0x07:{//AND A
                uint8_t and_res = and_8bit_rgstr(get_a_reg(), get_a_reg());
                set_a_reg(and_res);

                return 4;
            }
        }

        //XOR registers
        case 0xA8: case 0xA9: case 0xAA: case 0xAB:
        case 0xAC: case 0xAD: case 0xAE: case 0xAF:
        switch (lower_instr_byte) {
            case 0x08:{//XOR A,B
                uint8_t val = xor_8bit_rgstr(get_a_reg(), get_b_reg());
                set_a_reg(val);

                return 4;
            }
            case 0x09:{//XOR A,C
                uint8_t val = xor_8bit_rgstr(get_a_reg(), get_c_reg());
                set_a_reg(val);

                return 4;
            }
            case 0x0A:{//XOR A,D
                uint8_t val = xor_8bit_rgstr(get_a_reg(), get_d_reg());
                set_a_reg(val);

                return 4;
            }
            case 0x0B:{//XOR A,E
                uint8_t val = xor_8bit_rgstr(get_a_reg(), get_e_reg());
                set_a_reg(val);

                return 4;
            }
            case 0x0C:{//XOR A,H
                uint8_t val = xor_8bit_rgstr(get_a_reg(), get_h_reg());
                set_a_reg(val);

                return 4;
            }
            case 0x0D:{//XOR A,L
                uint8_t val = xor_8bit_rgstr(get_a_reg(), get_l_reg());
                set_a_reg(val);

                return 4;
            }
            case 0x0E:{//XOR A,HL
                uint8_t data = memory.read(this -> hl_register);

                uint8_t val = xor_8bit_rgstr(get_a_reg(), data);
                set_a_reg(val);

                return 4;
            }
            case 0x0F:{//XOR A,A
                uint8_t val = xor_8bit_rgstr(get_a_reg(), get_a_reg());
                set_a_reg(val);

                return 4;
            }
        }

        //OR instructions
        case 0xB0: case 0xB1: case 0xB2: case 0xB3:
        case 0xB4: case 0xB5: case 0xB6: case 0xB7:
        switch (lower_instr_byte) {

            case 0x00:{//OR AB
                uint8_t val = or_8bit_rgstr(get_a_reg(), get_b_reg());
                set_a_reg(val);

                return 4;
            }

            case 0x01:{//OR AC
                uint8_t val = or_8bit_rgstr(get_a_reg(), get_c_reg());
                set_a_reg(val);

                return 4;
            }

            case 0x02:{//OR AD
                uint8_t val = or_8bit_rgstr(get_a_reg(), get_d_reg());
                set_a_reg(val);

                return 4;
            }

            case 0x03:{//OR AE
                uint8_t val = or_8bit_rgstr(get_a_reg(), get_e_reg());
                set_a_reg(val);

                return 4;
            }

            case 0x04:{//OR AH
                uint8_t val = or_8bit_rgstr(get_a_reg(), get_h_reg());
                set_a_reg(val);

                return 4;
            }

            case 0x05:{//OR AL
                uint8_t val = or_8bit_rgstr(get_a_reg(), get_l_reg());
                set_a_reg(val);

                return 4;
            }

            case 0x06:{//OR A HL
                uint8_t data = memory.read(this -> hl_register);
                uint8_t val = or_8bit_rgstr(get_a_reg(), data);
                set_a_reg(val);

                return 4;
            }

            case 0x07:{//OR AA
                uint8_t val = or_8bit_rgstr(get_a_reg(), get_a_reg());
                set_a_reg(val);

                return 4;
            }
        }

        //compare instructions
        case 0xB8: case 0xB9: case 0xBA: case 0xBB:
        case 0xBC: case 0xBD: case 0xBE: case 0xBF:
        switch (lower_instr_byte) {
            case 0x08:{//CP AB
                cp_8bit_rgstr(get_a_reg(), get_b_reg());
            }

            case 0x09:{//CP AC
                cp_8bit_rgstr(get_a_reg(), get_c_reg());
            }

            case 0x0A:{//CP AD
                cp_8bit_rgstr(get_a_reg(), get_d_reg());
            }

            case 0x0B:{//CP AE
                cp_8bit_rgstr(get_a_reg(), get_e_reg());
            }

            case 0x0C:{//CP AH
                cp_8bit_rgstr(get_a_reg(), get_h_reg());
            }

            case 0x0D:{//CP AL
                cp_8bit_rgstr(get_a_reg(), get_l_reg());
            }

            case 0x0E:{//CP AHL
                uint8_t data = memory.read(this -> hl_register);
                cp_8bit_rgstr(get_a_reg(), data);
            }

            case 0x0F:{//CP AA
                cp_8bit_rgstr(get_a_reg(), get_a_reg());
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
//------------------//
