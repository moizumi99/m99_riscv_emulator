#!/bin/bash
test_list=(
  "target/share/riscv-tests/isa/rv32ui-p-add"
  "target/share/riscv-tests/isa/rv32ui-p-or"
  "target/share/riscv-tests/isa/rv32ui-p-addi"
  "target/share/riscv-tests/isa/rv32ui-p-ori"
  "target/share/riscv-tests/isa/rv32ui-p-and"
  "target/share/riscv-tests/isa/rv32ui-p-sb"
  "target/share/riscv-tests/isa/rv32ui-p-andi"
  "target/share/riscv-tests/isa/rv32ui-p-sh"
  "target/share/riscv-tests/isa/rv32ui-p-auipc"
  "target/share/riscv-tests/isa/rv32ui-p-simple"
  "target/share/riscv-tests/isa/rv32ui-p-beq"
  "target/share/riscv-tests/isa/rv32ui-p-sll"
  "target/share/riscv-tests/isa/rv32ui-p-bge"
  "target/share/riscv-tests/isa/rv32ui-p-slli"
  "target/share/riscv-tests/isa/rv32ui-p-bgeu"
  "target/share/riscv-tests/isa/rv32ui-p-slt"
  "target/share/riscv-tests/isa/rv32ui-p-blt"
  "target/share/riscv-tests/isa/rv32ui-p-slti"
  "target/share/riscv-tests/isa/rv32ui-p-bltu"
  "target/share/riscv-tests/isa/rv32ui-p-sltiu"
  "target/share/riscv-tests/isa/rv32ui-p-bne"
  "target/share/riscv-tests/isa/rv32ui-p-sltu"
  "target/share/riscv-tests/isa/rv32ui-p-fence_i"
  "target/share/riscv-tests/isa/rv32ui-p-sra"
  "target/share/riscv-tests/isa/rv32ui-p-jal"
  "target/share/riscv-tests/isa/rv32ui-p-srai"
  "target/share/riscv-tests/isa/rv32ui-p-jalr"
  "target/share/riscv-tests/isa/rv32ui-p-srl"
  "target/share/riscv-tests/isa/rv32ui-p-lb"
  "target/share/riscv-tests/isa/rv32ui-p-srli"
  "target/share/riscv-tests/isa/rv32ui-p-lbu"
  "target/share/riscv-tests/isa/rv32ui-p-sub"
  "target/share/riscv-tests/isa/rv32ui-p-lh"
  "target/share/riscv-tests/isa/rv32ui-p-sw"
  "target/share/riscv-tests/isa/rv32ui-p-lhu"
  "target/share/riscv-tests/isa/rv32ui-p-xor"
  "target/share/riscv-tests/isa/rv32ui-p-lui"
  "target/share/riscv-tests/isa/rv32ui-p-xori"
  "target/share/riscv-tests/isa/rv32ui-p-lw"
)
emulater="./RISCV_Emulator"
for test in "${test_list[@]}"; do
  echo -n "Run ${test} test: "
  $(eval "${emulater} ${test} 2> /dev/null")
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
