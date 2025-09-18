#include "src/cpu/cpu.h"
#include "src/memory/memory.h"
#include <iostream>

int main(){
    memory memory;
    cpu cpu;

    std::cout << cpu.execute(0x24 ,memory);
}
