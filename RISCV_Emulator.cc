#include "RISCV_Emulator.h"
#include "load_assembler.h"
#include "memory_wrapper.h"
#include "RISCV_cpu.h"
#include "pte.h"
#include <iostream>
#include <vector>
#include <fstream>

#ifndef WIN32

#include <elf.h>

#else
#include <win32_elf.h>
#endif

#include <string>
#include <cstring>
#include <tuple>
#include <cassert>

namespace {
bool flag_64bit = false;

std::vector<uint8_t> ReadFile(std::string filename) {
  // open the file:
  std::streampos size;
  std::ifstream file(filename, std::ios::binary);

  // get its size:
  file.seekg(0, std::ios::end);
  size = file.tellg();
  file.seekg(0, std::ios::beg);
  if (size > kMaxBinarySize || size <= 0) {
    std::cerr << "File size = " << size << "." << std::endl;
    exit(-1);
  }

  // read the data:
  std::vector<uint8_t> fileData;
  fileData.resize((unsigned int) size);
  file.read(reinterpret_cast<char *>(fileData.data()), size);
  return fileData;
}

bool IsRightElf(Elf32_Ehdr *ehdr) {
  if ((ehdr->e_ident[EI_MAG0] != ELFMAG0 || ehdr->e_ident[EI_MAG1] != ELFMAG1 ||
       ehdr->e_ident[EI_MAG2] != ELFMAG2 ||
       ehdr->e_ident[EI_MAG3] != ELFMAG3)) {
    std::cerr << "Not an Elf file." << std::endl;
    return false;
  }
  if (ehdr->e_ident[EI_CLASS] != ELFCLASS32 &&
      ehdr->e_ident[EI_CLASS] != ELFCLASS64) {
    std::cerr << "Not an 32bit or 64 bit("
              << static_cast<int>(ehdr->e_ident[EI_CLASS]) << ")" << std::endl;
    return false;
  }
  if (ehdr->e_ident[EI_DATA] != ELFDATA2LSB) {
    std::cerr << "Not little endian ("
              << static_cast<int>(ehdr->e_ident[EI_DATA]) << ")" << std::endl;
    return false;
  }
  if (ehdr->e_ident[EI_VERSION] != EV_CURRENT) {
    std::cerr << "Not the current version." << std::endl;
    return false;
  }
  if (ehdr->e_ident[EI_OSABI] != ELFOSABI_SYSV) {
    std::cerr << "Not SYSV ABI (" << static_cast<int>(ehdr->e_ident[EI_OSABI])
              << ")" << std::endl;
    return false;
  }
  if (ehdr->e_type != ET_EXEC) {
    std::cerr << "Not an executable file (" << static_cast<int>(ehdr->e_type)
              << ")" << std::endl;
    return false;
  }
  if (ehdr->e_machine != EM_RISCV) {
    std::cerr << "Not for RISCV (" << static_cast<int>(ehdr->e_machine) << ")"
              << std::endl;
    return false;
  }
  return true;
}

bool Is64BitElf(Elf32_Ehdr *ehdr) {
  if (ehdr->e_ident[EI_CLASS] == ELFCLASS64) {
    return true;
  }
  return false;
}

Elf32_Ehdr *GetElf32Ehdr(std::vector<uint8_t> &program) {
  Elf32_Ehdr *ehdr = (Elf32_Ehdr *) reinterpret_cast<Elf32_Ehdr *>(program.data());
  return ehdr;
}

Elf64_Ehdr *GetElf64Ehdr(std::vector<uint8_t> &program) {
  Elf64_Ehdr *ehdr = (Elf64_Ehdr *) reinterpret_cast<Elf64_Ehdr *>(program.data());
  return ehdr;
}

Elf32_Shdr *GetElf32Shdr(std::vector<uint8_t> &program, int index) {
  Elf32_Ehdr *ehdr = GetElf32Ehdr(program);
  if (index < 0 || index >= ehdr->e_shnum) {
    std::cerr << "Section header " << index << " not found." << std::endl;
    return (NULL);
  }
  Elf32_Shdr *shdr = (Elf32_Shdr *) (program.data() + ehdr->e_shoff +
                                     ehdr->e_shentsize * index);
  return shdr;
}

Elf64_Shdr *GetElf64Shdr(std::vector<uint8_t> &program, int index) {
  Elf64_Ehdr *ehdr = GetElf64Ehdr(program);
  if (index < 0 || index >= ehdr->e_shnum) {
    std::cerr << "Section header " << index << " not found." << std::endl;
    return (NULL);
  }
  Elf64_Shdr *shdr = (Elf64_Shdr *) (program.data() + ehdr->e_shoff +
                                     ehdr->e_shentsize * index);
  return shdr;
}

char *GetElf32SectionName(std::vector<uint8_t> &program, Elf32_Shdr *shdr) {
  Elf32_Ehdr *ehdr = GetElf32Ehdr(program);
  Elf32_Shdr *nhdr = GetElf32Shdr(program, ehdr->e_shstrndx);
  return reinterpret_cast<char *>(program.data()) + nhdr->sh_offset +
         shdr->sh_name;
}

char *GetElf64SectionName(std::vector<uint8_t> &program, Elf64_Shdr *shdr) {
  Elf64_Ehdr *ehdr = GetElf64Ehdr(program);
  Elf64_Shdr *nhdr = GetElf64Shdr(program, ehdr->e_shstrndx);
  return reinterpret_cast<char *>(program.data()) + nhdr->sh_offset +
         shdr->sh_name;
}

Elf32_Shdr *SearchElf32Shdr(std::vector<uint8_t> &program, std::string name) {
  Elf32_Ehdr *ehdr = GetElf32Ehdr(program);

  // Find the last section header that has the name information.
  for (int i = 0; i < ehdr->e_shnum; i++) {
    Elf32_Shdr *shdr = GetElf32Shdr(program, i);
    char *section_name = GetElf32SectionName(program, shdr);
    if (!std::strcmp(section_name, name.c_str())) {
      std::cerr << "Section " << name << " found at 0x0" << std::hex
                << shdr->sh_offset << "." << std::endl;
      return shdr;
    }
  }
  return NULL;
}

Elf64_Shdr *SearchElf64Shdr(std::vector<uint8_t> &program, std::string name) {
  Elf64_Ehdr *ehdr = GetElf64Ehdr(program);

  // Find the last section header that has the name information.
  for (int i = 0; i < ehdr->e_shnum; i++) {
    Elf64_Shdr *shdr = GetElf64Shdr(program, i);
    char *section_name = GetElf64SectionName(program, shdr);
    if (!std::strcmp(section_name, name.c_str())) {
      std::cerr << "Section " << name << " found at 0x0" << std::hex
                << shdr->sh_offset << "." << std::endl;
      return shdr;
    }
  }
  return NULL;
}

Elf32_Shdr *SearchElf32Shdr(std::vector<uint8_t> &program, Elf32_Word type) {
  Elf32_Ehdr *ehdr = GetElf32Ehdr(program);

  // Find the last section header that has the name information.
  for (int i = 0; i < ehdr->e_shnum; i++) {
    Elf32_Shdr *shdr = GetElf32Shdr(program, i);
    if (shdr->sh_type == type) {
      char *section_name = GetElf32SectionName(program, shdr);
      std::cerr << "Section " << section_name << "(" << shdr->sh_type
                << ") found at 0x0" << std::hex
                << shdr->sh_offset << "." << std::endl;
      return shdr;
    }
  }
  return NULL;
}

Elf64_Shdr *SearchElf64Shdr(std::vector<uint8_t> &program, Elf64_Word type) {
  Elf64_Ehdr *ehdr = GetElf64Ehdr(program);

  // Find the last section header that has the name information.
  for (int i = 0; i < ehdr->e_shnum; i++) {
    Elf64_Shdr *shdr = GetElf64Shdr(program, i);
    if (shdr->sh_type == type) {
      char *section_name = GetElf64SectionName(program, shdr);
      std::cerr << "Section " << section_name << "(" << shdr->sh_type
                << ") found at 0x0" << std::hex
                << shdr->sh_offset << "." << std::endl;
      return shdr;
    }
  }
  return NULL;
}

void Load32BitElfFile(std::vector<uint8_t> &program, MemoryWrapper &memory) {
  Elf32_Ehdr *ehdr = GetElf32Ehdr(program);
  for (int i = 0; i < ehdr->e_phnum; i++) {
    std::cerr << "Program Header " << i << ":";
    Elf32_Phdr *phdr = (Elf32_Phdr *) (program.data() + ehdr->e_phoff +
                                       ehdr->e_phentsize * i);
    switch (phdr->p_type) {

      case PT_LOAD:
        std::cerr << "Type: LOAD. ";
        std::cerr << "Copy to 0x" << std::hex << static_cast<int>(phdr->p_vaddr)
                  << " from 0x" << static_cast<int>(phdr->p_offset) << std::dec
                  << ", size " << static_cast<int>(phdr->p_filesz) << ". ";

        for (Elf32_Word i = 0; i < phdr->p_filesz; i++) {
          memory[phdr->p_vaddr + i] = program[phdr->p_offset + i];
        }
        std::cerr << "Loaded" << std::endl;
        break;
      default:
        std::cerr << "Type: OTHER" << std::endl;
        break;
    }
  }

  // Set up BSS (/
  Elf32_Shdr *shdr = SearchElf32Shdr(program, ".bss");
  if (shdr) {
    std::cerr << "Secure BSS." << std::endl;
    std::cerr << "BSS start address: " << std::hex << shdr->sh_addr;
    std::cerr << ", end address: " << shdr->sh_addr + shdr->sh_size
              << std::endl;
    for (uint32_t i = shdr->sh_addr; i < shdr->sh_addr + shdr->sh_size; i++) {
      memory[i] = 0;
    }
  } else {
    std::cerr << "No BSS found." << std::endl;
  }
}

void Load64BitElfFile(std::vector<uint8_t> &program, MemoryWrapper &memory) {
  Elf64_Ehdr *ehdr = GetElf64Ehdr(program);
  for (int i = 0; i < ehdr->e_phnum; i++) {
    std::cerr << "Program Header " << i << ":";
    Elf64_Phdr *phdr = (Elf64_Phdr *) (program.data() + ehdr->e_phoff +
                                       ehdr->e_phentsize * i);
    switch (phdr->p_type) {

      case PT_LOAD:
        std::cerr << "Type: LOAD. ";
        std::cerr << "Copy to 0x" << std::hex << static_cast<int>(phdr->p_vaddr)
                  << " from 0x" << static_cast<int>(phdr->p_offset) << std::dec
                  << ", size " << static_cast<int>(phdr->p_filesz) << ". ";

        for (Elf64_Word i = 0; i < phdr->p_filesz; i++) {
          memory[phdr->p_vaddr + i] = program[phdr->p_offset + i];
        }
        std::cerr << "Loaded" << std::endl;
        break;
      default:
        std::cerr << "Type: OTHER" << std::endl;
        break;
    }
  }

  // Set up BSS (/
  Elf64_Shdr *shdr = SearchElf64Shdr(program, ".bss");
  if (shdr) {
    std::cerr << "Secure BSS." << std::endl;
    std::cerr << "BSS start address: " << std::hex << shdr->sh_addr;
    std::cerr << ", end address: " << shdr->sh_addr + shdr->sh_size
              << std::endl;
    for (uint32_t i = shdr->sh_addr; i < shdr->sh_addr + shdr->sh_size; i++) {
      memory[i] = 0;
    }
  } else {
    std::cerr << "No BSS found." << std::endl;
  }
}

void LoadElfFile(std::vector<uint8_t> &program, MemoryWrapper &memory) {
  Elf32_Ehdr *ehdr = GetElf32Ehdr(program);
  if (IsRightElf(ehdr)) {
    std::cerr << "This is a supported RISC-V 32bit or 64bit Elf file"
              << std::endl;
  }

  flag_64bit = Is64BitElf(ehdr);
  if (flag_64bit) {
    Load64BitElfFile(program, memory);
  } else {
    Load32BitElfFile(program, memory);
  }
}

Elf32_Sym *
FindElf32Symbol(std::vector<uint8_t> &program, std::string target_name) {
  // Find the symbol table.
  Elf32_Shdr *shdr = SearchElf32Shdr(program, SHT_SYMTAB);
  if (!shdr) {
    std::cerr << "Symbol table not found." << std::endl;
    return NULL;
  }

  int number = shdr->sh_size / sizeof(Elf32_Sym);
  std::cerr << "Number of symbols = " << std::dec << number << ", ("
            << shdr->sh_size << " bytes)" << std::endl;

  Elf32_Shdr *strtab_shdr = SearchElf32Shdr(program, ".strtab");
  if (!strtab_shdr) {
    std::cerr << ".strtab not found." << std::endl;
    return NULL;
  }

  for (int i = 0; i < number; i++) {
    Elf32_Sym *symbol = (Elf32_Sym *) (program.data() + shdr->sh_offset +
                                       i * sizeof(Elf32_Sym));
//    std::cerr << "Symbol name offset = " << symbol->st_name << "." << std::endl;
    char *symbol_name =
      (char *) (program.data()) + strtab_shdr->sh_offset + symbol->st_name;
//    std::cerr << "Symbol: " << symbol_name << " found. Size =" << symbol->st_size << std::endl;
    if (!strcmp(symbol_name, target_name.c_str())) {
      std::cerr << "Symbol \"" << target_name << "\" found at index " << i
                << "." << std::endl;
      return symbol;
    }
  }
  return NULL;
}

Elf64_Sym *
FindElf64Symbol(std::vector<uint8_t> &program, std::string target_name) {
  // Find the symbol table.
  Elf64_Shdr *shdr = SearchElf64Shdr(program, SHT_SYMTAB);
  if (!shdr) {
    std::cerr << "Symbol table not found." << std::endl;
    return NULL;
  }

  int number = shdr->sh_size / sizeof(Elf64_Sym);
  std::cerr << "Number of symbols = " << std::dec << number << ", ("
            << shdr->sh_size << " bytes)" << std::endl;

  Elf64_Shdr *strtab_shdr = SearchElf64Shdr(program, ".strtab");
  if (!strtab_shdr) {
    std::cerr << ".strtab not found." << std::endl;
    return NULL;
  }

  for (int i = 0; i < number; i++) {
    Elf64_Sym *symbol = (Elf64_Sym *) (program.data() + shdr->sh_offset +
                                       i * sizeof(Elf64_Sym));
//    std::cerr << "Symbol name offset = " << symbol->st_name << "." << std::endl;
    char *symbol_name =
      (char *) (program.data()) + strtab_shdr->sh_offset + symbol->st_name;
//    std::cerr << "Symbol: " << symbol_name << " found. Size =" << symbol->st_size << std::endl;
    if (!strcmp(symbol_name, target_name.c_str())) {
      std::cerr << "Symbol \"" << target_name << "\" found at index " << i
                << "." << std::endl;
      return symbol;
    }
  }
  return NULL;
}

uint32_t GetElf32GlobalPointer(std::vector<uint8_t> &program) {
  std::string target_name = "__global_pointer$";
  Elf32_Sym *symbol = FindElf32Symbol(program, target_name);
  if (symbol) {
    std::cerr << "Global Pointer Value = 0x" << std::hex
              << symbol->st_value << std::dec << "." << std::endl;
    return symbol->st_value;
  }
  std::cerr << "Global Pointer Value not defined." << std::endl;
  return -1;
}

uint32_t GetElf64GlobalPointer(std::vector<uint8_t> &program) {
  std::string target_name = "__global_pointer$";
  Elf64_Sym *symbol = FindElf64Symbol(program, target_name);
  if (symbol) {
    std::cerr << "Global Pointer Value = 0x" << std::hex
              << symbol->st_value << std::dec << "." << std::endl;
    return symbol->st_value;
  }
  std::cerr << "Global Pointer Value not defined." << std::endl;
  return -1;
}

int64_t GetGlobalPointer(std::vector<uint8_t> &program) {
  if (flag_64bit) {
    return GetElf32GlobalPointer(program);
  } else {
    return GetElf64GlobalPointer(program);
  }
}

uint32_t GetElf32EntryPoint(std::vector<uint8_t> &program) {
  Elf32_Ehdr *ehdr = GetElf32Ehdr(program);
  return ehdr->e_entry;
}

uint32_t GetElf64EntryPoint(std::vector<uint8_t> &program) {
  Elf64_Ehdr *ehdr = GetElf64Ehdr(program);
  return ehdr->e_entry;
}

uint64_t GetEntryPoint(std::vector<uint8_t> &program) {
  if (flag_64bit) {
    return GetElf64EntryPoint(program);
  } else {
    return GetElf32EntryPoint(program);
  }
}

std::tuple<bool, std::string, bool, bool, bool, bool, bool>
ParseCmd(int argc, char (***argv)) {
  bool error = false;
  bool verbose = false;
  bool address64bit = false;
  bool paging = false;
  bool ecall_emulation = false;
  bool host_emulation = false;
  std::string filename = "";
  if (argc < 2) {
    error = true;
  } else {
    for (int i = 1; i < argc; i++) {
      if ((*argv)[i][0] == '-') {
        if ((*argv)[i][1] == 'v') {
          verbose = true;
        } else if ((*argv)[i][1] == '6' && (*argv)[i][2] == '4') {
          address64bit = true;
        } else if ((*argv)[i][1] == 'p') {
          paging = true;
        } else if ((*argv)[i][1] == 'e') {
          ecall_emulation = true;
        } else if ((*argv)[i][1] == 'h') {
          host_emulation = true;
        }
      } else {
        if (filename == "") {
          filename = std::string((*argv)[i]);
        } else {
          error = true;
        }
      }
    }
  }
  return std::make_tuple(error, filename, verbose, address64bit, paging,
                         ecall_emulation, host_emulation);
}

constexpr int k32BitMmuLevelOneSize = 1024; // 1024 x 4 B = 4 KiB.
constexpr int k32BitMmuLevelZeroSize = 1024; // 1024 x 4 B = 4 KiB.
constexpr int k32BitPteSize = 4;

void SetDefaultMmuTable32(std::shared_ptr<MemoryWrapper> memory) {
  uint32_t level1 = k32BitMmuLevel1;
  uint32_t level0 = k32BitMmuLevel0;
  // Sv32. Physical address = virtual address.
  // Level1.
  Pte32 pte(0);
  pte.SetV(1);
  for (int i = 0; i < k32BitMmuLevelOneSize; ++i) {
    uint32_t address = level1 + i * k32BitPteSize;
    uint32_t ppn = (level0 + i * k32BitMmuLevelZeroSize * k32BitPteSize) >> 12;
    pte.SetPpn(ppn);
    uint32_t pte_value = pte.GetValue();
    memory->Write32(address, pte_value);
  }
  // Level0.
  pte.SetX(1);
  pte.SetW(1);
  pte.SetR(1);
  for (int j = 0; j < k32BitMmuLevelOneSize; ++j) {
    for (int i = 0; i < k32BitMmuLevelZeroSize; ++i) {
      uint32_t ppn = j * k32BitMmuLevelZeroSize + i;
      uint32_t address = level0 + ppn * k32BitPteSize;
      assert(address < level1 ||
             level1 + k32BitMmuLevelOneSize * k32BitPteSize <= address);
      // ((ppn << 12) + offset) will be the physical address. So, x4096 is not needed here.
      pte.SetPpn(ppn);
      memory->Write32(address, pte.GetValue());
    }
  }
  return;
}

constexpr uint64_t k64BitMmuLevelTwoSize = 512; // 512 x 8 B = 4 KiB.
constexpr uint64_t k64BitMmuLevelOneSize = 512; // 512 x 8 B = 4 KiB.
constexpr uint64_t k64BitMmuLevelZeroSize = 512; // 512 x 8 B = 4 KiB.
constexpr int k64BitPteSize = 8;

void SetDefaultMmuTable64(std::shared_ptr<MemoryWrapper> memory) {
  uint64_t level2 = k64BitMmuLevel2;
  uint64_t level1 = k64BitMmuLevel1;
  uint64_t level0 = k64BitMmuLevel0;
  // Sv32. Physical address = virtual address.
  Pte32 pte(0);
  // Level2. Map only 32bit range = 4 entry at level 2.
  unsigned kLevel2ValidEntry = 4;
  for (unsigned i = 0; i < k64BitMmuLevelTwoSize; i++) {
    uint32_t address = level2 + i * k64BitPteSize;
    if (i < kLevel2ValidEntry) {
      pte.SetV(1);
      uint64_t ppn = (level1 + i * k64BitMmuLevelOneSize * k64BitPteSize) >> 12;
      pte.SetPpn(ppn);
    } else {
      pte.SetPpn(0);
    }
    uint64_t pte_value = pte.GetValue();
    memory->Write64(address, pte_value);
  }
  // Level1.
  pte.SetV(1);
  for (unsigned j = 0; j < kLevel2ValidEntry; ++j) {
    for (unsigned i = 0; i < k64BitMmuLevelOneSize; ++i) {
      uint32_t address =
        level1 + j * k64BitMmuLevelOneSize * k64BitPteSize + i * k64BitPteSize;
      uint64_t ppn = (level0 +
                      j * k64BitMmuLevelOneSize * k64BitMmuLevelZeroSize *
                      k64BitPteSize +
                      i * k64BitMmuLevelZeroSize * k64BitPteSize) >> 12;
      pte.SetPpn(ppn);
      uint64_t pte_value = pte.GetValue();
      memory->Write64(address, pte_value);
    }
  }
  // Level0.
  pte.SetX(1);
  pte.SetW(1);
  pte.SetR(1);
  for (unsigned k = 0; k < kLevel2ValidEntry; ++k) {
    for (unsigned j = 0; j < k64BitMmuLevelOneSize; ++j) {
      for (unsigned i = 0; i < k64BitMmuLevelZeroSize; ++i) {
        // ((ppn << 12) + offset) will be the physical address. So, x4096 is not needed for ppn.
        uint64_t ppn = k * k64BitMmuLevelOneSize * k64BitMmuLevelZeroSize +
                       j * k64BitMmuLevelZeroSize + i;
        uint32_t address = level0 + ppn * k64BitPteSize;
        pte.SetPpn(ppn);
        memory->Write64(address, pte.GetValue());
      }
    }
  }
  return;
}


void
SetDefaultMmuTable(bool address64bit, std::shared_ptr<MemoryWrapper> memory) {
  if (address64bit) {
    SetDefaultMmuTable64(memory);
  } else {
    SetDefaultMmuTable32(memory);
  }
}

} // anonymous namespace.

int main(int argc, char *argv[]) {
  bool cmdline_error, verbose, address64bit, paging, ecall_emulation, host_emulation;
  std::string filename;

  auto options = ParseCmd(argc, &argv);
  cmdline_error = std::get<0>(options);
  filename = std::get<1>(options);
  verbose = std::get<2>(options);
  address64bit = std::get<3>(options);
  paging = std::get<4>(options);
  ecall_emulation = std::get<5>(options);
  host_emulation = std::get<6>(options);


  if (cmdline_error) {
    std::cerr << "Uasge: " << argv[0] << " elf_file" << "[-v][-64][-p][-e][-h]"
              << std::endl;
    std::cerr << "-v: Verbose" << std::endl;
    std::cerr << "-e: System Call Emulation" << std::endl;
    std::cerr << "-p: Paging Enabled from Start" << std::endl;
    std::cerr << "-64: 64 bit (RV64I) (default is 32 bit mode, RV32I)"
              << std::endl;
    std::cerr << "-h: Use tohost and fromhost function" << std::endl;
    return -1;
  }

  std::cerr << "Elf file name: " << filename << std::endl;
  if (verbose) {
    std::cerr << "Verbose mode." << std::endl;
  }

  std::vector<uint8_t> program = ReadFile(filename);

  auto memory = std::make_shared<MemoryWrapper>(MemoryWrapper());
  LoadElfFile(program, *memory);
  uint32_t entry_point = GetEntryPoint(program);
  std::cerr << "Entry point is 0x" << std::hex << entry_point << std::dec
            << std::endl;

  int64_t global_pointer = GetGlobalPointer(program);
  if (global_pointer == -1) {
    global_pointer = entry_point;
  }
  std::cerr << "Global Pointer is 0x" << std::hex << global_pointer << std::dec
            << std::endl;

  uint32_t sp_value = kTop;

  // Run CPU emulator
  std::cerr << "Execution start" << std::endl;

  RiscvCpu cpu(address64bit);
  cpu.SetEcallEmulationEnable(ecall_emulation);
  cpu.SetHostEmulationEnable(host_emulation);
  cpu.SetRegister(SP, sp_value);
  cpu.SetRegister(GP, global_pointer);
  SetDefaultMmuTable(address64bit, memory);
  uint64_t satp = 0;
  if (paging) {
    std::cerr << "Paging enabled." << std::endl;
    if (address64bit) {
      satp = (k64BitMmuLevel2 >> 12) | (static_cast<uint64_t>(8) << 60);
    } else {
      satp = (k32BitMmuLevel1 >> 12) | (1 << 31);
    }
  }
  cpu.SetCsr(SATP, satp);
  cpu.SetMemory(memory);
  cpu.SetWorkMemory(kTop, kBottom);

  int error = cpu.RunCpu(entry_point, verbose);
  if (error) {
    printf("CPU execution fail.\n");
  }
  int return_value = cpu.ReadRegister(A0);

  std::cerr << "Return GetValue: " << return_value << "." << std::endl;

  return return_value;
}