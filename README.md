# What's this?

A simple RISCV32I/RISCV64I emulator that can execute an ELF executable file compiled with gnu tool chain and Newlib.

## Supported Instructions

- RV32I (Including compressed instructions)
- RV32M (Including compressed instructions)
- RV64I (Including compressed instructions)
- RV64M (Including compressed instructions)

Following instructions are partially supported
- RV32A (except LR and SC)
- RV64A (except LR and SC)

Support of privilege instruction is partial and not complete.

Some system calls are emulated with **limitation** 

# Usage

## Build

1. Run the following command

```
$ make 
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

## Test

### Compiling the RISCV-Tests

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

The test suite is generated under `$RISCV/target/`

Now you can run test.

## Running Test

1. Run the following command
```
$ make test
```

# Details 

## Runtime options

`-v`: Verbose. Emulator output the current PC, disassembled instruction, register status  
`-e`: System call emulation enabled. See the next section.  
`-64`: Run in 64  bit mode.  
`-m`: Disable machine interrupt delegation to supervisor mode. This is for compatibility with QEMU.  
`-h`: Use "tohost" and "fromhost" functions in riscv-test suite. This is only for riscv-test purpose.  
`-d:` Device emulation. Enable uart output, virtio disk, and interrupt timer. Not complete.  
`-s <filename>`: Load <filename> as disk iamge. You need to enable device emulation with `-d` option.  
`-p`: Paging enabled. This is for testing purpose.  

## System Call emulation

Following system calls are supported with `-e` option with limitation.

- open (1024)
- close (57)
- fstat (80)
- write (64)
- read (63)
- lseek (62)
- brk (214)
- exit (93)

# Running xv6 for riscv

_Support of xv6 is still under development._

This emulator is able to run [xv6 for riscv](https://github.com/mit-pdos/xv6-riscv) to show sh prompt. 
(But nothing further because no key input is supported yet.)  

You first need to compile [xv6 for riscv](https://github.com/mit-pdos/xv6-riscv) with CPUS=1 option. 
Then, you need to copy `kernel/kernel` in the Emulator directory.   
Also, you need `fs.img` (disk image) from xv6 too. `fs.img` is generated when running xv6 with `QEMU` option. 

Run the emulator with `-64 -m -d -s fs.img` option.  

```
$ ./RISCV_Emulator -64 -m -d -s fs.img kernel
```

You will eventually see the prompt (`$`) pops up.  
(Support of key input is planned with no commitment date.)
