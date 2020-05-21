#!/bin/bash
test_list=(
  "target/share/riscv-tests/isa/rv32um-v-div"
  "target/share/riscv-tests/isa/rv32um-v-divu"
  "target/share/riscv-tests/isa/rv32um-v-mul"
  "target/share/riscv-tests/isa/rv32um-v-mulh"
  "target/share/riscv-tests/isa/rv32um-v-mulhsu"
  "target/share/riscv-tests/isa/rv32um-v-mulhu"
  "target/share/riscv-tests/isa/rv32um-v-rem"
  "target/share/riscv-tests/isa/rv32um-v-remu"
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
