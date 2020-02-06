# What's this?

A simple RISCV32I emulator that can execute an ELF executable file compiled with gnu tool chain and Newlib.

So far, most of user land instructions are supported.

Also emulation of a few system calls are supported for testing purpose with **limitation**.
- open
- close
- fstat
- write
- read
- lseek
- brk
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

2. If you have risc-v test suite (RV32UI) compiled and store the tests under target folder (see the next section), you can run risc-v test suite with the following command

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
$ riscv32-unknown-elf-gcc file_name.c -o file_name -Wall -march=rv32i -mabi=ilp32 -static
```

3. Run the emulator.

Move to the emulator folder, then run the following command.

```
$ ./RISCV_Emulator file_name
```

Note: Addresses between 0x40000000 and 0x7FFFFFFF are reserved for stack and heap. So, the binary and data read from ELF file must be located on other addresses.

## Compiling the RISCV-Tests

To run the RISCV tests with this emulator, first set $RISCV to the location of where m99_riscv_emulator is placed.

```
$ RISCV=/folder/where/the/emulator/is/placed
```

Then, follow the instruction on [riscv-tests](https://github.com/riscv/riscv-tests) page as below.

```
$ git clone https://github.com/riscv/riscv-tests
$ cd riscv-tests
$ git submodule update --init --recursive
$ autoconf
$ ./configure --prefix=$RISCV/target
$ make
$ make install
```

Now, go back to to the emulator folder. Run the test script.

```
$ ./riscv_tests.sh
```

