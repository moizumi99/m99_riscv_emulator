#!/bin/bash
test_list=(
  "target/share/riscv-tests/isa/rv32ua-p-amoadd_w"
  "target/share/riscv-tests/isa/rv32ua-p-amoand_w"
  "target/share/riscv-tests/isa/rv32ua-p-amomaxu_w"
  "target/share/riscv-tests/isa/rv32ua-p-amomax_w"
  "target/share/riscv-tests/isa/rv32ua-p-amominu_w"
  "target/share/riscv-tests/isa/rv32ua-p-amomin_w"
  "target/share/riscv-tests/isa/rv32ua-p-amoor_w"
  "target/share/riscv-tests/isa/rv32ua-p-amoswap_w"
  "target/share/riscv-tests/isa/rv32ua-p-amoxor_w"
  # "target/share/riscv-tests/isa/rv32ua-p-lrsc"
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
