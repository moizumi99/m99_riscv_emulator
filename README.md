# What's this?

A simple RISCV32I emulator that can execute an ELF executable file compiled with gnu tool chain with Newlib.

So far, most of user land instructions are supported.

It supports emulation of a few system calls limitation for testing purpose.
- open
- close
- fstat
- write
- read
- lseek
- exit


## Build

1. Run the following command

```
$ make all 
```

## Test

1. Run the following command
```
$ make test
```

2. If you have risc-v test suite (RV32UI) compiled (see the next section), you can run risc-v test suite with the following command

```
$ ./riscv_tests.sh
```

## How to generate executable for the emulator.

1. Install [GNU tool chain](https://github.com/riscv/riscv-gnu-toolchain) for RISCV 32bit with support of ilp.

```
$ ./configure --prefix=/opt/riscv --with-arch=rv32i --with-abi=ilp32
$ make -j
```

2. Compile the target code for rv32i and ilp32 with static option. For example.

```
$ riscv32-unknown-elf-gcc test.c -o test -Wall -march=rv32i -mabi=ilp32 -static
```

3. Run the emulator.

```
$ RISCV_Emulator file_name
```


