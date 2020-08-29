cmake_minimum_required(VERSION 3.14)
project(assembler_test)

set(CMAKE_CXX_STANDARD 14)

########### Needed for profiling.
# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")
###########

include_directories(.)

add_executable(assembler_test
        tests/assembler.cc
        tests/assembler.h
        bit_tools.cc
        bit_tools.h
        instruction_encdec.cc
        instruction_encdec.h
        tests/load_assembler.cc
        tests/load_assembler.h
        tests/load_assembler_test.cc
        tests/load_assembler_test.h
        memory_wrapper.h
        memory_wrapper.cpp
        )

add_executable(cpu_test
        tests/assembler.cc
        tests/assembler.h
        bit_tools.cc
        bit_tools.h
        instruction_encdec.cc
        instruction_encdec.h
        tests/load_assembler.cc
        tests/load_assembler.h
        tests/cpu_test.cc
        RISCV_cpu.cc
        RISCV_cpu.h
        memory_wrapper.cpp memory_wrapper.h
        system_call_emulator.cpp system_call_emulator.h
        pte.cpp pte.h
        Mmu.cpp Mmu.h
        riscv_cpu_common.h
        Disassembler.cpp Disassembler.h
        PeripheralEmulator.cpp PeripheralEmulator.h)

add_executable(RISCV_Emulator
        bit_tools.cc
        bit_tools.h
        instruction_encdec.cc
        instruction_encdec.h
        RISCV_cpu.cc
        RISCV_cpu.h
        RISCV_Emulator.cc
        RISCV_Emulator.h
        memory_wrapper.cpp
        memory_wrapper.h
        system_call_emulator.cpp
        system_call_emulator.h
        pte.cpp pte.h
        Mmu.cpp Mmu.h
        riscv_cpu_common.h
        Disassembler.cpp Disassembler.h
        PeripheralEmulator.cpp PeripheralEmulator.h
        ScreenEmulation.cpp ScreenEmulation.h)

target_link_libraries(RISCV_Emulator ncurses)

add_executable(memory_wrapper_test
        memory_wrapper.cpp
        memory_wrapper.h
        tests/memory_wrapper_test.cpp
        )

add_executable(pte_test
        pte.cpp
        pte.h
        tests/pte_test.cpp
        bit_tools.h
        bit_tools.cc
        )