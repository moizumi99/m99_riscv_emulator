cmake_minimum_required(VERSION 3.14)
project(assembler_test)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_executable(assembler_test
        assembler.cc
        assembler.h
        bit_tools.cc
        bit_tools.h
        instruction_encdec.cc
        instruction_encdec.h
        load_assembler.cc
        load_assembler.h
        load_assembler_test.cc
        load_assembler_test.h
        RISCV_cpu.cc
        RISCV_cpu.h
        memory_wrapper.h
        memory_wrapper.cpp
        )

add_executable(cpu_test
        assembler.cc
        assembler.h
        bit_tools.cc
        bit_tools.h
        instruction_encdec.cc
        instruction_encdec.h
        load_assembler.cc
        load_assembler.h
        cpu_test.cc
        RISCV_cpu.cc
        RISCV_cpu.h
        memory_wrapper.cpp memory_wrapper.h)

add_executable(RISCV_Emulator
        assembler.cc
        assembler.h
        bit_tools.cc
        bit_tools.h
        instruction_encdec.cc
        instruction_encdec.h
        load_assembler.cc
        load_assembler.h
        RISCV_cpu.cc
        RISCV_cpu.h
        RISCV_Emulator.cc
        RISCV_Emulator.h
        memory_wrapper.cpp memory_wrapper.h)

add_executable(memory_wrapper_test
        memory_wrapper.cpp
        memory_wrapper.h
        memory_wrapper_test.cpp
)