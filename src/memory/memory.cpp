#include "memory.h"
#include <cstdint>
#include <iostream>



void memory::write(uint16_t mem_address, byte data){

    if(mem_address < 0x0000 || mem_address >= 0xFFFF){
        std::cerr << "memory address: " << "0x" << std::hex << std::uppercase << mem_address <<" is out of bounds\n";
    }else if(
        mem_address >= 0xFF4C && mem_address <= 0xFEFF || //I/O AND UNUSABLE MEMORY
        mem_address >= 0xE000 && mem_address <= 0xFDFF || // UNUSABLE MEMORY
        mem_address == 0xFFFF //INTERRUPT REGISTER
    ){
        std::cerr << "memory address: " << "0x" << std::hex << std::uppercase << mem_address <<" cannot be written to \n";
    }else{
        main_memory[mem_address] = data;
    }
}

byte memory::read(uint16_t mem_address){

    if(mem_address >= 0x0000 && mem_address <=0xFFFF){
        std::cerr << "memory address: " << "0x" << std::hex << std::uppercase << mem_address <<" could not be read from\n";
        return 0xFF; //? unlikely
    }else{
        return main_memory[mem_address];
    }
}

// uint16_t memory::read_16bit_data(uint16_t program_counter){
//     uint8_t lower_byte = read(++this -> program_counter);
//     uint8_t upper_byte = read(++this -> program_counter);

//     uint16_t data = (static_cast<uint16_t>(upper_byte) << 8) | lower_byte;
// }
