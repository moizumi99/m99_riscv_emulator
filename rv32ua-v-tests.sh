#!/bin/bash
test_list=(
  "target/share/riscv-tests/isa/rv32ua-v-amoadd_w"
  "target/share/riscv-tests/isa/rv32ua-v-amoand_w"
  "target/share/riscv-tests/isa/rv32ua-v-amomaxu_w"
  "target/share/riscv-tests/isa/rv32ua-v-amomax_w"
  "target/share/riscv-tests/isa/rv32ua-v-amominu_w"
  "target/share/riscv-tests/isa/rv32ua-v-amomin_w"
  "target/share/riscv-tests/isa/rv32ua-v-amoor_w"
  "target/share/riscv-tests/isa/rv32ua-v-amoswap_w"
  "target/share/riscv-tests/isa/rv32ua-v-amoxor_w"
  # "target/share/riscv-tests/isa/rv32ua-v-lrsc"
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
