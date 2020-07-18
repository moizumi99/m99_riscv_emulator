#!/bin/bash
test_list=(
  "target/share/riscv-tests/isa/rv32um-p-div"
  "target/share/riscv-tests/isa/rv32um-p-divu"
  "target/share/riscv-tests/isa/rv32um-p-mul"
  "target/share/riscv-tests/isa/rv32um-p-mulh"
  "target/share/riscv-tests/isa/rv32um-p-mulhsu"
  "target/share/riscv-tests/isa/rv32um-p-mulhu"
  "target/share/riscv-tests/isa/rv32um-p-rem"
  "target/share/riscv-tests/isa/rv32um-p-remu"
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
