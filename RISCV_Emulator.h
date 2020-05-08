#ifndef RISCV_EMULATOR_H
#define RISCV_EMULATOR_H

#include <cstdint>

/*
 * Memory map
 * 0x00000000 - 0x00000FFF : Reserved
 * 0x00001000 - 0x0FFFFFFF : Text
 * 0x10000000 - 0x3FFFFFFF : Static Data
 * 0x40000000 - 0x7FFFFFFF : Stack and Heap
 * 0x80000000 - 0xBFFFFFFF : Text (optional)
 * 0xC0000000 - 0xFFFFFFFF : System Emulation (e.g. MMU)
 */

constexpr int kUnitSize = 1024 * 1024; // 1 MB
constexpr int kMaxBinarySize = 1024 * 1024 * 1024; // 1 GB
constexpr uint32_t kTop = 0x80000000;
constexpr uint32_t kBottom = 0x40000000;

/*
 * 32 bit MMU table
 * 0xC0000000 - 0xC0100000 : Level 0
 * 0xC0100000 - 0xC0101000 : Level 1
 */
constexpr uint32_t k32BitMmuLevel1 = 0xC0000000; // Size = 2 ^ 10 x 4B.
constexpr uint32_t k32BitMmuLevel0 = k32BitMmuLevel1 + (1 << 10) * 4;

constexpr uint64_t k64BitMmuLevel2 = 0xC0000000; // Size = 2 ^ 9 x 8B. Only 4 is valid.
constexpr uint64_t k64BitMmuLevel1 = k64BitMmuLevel2 + (1 << 9) * 8; // Size = 4 x 2 ^ 9 x 8B.
constexpr uint64_t k64BitMmuLevel0 = k64BitMmuLevel1 + 4 * (1 << 9) * 8;

#endif // RISCV_EMULATOR_H