#!/bin/bash
test_list=(
  "target/share/riscv-tests/isa/rv64um-p-div"
  "target/share/riscv-tests/isa/rv64um-p-divu"
  "target/share/riscv-tests/isa/rv64um-p-divuw"
  "target/share/riscv-tests/isa/rv64um-p-divw"
  "target/share/riscv-tests/isa/rv64um-p-mul"
  "target/share/riscv-tests/isa/rv64um-p-mulh"
  "target/share/riscv-tests/isa/rv64um-p-mulhsu"
  "target/share/riscv-tests/isa/rv64um-p-mulhu"
  "target/share/riscv-tests/isa/rv64um-p-mulw"
  "target/share/riscv-tests/isa/rv64um-p-rem"
  "target/share/riscv-tests/isa/rv64um-p-remu"
  "target/share/riscv-tests/isa/rv64um-p-remuw"
  "target/share/riscv-tests/isa/rv64um-p-remw"
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
