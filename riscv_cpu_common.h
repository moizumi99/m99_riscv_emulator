//
// Created by moiz on 5/10/20.
//

#ifndef RISCV_CPU_RISCV_CPU_COMMON_H
#define RISCV_CPU_RISCV_CPU_COMMON_H

enum class PrivilegeMode {
  USER_MODE = 0,
  SUPERVISOR_MODE = 1,
  MACHINE_MODE = 3
};

#endif //RISCV_CPU_RISCV_CPU_COMMON_H
