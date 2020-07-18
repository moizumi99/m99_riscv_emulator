#!/bin/bash
test_list=(
  "target/share/riscv-tests/isa/rv32uc-p-rvc"
  "target/share/riscv-tests/isa/rv32uc-v-rvc"
  "target/share/riscv-tests/isa/rv64uc-p-rvc -64"
  "target/share/riscv-tests/isa/rv64uc-v-rvc -64"
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
