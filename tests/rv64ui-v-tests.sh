#!/bin/bash
test_list=(
  "target/share/riscv-tests/isa/rv64ui-v-add"
  "target/share/riscv-tests/isa/rv64ui-v-addi"
  "target/share/riscv-tests/isa/rv64ui-v-addiw"
  "target/share/riscv-tests/isa/rv64ui-v-addw"
  "target/share/riscv-tests/isa/rv64ui-v-and"
  "target/share/riscv-tests/isa/rv64ui-v-sll"
  "target/share/riscv-tests/isa/rv64ui-v-andi"
  "target/share/riscv-tests/isa/rv64ui-v-slli"
  "target/share/riscv-tests/isa/rv64ui-v-auipc"
  "target/share/riscv-tests/isa/rv64ui-v-slliw"
  "target/share/riscv-tests/isa/rv64ui-v-sllw"
  "target/share/riscv-tests/isa/rv64ui-v-slt"
  "target/share/riscv-tests/isa/rv64ui-v-slti"
  "target/share/riscv-tests/isa/rv64ui-v-sltiu"
  "target/share/riscv-tests/isa/rv64ui-v-sltu"
  "target/share/riscv-tests/isa/rv64ui-v-sra"
  "target/share/riscv-tests/isa/rv64ui-v-srai"
  "target/share/riscv-tests/isa/rv64ui-v-sraiw"
  "target/share/riscv-tests/isa/rv64ui-v-jalr"
  "target/share/riscv-tests/isa/rv64ui-v-sraw"
  "target/share/riscv-tests/isa/rv64ui-v-srl"
  "target/share/riscv-tests/isa/rv64ui-v-srli"
  "target/share/riscv-tests/isa/rv64ui-v-srliw"
  "target/share/riscv-tests/isa/rv64ui-v-srlw"
  "target/share/riscv-tests/isa/rv64ui-v-sub"
  "target/share/riscv-tests/isa/rv64ui-v-lui"
  "target/share/riscv-tests/isa/rv64ui-v-subw"
 "target/share/riscv-tests/isa/rv64ui-v-xor"
  "target/share/riscv-tests/isa/rv64ui-v-or"
  "target/share/riscv-tests/isa/rv64ui-v-xori"
  "target/share/riscv-tests/isa/rv64ui-v-ori"
  "target/share/riscv-tests/isa/rv64ui-v-beq"
  "target/share/riscv-tests/isa/rv64ui-v-bge"
  "target/share/riscv-tests/isa/rv64ui-v-bgeu"
  "target/share/riscv-tests/isa/rv64ui-v-bltu"
  "target/share/riscv-tests/isa/rv64ui-v-bne"
  "target/share/riscv-tests/isa/rv64ui-v-blt"
  "target/share/riscv-tests/isa/rv64ui-v-jal"
  "target/share/riscv-tests/isa/rv64ui-v-lb"
  "target/share/riscv-tests/isa/rv64ui-v-lbu"
  "target/share/riscv-tests/isa/rv64ui-v-lh"
  "target/share/riscv-tests/isa/rv64ui-v-lhu"
  "target/share/riscv-tests/isa/rv64ui-v-ld"
  "target/share/riscv-tests/isa/rv64ui-v-lw"
  "target/share/riscv-tests/isa/rv64ui-v-lwu"
   "target/share/riscv-tests/isa/rv64ui-v-sb"
  "target/share/riscv-tests/isa/rv64ui-v-sd"
  "target/share/riscv-tests/isa/rv64ui-v-sh"
  "target/share/riscv-tests/isa/rv64ui-v-sw"
  "target/share/riscv-tests/isa/rv64ui-v-fence_i"
  "target/share/riscv-tests/isa/rv64ui-v-simple"
  )
emulater="./RISCV_Emulator"
flag="-h -64"
for test in "${test_list[@]}"; do
  echo -n "Run ${test} test: "
  $(eval "${emulater}  ${flag} ${test} 2> /dev/null")
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