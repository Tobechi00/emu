#pragma once
#include <cstdint>
#include <sys/types.h>

typedef uint8_t byte;

class memory{

    private:
    byte main_memory[0x10000]; //64KiB or 65536 bytes

    public:
    void write(uint16_t mem_address, byte data);
    byte read(uint16_t mem_address);
    uint16_t read_16bit_data(uint16_t program_counter);
};
