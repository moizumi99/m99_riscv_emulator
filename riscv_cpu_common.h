//
// Created by moiz on 5/10/20.
//

#ifndef RISCV_CPU_RISCV_CPU_COMMON_H
#define RISCV_CPU_RISCV_CPU_COMMON_H

namespace RISCV_EMULATOR {

enum class PrivilegeMode {
  USER_MODE = 0,
  SUPERVISOR_MODE = 1,
  MACHINE_MODE = 3
};
} // namespace RISCV_EMULATOR

#endif //RISCV_CPU_RISCV_CPU_COMMON_H
