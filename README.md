# A simple RISCV32I emulator that can execute an ELF executable file.

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
$ RISCV_Emulator test
```


