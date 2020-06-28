#!/bin/bash
test_list=(
  "target/share/riscv-tests/isa/rv64ua-v-amoadd_d"
  "target/share/riscv-tests/isa/rv64ua-v-amoadd_w"
  "target/share/riscv-tests/isa/rv64ua-v-amoand_d"
  "target/share/riscv-tests/isa/rv64ua-v-amoand_w"
  "target/share/riscv-tests/isa/rv64ua-v-amomax_d"
  "target/share/riscv-tests/isa/rv64ua-v-amomax_w"
  "target/share/riscv-tests/isa/rv64ua-v-amomaxu_d"
  "target/share/riscv-tests/isa/rv64ua-v-amomaxu_w"
  "target/share/riscv-tests/isa/rv64ua-v-amomin_d"
  "target/share/riscv-tests/isa/rv64ua-v-amomin_w"
  "target/share/riscv-tests/isa/rv64ua-v-amominu_d"
  "target/share/riscv-tests/isa/rv64ua-v-amominu_w"
  "target/share/riscv-tests/isa/rv64ua-v-amoor_d"
  "target/share/riscv-tests/isa/rv64ua-v-amoor_w"
  "target/share/riscv-tests/isa/rv64ua-v-amoswap_d"
  "target/share/riscv-tests/isa/rv64ua-v-amoswap_w"
  "target/share/riscv-tests/isa/rv64ua-v-amoxor_d"
  "target/share/riscv-tests/isa/rv64ua-v-amoxor_w"
  # "target/share/riscv-tests/isa/rv64ua-v-lrsc"
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
