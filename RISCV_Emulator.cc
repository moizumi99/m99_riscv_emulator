#include "RISCV_Emulator.h"
#include "load_assembler.h"
#include "RISCV_cpu.h"
#include <iostream>
#include <vector>
#include <fstream>
#include <elf.h>

// Code
uint8_t mem[256];

constexpr int kMaxBinarySize = 1024 * 1024 * 1024; // 1 GB

std::vector<uint8_t> readFile(std::string filename)
{
  // open the file:
  std::streampos size;
  std::ifstream file(filename, std::ios::binary);

  // get its size:
  file.seekg(0, std::ios::end);
  size = file.tellg();
  file.seekg(0, std::ios::beg);
  if (size > kMaxBinarySize || size <= 0) {
    std::cout << "File size = " << size << "." << std::endl;
    exit(-1);
  }

  // read the data:
  std::vector<uint8_t> fileData;
  fileData.resize(size);
  file.read(reinterpret_cast<char *>(fileData.data()), size);
  return fileData;
}

bool isRightElf(Elf32_Ehdr *ehdr) {
  if ((ehdr->e_ident[EI_MAG0] != ELFMAG0 || ehdr->e_ident[EI_MAG1] != ELFMAG1 ||
      ehdr->e_ident[EI_MAG2] != ELFMAG2 || ehdr->e_ident[EI_MAG3] != ELFMAG3)) {
    std::cerr << "Not an Elf file." << std::endl;
    return false;
  }
  if (ehdr->e_ident[EI_CLASS] != ELFCLASS32) {
    std::cerr << "Not an 32 bit elf (" << static_cast<int>(ehdr->e_ident[EI_CLASS]) << ")" << std::endl;
    return false;
  }
  if (ehdr->e_ident[EI_DATA] != ELFDATA2LSB) {
    std::cerr << "Not little endian (" << static_cast<int>(ehdr->e_ident[EI_DATA]) << ")" << std::endl;
    return false;
  }
  if (ehdr->e_ident[EI_VERSION] != EV_CURRENT) {
    std::cerr << "Not the current version." << std::endl;
    return false;
  }
  if (ehdr->e_ident[EI_OSABI] != ELFOSABI_SYSV) {
    std::cerr << "Not SYSV ABI (" << static_cast<int>(ehdr->e_ident[EI_OSABI]) << ")" << std::endl;
    return false;
  }
  if (ehdr->e_type != ET_EXEC) {
    std::cerr << "Not an executable file (" << static_cast<int>(ehdr->e_type) << ")" << std::endl;
    return false;
  }
  if (ehdr->e_machine != EM_RISCV) {
    std::cerr << "Not for RISCV (" << static_cast<int>(ehdr->e_machine) << ")" << std::endl;
    return false;
  }
  return true;
}

typedef void (*func_t)();

void loadElfFile(std::vector<uint8_t> program) {
  Elf32_Ehdr *ehdr;
  Elf32_Phdr *phdr;
  Elf32_Shdr *shdr;
  func_t f;

  ehdr = (Elf32_Ehdr *) reinterpret_cast<Elf32_Ehdr *>(program.data());
  if (isRightElf(ehdr)) {
    std::cout << "This is an Elf file" << std::endl;
  }

  for(int i = 0; i < ehdr->e_phnum; i++) {
    std::cout << "Program Header " << i << ":";
    phdr = (Elf32_Phdr *)(program.data() + ehdr->e_phoff + ehdr->e_phentsize * i);
    switch (phdr->p_type) {
      case PT_LOAD:
        std::cout << "Type: LOAD" << std::endl;
        break;
      default:
        std::cout << "Type: OTHER" << std::endl;
        break;
    }
  }

}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cerr << "Uasge: "  << argv[0] << " elf_file" << std::endl;
    return -1;
  }
  std::vector<uint8_t> program = readFile(argv[1]);

  loadElfFile(program);

  // Generate very primitive assembly code
  load_assembler_sum(mem);
  printf("Assembler set.\n");

  // Run CPU emulator
  printf("Execution start\n");

  RiscvCpu cpu;
  int error = cpu.run_cpu(mem, 0, false);
  if (error) {
    printf("CPU execution fail.\n");
  }
  int return_value = cpu.read_register(A0);

  printf("Return value: %d\n", return_value);

  return return_value;
}