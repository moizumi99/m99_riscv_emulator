#!/bin/bash
test_list=(
  "target/share/riscv-tests/isa/rv64ua-p-amoadd_d"
  "target/share/riscv-tests/isa/rv64ua-p-amoadd_w"
  "target/share/riscv-tests/isa/rv64ua-p-amoand_d"
  "target/share/riscv-tests/isa/rv64ua-p-amoand_w"
  "target/share/riscv-tests/isa/rv64ua-p-amomax_d"
  "target/share/riscv-tests/isa/rv64ua-p-amomax_w"
  "target/share/riscv-tests/isa/rv64ua-p-amomaxu_d"
  "target/share/riscv-tests/isa/rv64ua-p-amomaxu_w"
  "target/share/riscv-tests/isa/rv64ua-p-amomin_d"
  "target/share/riscv-tests/isa/rv64ua-p-amomin_w"
  "target/share/riscv-tests/isa/rv64ua-p-amominu_d"
  "target/share/riscv-tests/isa/rv64ua-p-amominu_w"
  "target/share/riscv-tests/isa/rv64ua-p-amoor_d"
  "target/share/riscv-tests/isa/rv64ua-p-amoor_w"
  "target/share/riscv-tests/isa/rv64ua-p-amoswap_d"
  "target/share/riscv-tests/isa/rv64ua-p-amoswap_w"
  "target/share/riscv-tests/isa/rv64ua-p-amoxor_d"
  "target/share/riscv-tests/isa/rv64ua-p-amoxor_w"
  # "target/share/riscv-tests/isa/rv64ua-p-a_lrsc"
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
