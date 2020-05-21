#!/bin/bash
test_list=(
  "target/share/riscv-tests/isa/rv64um-v-div"
  "target/share/riscv-tests/isa/rv64um-v-divu"
  "target/share/riscv-tests/isa/rv64um-v-divuw"
  "target/share/riscv-tests/isa/rv64um-v-divw"
  "target/share/riscv-tests/isa/rv64um-v-mul"
  "target/share/riscv-tests/isa/rv64um-v-mulh"
  "target/share/riscv-tests/isa/rv64um-v-mulhsu"
  "target/share/riscv-tests/isa/rv64um-v-mulhu"
  "target/share/riscv-tests/isa/rv64um-v-mulw"
  "target/share/riscv-tests/isa/rv64um-v-rem"
  "target/share/riscv-tests/isa/rv64um-v-remu"
  "target/share/riscv-tests/isa/rv64um-v-remuw"
  "target/share/riscv-tests/isa/rv64um-v-remw"
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
