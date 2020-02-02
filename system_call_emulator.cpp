//
// Created by moiz on 2/1/20.
//

#include <iostream>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include "system_call_emulator.h"
#include "RISCV_cpu.h"

void show_guest_stat(const riscv32_newlib_stat &guest_stat) {
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

void show_host_stat(const struct stat &host_stat) {
  std::cerr << "st_dev: " << host_stat.st_dev << std::endl;
  std::cerr << "st_ino: " << host_stat.st_ino << std::endl;
  std::cerr << "st_mode: " << host_stat.st_mode << std::endl;
  std::cerr << "st_nlink: " << host_stat.st_nlink << std::endl;
  std::cerr << "st_uid: " << host_stat.st_uid << std::endl;
  std::cerr << "st_gid: " << host_stat.st_gid << std::endl;
  std::cerr << "st_rdev: " << host_stat.st_rdev << std::endl;
  std::cerr << "st_size: " << host_stat.st_size << std::endl;
  std::cerr << "st_blksize: " << host_stat.st_blksize << std::endl;
  std::cerr << "st_blocks: " << host_stat.st_blocks << std::endl;
  std::cerr << "st_atim.tv_sec: " << host_stat.st_atim.tv_sec << std::endl;
  std::cerr << "st_atim.tv_nsec: " << host_stat.st_atim.tv_nsec << std::endl;
  std::cerr << "st_mtim.tv_sec: " << host_stat.st_mtim.tv_sec << std::endl;
  std::cerr << "st_mtim.tv_nsec: " << host_stat.st_mtim.tv_nsec << std::endl;
  std::cerr << "st_ctim.tv_sec: " << host_stat.st_ctim.tv_sec << std::endl;
  std::cerr << "st_ctim.tv_nsec: " << host_stat.st_ctim.tv_nsec << std::endl;
}

void conv_guest_stat_to_host_stat(const riscv32_newlib_stat &guest_stat, struct stat *host_stat) {
  host_stat->st_dev = guest_stat.st_dev;
  host_stat->st_ino = guest_stat.st_ino;
  host_stat->st_mode = guest_stat.st_mode;
  host_stat->st_nlink = guest_stat.st_nlink;
  host_stat->st_uid = guest_stat.st_uid;
  host_stat->st_gid = guest_stat.st_gid;
  host_stat->st_rdev = guest_stat.st_rdev;
  host_stat->st_size = guest_stat.st_size;
  host_stat->st_blksize = guest_stat.st_blksize;
  host_stat->st_blocks = guest_stat.st_blocks;
  host_stat->st_atim.tv_sec = guest_stat.st_atim;
  host_stat->st_atim.tv_nsec = 0;
  host_stat->st_mtim.tv_sec = guest_stat.st_mtim ;
  host_stat->st_mtim.tv_nsec = 0;
  host_stat->st_ctim.tv_sec = guest_stat.st_ctim;
  host_stat->st_ctim.tv_nsec = 0;
}

void conv_host_stat_to_guest_stat(const struct stat &host_stat, riscv32_newlib_stat *guest_stat) {
  guest_stat->st_dev = host_stat.st_dev;
  guest_stat->st_ino = host_stat.st_ino;
  guest_stat->st_mode = host_stat.st_mode;
  guest_stat->st_nlink = host_stat.st_nlink;
  guest_stat->st_uid = host_stat.st_uid;
  guest_stat->st_gid = host_stat.st_gid;
  guest_stat->st_rdev = host_stat.st_rdev;
  guest_stat->st_size = host_stat.st_size;
  guest_stat->st_blksize = host_stat.st_blksize;
  guest_stat->st_blocks = host_stat.st_blocks;
  guest_stat->st_atim = (int64_t) host_stat.st_atim.tv_sec;
  guest_stat->st_mtim = (int64_t) host_stat.st_mtim.tv_sec;
  guest_stat->st_ctim = (int64_t) host_stat.st_ctim.tv_sec;
}

size_t memory_wrapper_strlen(const memory_wrapper &mem, size_t address, size_t max) {
  size_t counter = 0;
  size_t index = address;
  while(mem[index++] && counter < max) {
    ++counter;
  }
  return counter;
}

char *memory_wrapper_copy(const memory_wrapper &mem, size_t address, size_t length, char *dst) {
  for (size_t i = 0; i < length; ++i) {
    dst[i] = mem[address + i];
  }
  return dst;
}

std::pair<bool, bool> system_call_emulation(std::shared_ptr<memory_wrapper> memory, uint32_t reg[], const uint32_t top, uint32_t *break_address) {
  auto &brk = *break_address;
  auto &mem = *memory;
  bool end_flag = false;
  bool error_flag = false;
  if (reg[A7] == 93) {
    // Exit system call.
    std::cerr << "Exit System Call" << std::endl;
    end_flag = true;
  } else if (reg[A7] == 64) {
    // Write.
    std::cerr << "Write System Call" << std::endl;
    std::cerr << "FD = " << reg[A0] << std::endl;
    int length = reg[A2];
    unsigned char *buffer = new unsigned char[length];
    for (int i = 0; i < length; i++) {
      buffer[i] = mem[reg[A1] + i];
    }
    size_t return_value = write(reg[A0], buffer, length);
//    // Ignore fd. Always output to stdout.
//    for (int i = 0; i < length; i++) {
//      putchar(mem[reg[A1] + i]);
//    }
    reg[A0] = (uint32_t) return_value;
    delete buffer;
  } else if (reg[A7] == 214) {
    // BRK
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
    // READ
    std::cerr << "Read System Call" << std::endl;
    int length = reg[A2];
    unsigned char *buffer = new unsigned  char[length];
    size_t return_value = read(reg[A0], buffer, length);
    reg[A0] = (uint32_t) return_value;
    for (int i = 0; i < length; i++) {
      mem[reg[A1] + i] = buffer[i];
    }
    delete buffer;
  } else if (reg[A7] == 80) {
    // fstat
    std::cerr << "Fstat System Call" << std::endl;
    std::cerr << "fd: " << reg[A0] << std::endl;
    struct riscv32_newlib_stat guest_stat;
    std::cerr << "riscv32_stat size: " << sizeof(struct riscv32_newlib_stat) << std::endl;
    std::cerr << "&st_atime: " <<
              (int) (((char *) (&guest_stat.st_atim)) - ((char *) (&guest_stat))) << std::endl;
    std::cerr << "&st_mtime: " <<
              (int) (((char *) (&guest_stat.st_mtim)) - ((char *) (&guest_stat))) << std::endl;
    std::cerr << "&st_blksize: " <<
              (int) (((char *) (&guest_stat.st_blksize)) - ((char *) (&guest_stat))) << std::endl;
    std::cerr << "&st_spare4[0]: " <<
              (int) (((char *) (&guest_stat.st_spare4[0])) - ((char *) (&guest_stat))) << std::endl;
    unsigned char *statbuf_p = (unsigned char *) &guest_stat;
    for (int i = 0; i < sizeof(struct riscv32_newlib_stat); i++) {
      statbuf_p[i] = mem[reg[A1] + i];
    }
    std::cerr << "Guest: struct stat (initial)" << std::endl;
    show_guest_stat(guest_stat);

    struct stat host_stat;
    conv_guest_stat_to_host_stat(guest_stat, &host_stat);
    int return_value = fstat(reg[A0], &host_stat);
    // std::cerr << "Host: struct stat\n";
    // show_host_stat(host_stat);

    conv_host_stat_to_guest_stat(host_stat, &guest_stat);
    std::cerr << "ret: " << reg[A0] << std::endl;
    std::cerr << "Guest: struct stat\n";
    show_guest_stat(guest_stat);
    for (int i = 0; i < sizeof(riscv32_newlib_stat); i++) {
      (*memory)[reg[A1] + i] = statbuf_p[i];
    }
    reg[A0] = return_value;
  } else if (reg[A7] == 57) {
    // Close.
    std::cerr << "Close System Call" << std::endl;
    int return_value = close(reg[A0]);
    reg[A0] = return_value;
  } else if (reg[A7] == 1024) {
    // Open
    std::cerr << "Open System Call" << std::endl;
    constexpr size_t kMax = 1024;
    size_t length = memory_wrapper_strlen(*memory, reg[A0], kMax);
    int return_value = -1;
    if (length > 0) {
      char *buffer = new char[length + 1];
      memory_wrapper_copy(*memory, reg[A0], length, buffer);
      buffer[length] = 0;
      std::cerr << "FIle path: " << buffer << std::endl;
      return_value = open(buffer, reg[A1], reg[A2]);
      delete buffer;
    }
    reg[A0] = return_value;
  } else if (reg[A7] == 62){
    // lseek
    std::cerr << "Lseek System Call" << std::endl;
    int return_value = lseek(reg[A0], reg[A1], reg[A2]);
    reg[A0] = return_value;
  } else {
    std::cerr << "Undefined system call (" << reg[A7] << ")\n";
    reg[A0] = 0;
  }

  return std::make_pair(error_flag, end_flag);
}