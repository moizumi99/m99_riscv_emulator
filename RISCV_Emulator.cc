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
    std::cerr << "File size = " << size << "." << std::endl;
    exit(-1);
  }

  // read the data:
  std::vector<uint8_t> fileData;
  fileData.resize((unsigned int)size);
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

Elf32_Ehdr *get_ehdr(std::vector<uint8_t> &program) {
  Elf32_Ehdr *ehdr = (Elf32_Ehdr *) reinterpret_cast<Elf32_Ehdr *>(program.data());
  return ehdr;
}

Elf32_Shdr *get_shdr(std::vector<uint8_t > &program, int index) {
  Elf32_Ehdr *ehdr = get_ehdr(program);
  if (index < 0 || index >= ehdr->e_shnum) {
    std::cerr << "Section header " << index << " not found." << std::endl;
    return(NULL);
  }
  Elf32_Shdr *shdr = (Elf32_Shdr *)(program.data() + ehdr->e_shoff + ehdr->e_shentsize * index);
  return shdr;
}

char *get_section_name(std::vector<uint8_t> &program, Elf32_Shdr *shdr) {
  Elf32_Ehdr *ehdr = get_ehdr(program);
  Elf32_Shdr *nhdr = get_shdr(program, ehdr->e_shstrndx);
  return reinterpret_cast<char *>(program.data()) + nhdr->sh_offset + shdr->sh_name;
}

Elf32_Shdr *search_shdr(std::vector<uint8_t> &program, std::string name) {
  Elf32_Ehdr *ehdr = get_ehdr(program);

  // Find the last section header that has the name information.
  Elf32_Shdr *nhdr = get_shdr(program, ehdr->e_shstrndx);
  for(int i = 0; i < ehdr->e_shnum; i++) {
    Elf32_Shdr *shdr = get_shdr(program, i);
    char *section_name = get_section_name(program, shdr);
//    char *section_name = reinterpret_cast<char *>(program.data()) + nhdr->sh_offset + shdr->sh_name;
    if (!std::strcmp(section_name, name.c_str())) {
      std::cerr << "Section " << name << " found at 0x0" << std::hex
                << shdr->sh_offset << "." << std::endl;
      return shdr;
    }
  }
  return NULL;
}

Elf32_Shdr *search_shdr(std::vector<uint8_t> &program, Elf32_Word type) {
  Elf32_Ehdr *ehdr = get_ehdr(program);

  // Find the last section header that has the name information.
  for(int i = 0; i < ehdr->e_shnum; i++) {
    Elf32_Shdr *shdr = get_shdr(program, i);
    if (shdr->sh_type == type) {
      char *section_name = get_section_name(program, shdr);
      std::cerr << "Section " << section_name << "(" << shdr->sh_type << ") found at 0x0" << std::hex
                << shdr->sh_offset << "." << std::endl;
      return shdr;
    }
  }
  return NULL;
}

void extend_mem_size(std::vector<uint8_t> &memory, size_t new_size) {
  if (memory.size() < new_size) {
    new_size = (new_size + kUnitSize - 1) / kUnitSize * kUnitSize;
    memory.resize(new_size);
    std::cerr << "\nMemory size extended to " << std::hex << new_size << std::endl;
  }
}

void loadElfFile(std::vector<uint8_t> &program, memory_wrapper &memory) {
  Elf32_Ehdr *ehdr = get_ehdr(program);
  if (isRightElf(ehdr)) {
    std::cerr << "This is a supported RISC-V 32bit Elf file" << std::endl;
  }

  for(int i = 0; i < ehdr->e_phnum; i++) {
    std::cerr << "Program Header " << i << ":";
    Elf32_Phdr *phdr = (Elf32_Phdr *)(program.data() + ehdr->e_phoff + ehdr->e_phentsize * i);
    switch (phdr->p_type) {
      int total_new_size;

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
  Elf32_Shdr *shdr = search_shdr(program, ".bss");
  if (shdr) {
    std::cerr << "Secure BSS." << std::endl;
    int total_new_size = shdr->sh_addr + shdr->sh_size;
    extend_mem_size(program, total_new_size);
  } else {
    std::cerr << "No BSS found." << std::endl;
  }
}

Elf32_Sym *find_symbol(std::vector<uint8_t> &program, std::string target_name) {
  // Find the symbol table.
  Elf32_Shdr *shdr = search_shdr(program, SHT_SYMTAB);
  if (!shdr) {
    std::cerr << "Symbol table not found." << std::endl;
    return NULL;
  }

  int number = shdr->sh_size / sizeof(Elf32_Sym);
  std::cerr << "Number of symbols = " << std::dec << number << ", (" << shdr->sh_size << " bytes)" << std::endl;

  Elf32_Shdr *strtab_shdr = search_shdr(program, ".strtab");
  if (!strtab_shdr) {
    std::cerr << ".strtab not found." << std::endl;
    return NULL;
  }

  for (int i = 0; i < number; i++) {
    Elf32_Sym *symbol = (Elf32_Sym *)(program.data() + shdr->sh_offset + i * sizeof(Elf32_Sym));
//    std::cerr << "Symbol name offset = " << symbol->st_name << "." << std::endl;
    char *symbol_name = (char *)(program.data()) + strtab_shdr->sh_offset + symbol->st_name;
//    std::cerr << "Symbol: " << symbol_name << " found. Size =" << symbol->st_size << std::endl;
    if (!strcmp(symbol_name, target_name.c_str())) {
      std::cerr << "Symbol \"" << target_name << "\" found at index "  << i << "." << std::endl;
      return symbol;
    }
  }
  return NULL;
}

int get_global_pointer(std::vector<uint8_t> &program) {
  std::string target_name = "__global_pointer$";
  Elf32_Sym *symbol = find_symbol(program, target_name);
  if (symbol) {
    std::cerr << "Global Pointer Value = 0x" << std::hex
              << symbol->st_value << std::dec << "." << std::endl;
    return symbol->st_value;
  }
  std::cerr << "Global Pointer Value not defined. Set to zero." << std::endl;
  return 0;
}

int get_entry_point(std::vector<uint8_t> &program) {
  Elf32_Ehdr *ehdr = get_ehdr(program);
  return ehdr->e_entry;
}

std::tuple<bool, std::string, bool>parse_cmd(int argc, char *(*argv[])) {
  bool error = false;
  bool verbose = false;
  std::string filename = "";
  if (argc < 2) {
    error = true;
  } else {
    for(int i = 1; i < argc; i++) {
      if ((*argv)[i][0] == '-') {
        if ((*argv)[i][1] == 'v') {
          verbose = true;
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
  return std::make_tuple(error, filename, verbose);
}

constexpr int kMmuLevelOneSize = 4096; // 4096 x 4 B = 16 KiB.
constexpr int kMmuLevelZeroSize = 1024; // 1024 x 4 B = 4 KiB.
constexpr int kPteSize = 4;

void set_default_mmu_table(uint32_t level1, uint32_t level0, std::shared_ptr<memory_wrapper> memory) {
  memory_wrapper& mem = *memory;
  // Level1.
  Pte pte(0);
  pte.SetV(1);
  for (int i = 0; i < kMmuLevelOneSize; ++i) {
    uint32_t address = level1 + i * kPteSize;
    uint32_t ppn = (level0 + i * kMmuLevelZeroSize * kPteSize) >> 12;
    pte.SetPpn(ppn);
    uint32_t pte_value = pte.GetValue();
    memory->write32(address, pte_value);
  }
  // Level0.
  pte.SetX(1);
  pte.SetW(1);
  pte.SetR(1);
  for (int j = 0; j < kMmuLevelOneSize; ++j) {
    for (int i = 0; i < kMmuLevelZeroSize; ++i) {
      uint32_t address = level0 + (j * kMmuLevelZeroSize + i) * kPteSize;
      assert(address < level1);
      // ((ppn << 12) + offset) will be the physical address. So, x4096 is not needed here.
      uint32_t ppn = j * kMmuLevelZeroSize + i;
      pte.SetPpn(ppn);
      memory->write32(address, pte.GetValue());
    }
  }
  return;
}

int main(int argc, char *argv[]) {
  bool cmdline_error, verbose;
  std::string filename;

  std::tie(cmdline_error, filename, verbose) = parse_cmd(argc, &argv);

  if (cmdline_error) {
    std::cerr << "Uasge: "  << argv[0] << " elf_file" << "[-v]" << std::endl;
    std::cerr << "-v: Verbose" << std::endl;
    return -1;
  }

  std::cerr << "Elf file name: " << filename << std::endl;
  if (verbose) {
    std::cerr << "Verbose mode." << std::endl;
  }

  std::vector<uint8_t> program = readFile(filename);

  auto memory = std::make_shared<memory_wrapper>(memory_wrapper());
  loadElfFile(program, *memory);
  int entry_point = get_entry_point(program);
  std::cerr << "Entry point is 0x" << std::hex << entry_point << std::dec << std::endl;

  int global_pointer = get_global_pointer(program);
  std::cerr << "Global Pointer is 0x" << std::hex << global_pointer << std::dec << std::endl;

 int sp_value = kTop;

  // Run CPU emulator
  std::cerr << "Execution start" << std::endl;

  RiscvCpu cpu;
  cpu.set_register(SP, sp_value);
  cpu.set_register(GP, global_pointer);
  set_default_mmu_table(mmu_level1, mmu_level0, memory);
  // Enable paging by setting mode (31st bit).
  uint32_t sptbr = (mmu_level1 >> 12) | (1 << 31);
  cpu.set_csr(kSptbr, sptbr);
  cpu.set_memory(memory);
  cpu.set_work_memory(kTop, kBottom);

  int error = cpu.run_cpu(entry_point, verbose);
  if (error) {
    printf("CPU execution fail.\n");
  }
  int return_value = cpu.read_register(A0);

  std::cerr << "Return value: " << return_value << "." << std::endl;

  return return_value;
}