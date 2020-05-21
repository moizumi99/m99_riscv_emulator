//
// Created by moiz on 2/1/20.
//

#include <iostream>
#include <sys/stat.h>
#ifndef WIN32
#include <unistd.h>
#else
#endif
#include <fcntl.h>

#include "system_call_emulator.h"
#include "RISCV_cpu.h"
#ifdef WIN32
#include "io.h"
typedef signed int ssize_t;
#endif

namespace RISCV_EMULATOR {

void ShowGuestStat(const Riscv32NewlibStat &guest_stat) {
  std::cerr << "st_dev: " << guest_stat.st_dev << std::endl;
  std::cerr << "st_ino: " << guest_stat.st_ino << std::endl;
  std::cerr << "st_mode: " << guest_stat.st_mode << std::endl;
  std::cerr << "st_nlink: " << guest_stat.st_nlink << std::endl;
  std::cerr << "st_uid: " << guest_stat.st_uid << std::endl;
  std::cerr << "st_gid: " << guest_stat.st_gid << std::endl;
  std::cerr << "st_rdev: " << guest_stat.st_rdev << std::endl;
  std::cerr << "st_size: " << guest_stat.st_size << std::endl;
  std::cerr << "st_atime: " << guest_stat.st_atim << std::endl;
  std::cerr << "st_mtime: " << guest_stat.st_mtim << std::endl;
  std::cerr << "st_ctime: " << guest_stat.st_ctim << std::endl;
  std::cerr << "st_blksize: " << guest_stat.st_blksize << std::endl;
  std::cerr << "st_blocks: " << guest_stat.st_blocks << std::endl;
}

void ShowHostStat(const struct stat &host_stat) {
  std::cerr << "st_dev: " << host_stat.st_dev << std::endl;
  std::cerr << "st_ino: " << host_stat.st_ino << std::endl;
  std::cerr << "st_mode: " << host_stat.st_mode << std::endl;
  std::cerr << "st_nlink: " << host_stat.st_nlink << std::endl;
  std::cerr << "st_uid: " << host_stat.st_uid << std::endl;
  std::cerr << "st_gid: " << host_stat.st_gid << std::endl;
  std::cerr << "st_rdev: " << host_stat.st_rdev << std::endl;
  std::cerr << "st_size: " << host_stat.st_size << std::endl;
#ifndef WIN32
  std::cerr << "st_blksize: " << host_stat.st_blksize << std::endl;
  std::cerr << "st_blocks: " << host_stat.st_blocks << std::endl;
  std::cerr << "st_atim.tv_sec: " << host_stat.st_atim.tv_sec << std::endl;
  std::cerr << "st_atim.tv_nsec: " << host_stat.st_atim.tv_nsec << std::endl;
  std::cerr << "st_mtim.tv_sec: " << host_stat.st_mtim.tv_sec << std::endl;
  std::cerr << "st_mtim.tv_nsec: " << host_stat.st_mtim.tv_nsec << std::endl;
  std::cerr << "st_ctim.tv_sec: " << host_stat.st_ctim.tv_sec << std::endl;
  std::cerr << "st_ctim.tv_nsec: " << host_stat.st_ctim.tv_nsec << std::endl;
#endif
}

void ConvGuestStatToHostStat(const Riscv32NewlibStat &guest_stat,
                             struct stat *host_stat) {
  host_stat->st_dev = guest_stat.st_dev;
  host_stat->st_ino = guest_stat.st_ino;
  host_stat->st_mode = guest_stat.st_mode;
  host_stat->st_nlink = guest_stat.st_nlink;
  host_stat->st_uid = guest_stat.st_uid;
  host_stat->st_gid = guest_stat.st_gid;
  host_stat->st_rdev = guest_stat.st_rdev;
  host_stat->st_size = guest_stat.st_size;
#ifndef WIN32
  host_stat->st_blksize = guest_stat.st_blksize;
  host_stat->st_blocks = guest_stat.st_blocks;
  host_stat->st_atim.tv_sec = guest_stat.st_atim;
  host_stat->st_atim.tv_nsec = 0;
  host_stat->st_mtim.tv_sec = guest_stat.st_mtim;
  host_stat->st_mtim.tv_nsec = 0;
  host_stat->st_ctim.tv_sec = guest_stat.st_ctim;
  host_stat->st_ctim.tv_nsec = 0;
#endif
}

void ConvHostStatToGuestStat(const struct stat &host_stat,
                             Riscv32NewlibStat *guest_stat) {
  guest_stat->st_dev = host_stat.st_dev;
  guest_stat->st_ino = host_stat.st_ino;
  guest_stat->st_mode = host_stat.st_mode;
  guest_stat->st_nlink = host_stat.st_nlink;
  guest_stat->st_uid = host_stat.st_uid;
  guest_stat->st_gid = host_stat.st_gid;
  guest_stat->st_rdev = host_stat.st_rdev;
  guest_stat->st_size = host_stat.st_size;
#ifndef WIN32
  guest_stat->st_blksize = host_stat.st_blksize;
  guest_stat->st_blocks = host_stat.st_blocks;
  guest_stat->st_atim = (int64_t) host_stat.st_atim.tv_sec;
  guest_stat->st_mtim = (int64_t) host_stat.st_mtim.tv_sec;
  guest_stat->st_ctim = (int64_t) host_stat.st_ctim.tv_sec;
#endif
}

size_t
MemoryWrapperStrlen(const MemoryWrapper &mem, size_t address, size_t max) {
  size_t counter = 0;
  size_t index = address;
  while (mem[index++] && counter < max) {
    ++counter;
  }
  return counter;
}

char *MemoryWrapperCopy(const MemoryWrapper &mem, size_t address, size_t length,
                        char *dst) {
  for (size_t i = 0; i < length; ++i) {
    dst[i] = mem[address + i];
  }
  return dst;
}

std::pair<bool, bool>
SystemCallEmulation(std::shared_ptr<MemoryWrapper> memory, uint64_t *reg,
                    const uint64_t top,
                    uint64_t *break_address, bool debug) {
  auto &brk = *break_address;
  auto &mem = *memory;
  bool end_flag = false;
  bool error_flag = false;
  if (reg[A7] == 93) {
    // Exit system call.
    if (debug)
      std::cerr << "Exit System Call" << std::endl;
    end_flag = true;
  } else if (reg[A7] == 64) {
    // Write.
    if (debug) {
      std::cerr << "Write System Call" << std::endl;
      std::cerr << "FD = " << reg[A0] << ", length = " << reg[A2] << std::endl;
    }
    int length = reg[A2];
    unsigned char *buffer = new unsigned char[length];
    for (int i = 0; i < length; i++) {
      buffer[i] = mem[reg[A1] + i];
    }
    std::cerr << std::endl;
    ssize_t return_value = _write(reg[A0], buffer, length);
    reg[A0] = return_value;
    delete buffer;
  } else if (reg[A7] == 214) {
    // BRK.
    if (debug)
      std::cerr << "BRK System Call" << std::endl;
    if (reg[A0] == 0) {
      reg[A0] = brk;
    } else if (reg[A0] < top) {
      brk = reg[A0];
      // reg[A0] = 0;
    } else {
      reg[A0] = -1;
    }
  } else if (reg[A7] == 63) {
    // READ.
    if (debug)
      std::cerr << "Read System Call" << std::endl;
    int length = reg[A2];
    unsigned char *buffer = new unsigned char[length];
    size_t return_value = _read(reg[A0], buffer, length);
    reg[A0] = (uint32_t) return_value;
    for (int i = 0; i < length; i++) {
      mem[reg[A1] + i] = buffer[i];
    }
    delete buffer;
  } else if (reg[A7] == 80) {
    // FSTAT.
    struct Riscv32NewlibStat guest_stat;
    if (debug) {
      std::cerr << "Fstat System Call" << std::endl;
      std::cerr << "fd: " << reg[A0] << std::endl;
      std::cerr << "riscv32_stat size: " << sizeof(struct Riscv32NewlibStat)
                << std::endl;
    }
    unsigned char *statbuf_p = (unsigned char *) &guest_stat;

    struct stat host_stat;
    int return_value = fstat(reg[A0], &host_stat);

    ConvHostStatToGuestStat(host_stat, &guest_stat);
    if (debug) {
      std::cerr << "ret: " << reg[A0] << std::endl;
      std::cerr << "Guest: struct stat\n";
      ShowGuestStat(guest_stat);
    }
    for (unsigned int i = 0; i < sizeof(Riscv32NewlibStat); i++) {
      mem[reg[A1] + i] = statbuf_p[i];
    }
    reg[A0] = return_value;
  } else if (reg[A7] == 57) {
    // Close.
    if (debug)
      std::cerr << "Close System Call" << std::endl;
    int return_value = _close(reg[A0]);
    reg[A0] = return_value;
  } else if (reg[A7] == 1024) {
    // Open.
    if (debug)
      std::cerr << "Open System Call" << std::endl;
    // These values are found in newlib/libc/include/sys/_default_fcntl.h
    constexpr uint32_t kO_READ = 0x000001;
    constexpr uint32_t kO_WRITE = 0x000002;
    constexpr uint32_t kO_APPEND = 0x000008;
    constexpr uint32_t kO_CLOEXEC = 0x040000;
    constexpr uint32_t kO_CREAT = 0x000200;
    constexpr uint32_t kO_DIRECT = 0x080000;
    constexpr uint32_t kO_DIRECTORY = 0x200000;
    constexpr uint32_t kO_EXCL = 0x000800;
    constexpr uint32_t kO_NOCTTY = 0x008000;
    constexpr uint32_t kO_NONBLOCK = 0x004000;
    constexpr uint32_t kO_SYNC = 0x002000;
    constexpr uint32_t kO_TRUNC = 0x000400;
#ifndef WIN32
    uint32_t flag = (reg[A1] & kO_WRITE) ? O_RDWR : O_RDONLY;
    flag |= (reg[A1] & kO_APPEND) ? O_APPEND : 0;
    flag |= (reg[A1] & kO_CLOEXEC) ? O_CLOEXEC : 0;
    flag |= (reg[A1] & kO_CREAT) ? O_CREAT : 0;
    flag |= (reg[A1] & kO_DIRECT) ? O_DIRECT : 0;
    flag |= (reg[A1] & kO_DIRECTORY) ? O_DIRECTORY : 0;
    flag |= (reg[A1] & kO_EXCL) ? O_EXCL : 0;
    flag |= (reg[A1] & kO_NOCTTY) ? O_NOCTTY : 0;
    flag |= (reg[A1] & kO_NONBLOCK) ? O_NONBLOCK : 0;
    flag |= (reg[A1] & kO_SYNC) ? O_SYNC : 0;
    flag |= (reg[A1] & kO_TRUNC) ? O_TRUNC : 0;
#else
	uint32_t flag = 0;
#endif
    constexpr size_t kMax = 1024;
    size_t length = MemoryWrapperStrlen(mem, reg[A0], kMax);
    int return_value = -1;
    if (length > 0) {
      char *buffer = new char[length + 1];
      MemoryWrapperCopy(mem, reg[A0], length, buffer);
      buffer[length] = 0;
      if (debug) {
        std::cerr << "FIle path: " << buffer << std::endl;
        std::cerr << "File parameter: " << std::hex << reg[A1] << ", "
                  << reg[A2]
                  << std::dec << std::endl;
        std::cerr << "Flag = " << std::hex << flag << std::dec << std::endl;
      }
      return_value = _open(buffer, flag, reg[A2]);
      delete buffer;
    }
    reg[A0] = return_value;
  } else if (reg[A7] == 62) {
    // lseek.
    std::cerr << "Lseek System Call" << std::endl;
    int return_value = _lseek(reg[A0], reg[A1], reg[A2]);
    reg[A0] = return_value;
  } else {
    std::cerr << "Undefined system call (" << reg[A7] << "). Exit.\n";
    end_flag = true;
  }

  return std::make_pair(error_flag, end_flag);
}

} // namespace RISCV_EMULATOR