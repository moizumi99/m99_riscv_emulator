//
// Created by moiz on 2/1/20.
//

#ifndef ASSEMBLER_TEST_SYSTEM_CALL_EMULATOR_H
#define ASSEMBLER_TEST_SYSTEM_CALL_EMULATOR_H

#include <memory>
#include "memory_wrapper.h"

struct riscv32_newlib_stat {
  uint16_t st_dev;  // 0
  uint16_t st_ino;  // 2
  uint32_t st_mode;  // 4
  uint16_t st_nlink;  // 8
  uint16_t st_uid;  // 10
  uint16_t st_gid;  // 12
  uint16_t st_rdev;  // 14
  int32_t st_size;  // 16
  int32_t st_spare0; // 20
  int64_t st_atim; // 24
  int32_t st_spare1; // 32
  int32_t st_spare1_2; // 36
  int64_t st_mtim; // 40
  int32_t st_spare2; // 48
  int32_t st_spare2_2; // 52
  int64_t st_ctim; // 56
  int32_t st_spare3; // 60
  int32_t st_blksize; // 68
  int32_t st_blocks; // 72
  int32_t st_spare4[2]; // 76
};

void show_guest_stat(const riscv32_newlib_stat &guest_stat);
void show_host_stat(const struct stat &host_stat);
void conv_guest_stat_to_host_stat(const riscv32_newlib_stat &guest_stat, struct stat *host_stat);
void conv_host_stat_to_guest_stat(const struct stat &host_stat, riscv32_newlib_stat *guest_stat);
constexpr size_t kMaxBufferSize = UINT16_MAX;
size_t memory_wrapper_strlen(const memory_wrapper &mem, size_t address, size_t max = kMaxBufferSize);
char *memory_wrapper_copy(const memory_wrapper &mem, size_t address, size_t length, char *dst);
std::pair<bool, bool> system_call_emulation(std::shared_ptr<memory_wrapper> memory, uint32_t reg[], const uint32_t top, uint32_t *break_address);

#endif //ASSEMBLER_TEST_SYSTEM_CALL_EMULATOR_H
