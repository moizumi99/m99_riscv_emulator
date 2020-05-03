#!/bin/bash
test_list=(
  "target/share/riscv-tests/isa/rv32ui-v-lui"
  "target/share/riscv-tests/isa/rv32ui-v-simple"
  "target/share/riscv-tests/isa/rv32ui-v-or"
  "target/share/riscv-tests/isa/rv32ui-v-lbu"
  "target/share/riscv-tests/isa/rv32ui-v-xori"
  "target/share/riscv-tests/isa/rv32ui-v-sll"
  "target/share/riscv-tests/isa/rv32ui-v-sw"
  "target/share/riscv-tests/isa/rv32ui-v-bge"
  "target/share/riscv-tests/isa/rv32ui-v-lb"
  "target/share/riscv-tests/isa/rv32ui-v-slli"
  "target/share/riscv-tests/isa/rv32ui-v-srai"
  "target/share/riscv-tests/isa/rv32ui-v-sub"
  "target/share/riscv-tests/isa/rv32ui-v-lw"
  "target/share/riscv-tests/isa/rv32ui-v-sltiu"
  "target/share/riscv-tests/isa/rv32ui-v-bltu"
  "target/share/riscv-tests/isa/rv32ui-v-slti"
  "target/share/riscv-tests/isa/rv32ui-v-ori"
  "target/share/riscv-tests/isa/rv32ui-v-srli"
  "target/share/riscv-tests/isa/rv32ui-v-xor"
  "target/share/riscv-tests/isa/rv32ui-v-add"
  "target/share/riscv-tests/isa/rv32ui-v-bne"
  "target/share/riscv-tests/isa/rv32ui-v-bgeu"
  "target/share/riscv-tests/isa/rv32ui-v-sra"
  "target/share/riscv-tests/isa/rv32ui-v-slt"
  "target/share/riscv-tests/isa/rv32ui-v-jal"
  "target/share/riscv-tests/isa/rv32ui-v-blt"
  "target/share/riscv-tests/isa/rv32ui-v-srl"
  "target/share/riscv-tests/isa/rv32ui-v-sb"
  "target/share/riscv-tests/isa/rv32ui-v-beq"
  "target/share/riscv-tests/isa/rv32ui-v-sltu"
  "target/share/riscv-tests/isa/rv32ui-v-lh"
  "target/share/riscv-tests/isa/rv32ui-v-lhu"
  "target/share/riscv-tests/isa/rv32ui-v-auipc"
  "target/share/riscv-tests/isa/rv32ui-v-addi"
  "target/share/riscv-tests/isa/rv32ui-v-fence_i"
  "target/share/riscv-tests/isa/rv32ui-v-sh"
  "target/share/riscv-tests/isa/rv32ui-v-andi"
  "target/share/riscv-tests/isa/rv32ui-v-jalr"
  "target/share/riscv-tests/isa/rv32ui-v-and"
)
emulater="./RISCV_Emulator"
flag="-h"
for test in "${test_list[@]}"; do
  echo -n "Run ${test} test: "
  $(eval "${emulater} ${flag} ${test} 2> /dev/null")
  exit_status=$?
  if [[ ${exit_status} -eq 0 ]]; then
    echo "Pass"
  else
    echo "Fail"
    exit ${exit_status}
  fi
done
echo "All tests passed."
exit 0
