
#include "RISCV_cpu.h"
#include "bit_tools.h"
#include "load_assembler.h"
#include "assembler.h"
#include <map>
#include <iostream>

using namespace std;


namespace cpu_test {

    constexpr int kMemSize = 0x010000;
    uint8_t mem[kMemSize];

    constexpr int kUnitTestMax = 100;

    void print_error_message(const string &text, bool error, int32_t expected, int32_t actual) {
        if (error) {
            printf("%s test failed.", text.c_str());
        } else {
            printf("%s test passed.", text.c_str());

        }
        printf(" Expected %08x, Actual %08x\n", expected, actual);
        printf(" Expected %d, Actual %d\n", expected, actual);
    }

    enum ITYPE_TEST {
        TEST_ADDI, TEST_ANDI, TEST_ORI, TEST_XORI, TEST_SLLI, TEST_SRLI, TEST_SRAI, TEST_SLTI, TEST_SLTIU
    };

    bool test_i_type(ITYPE_TEST test_type, int32_t rd, int32_t rs1, int32_t value, int32_t imm12, bool verbose) {
        bool error = false;
        int32_t expected;
        string test_case = "";

        uint8_t *pointer = mem;
        uint32_t val20 = value >> 12;
        uint32_t val12 = value & 0xFFF;
        if (val12 >> 11) {
            val20++;
        }
        // Check if the two values make the correct final immediate.
        if ((val20 << 12) + sext(val12, 12) != value) {
            printf("Test bench error. value = %8X, val20 = %8X, val12 = %8X\n", value, val20, sext(val12, 12));
            return true;
        }
        pointer = add_cmd(pointer, asm_lui(rs1, val20));
        pointer = add_cmd(pointer, asm_addi(rs1, rs1, val12));
        if (rs1 == 0) {
            value = 0;
        }
        switch (test_type) {
            case TEST_ADDI:
                pointer = add_cmd(pointer, asm_addi(rd, rs1, imm12));
                expected = value + sext(imm12 & 0x0FFF, 12);
                test_case = "ADDI";
                break;
            case TEST_ANDI:
                pointer = add_cmd(pointer, asm_andi(rd, rs1, imm12));
                expected = value & sext(imm12 & 0x0FFF, 12);
                test_case = "ANDI";
                break;
            case TEST_ORI:
                pointer = add_cmd(pointer, asm_ori(rd, rs1, imm12));
                expected = value | sext(imm12 & 0x0FFF, 12);
                test_case = "ORI";
                break;
            case TEST_XORI:
                pointer = add_cmd(pointer, asm_xori(rd, rs1, imm12));
                expected = value ^ sext(imm12 & 0x0FFF, 12);
                test_case = "XORI";
                break;
            case TEST_SLLI:
                imm12 = imm12 & 0b0011111;
                pointer = add_cmd(pointer, asm_slli(rd, rs1, imm12));
                expected = value << imm12;
                test_case = "SLLI";
                break;
            case TEST_SRLI:
                imm12 = imm12 & 0b0011111;
                pointer = add_cmd(pointer, asm_srli(rd, rs1, imm12));
                expected = value >> imm12;
                test_case = "SRLI";
                break;
            case TEST_SRAI:
                imm12 = imm12 & 0b0011111;
                pointer = add_cmd(pointer, asm_srai(rd, rs1, imm12));
                expected = value >> imm12;
                test_case = "SRAI";
                break;
            case TEST_SLTI:
                pointer = add_cmd(pointer, asm_slti(rd, rs1, imm12));
                expected = value < imm12 ? 1 : 0;
                test_case = "SLTI";
                break;
            case TEST_SLTIU:
                pointer = add_cmd(pointer, asm_sltiu(rd, rs1, imm12));
                expected = static_cast<uint32_t>(value) < static_cast<uint32_t >(imm12) ? 1 : 0;
                test_case = "SLTIU";
                break;
            default:
                printf("I TYPE Test case undefined.\n");
                return true;
        }
        pointer = add_cmd(pointer, asm_addi(A0, rd, 0));
        pointer = add_cmd(pointer, asm_jalr(ZERO, RA, 0));

        if (rd == 0) {
            expected = 0;
        }
        RiscvCpu cpu;
        error = cpu.run_cpu(mem, 0, verbose) != 0;
        int return_value = cpu.read_register(A0);
        error |= return_value != expected;
        if (error & verbose) {
            printf("RD: %d, RS1: %d, Value: %d(%08x), imm12: %d(%03x)\n", rd, rs1, value, value, imm12, imm12);
        }
        if (verbose) {
            print_error_message(test_case, error, expected, return_value);
        }
        return error;
    }

    void print_i_type_instruction_message(ITYPE_TEST test_case, bool error) {
        map<ITYPE_TEST, const string> test_name = {{TEST_ADDI,  "ADDI"},
                                                   {TEST_ANDI,  "ANDI"},
                                                   {TEST_ORI,   "ORI"},
                                                   {TEST_XORI,  "XORI"},
                                                   {TEST_SLLI,  "SLLI"},
                                                   {TEST_SRLI,  "SRLI"},
                                                   {TEST_SRAI,  "SRAI"},
                                                   {TEST_SLTI,  "SLTI"},
                                                   {TEST_SLTIU, "SLTIU"}};
        printf("%s test %s.\n", test_name[test_case].c_str(), error ? "failed" : "passed");
    }

    bool test_i_type_loop(bool verbose) {
        bool total_error = false;
        ITYPE_TEST test_set[] = {TEST_ADDI, TEST_ANDI, TEST_ORI, TEST_XORI, TEST_SLLI, TEST_SRLI, TEST_SRAI, TEST_SLTI,
                                 TEST_SLTIU};
        for (ITYPE_TEST test_case: test_set) {
            bool error = false;
            for (int i = 0; i < kUnitTestMax && !error; i++) {
                uint32_t rd = rand() & 0x1F;
                uint32_t rs1 = rand() & 0x1F;
                int32_t value = rand();
                int32_t imm12 = sext(rand() & 0x0FFF, 12);
                bool test_error = test_i_type(test_case, rd, rs1, value, imm12, false);
                if (test_error) {
                    test_error |= test_i_type(test_case, rd, rs1, value, imm12, true);
                }
                error |= test_error;
            }
            if (verbose) {
                print_i_type_instruction_message(test_case, error);
            }
            total_error |= error;
        }
        return total_error;
    }

    enum R_TYPE_TEST {
        TEST_ADD, TEST_SUB, TEST_AND, TEST_OR, TEST_XOR, TEST_SLL, TEST_SRL, TEST_SRA, TEST_SLT, TEST_SLTU
    };

    bool
    test_r_type(R_TYPE_TEST test_type, int32_t rd, int32_t rs1, int32_t rs2, int32_t value1, int32_t value2,
                bool verbose) {
        bool error = false;
        int32_t expected;
        string test_case = "";

        uint8_t *pointer = mem;
        uint32_t val20 = value1 >> 12;
        uint32_t val12 = value1 & 0xFFF;
        if (val12 >> 11) {
            val20++;
        }
        if ((val20 << 12) + sext(val12, 12) != value1) {
            printf("Test bench error: value1 = %8X, val20 = %8X, val12 = %8X\n", value1, val20, val12);
        }
        pointer = add_cmd(pointer, asm_lui(rs1, val20));
        pointer = add_cmd(pointer, asm_addi(rs1, rs1, val12));
        val20 = value2 >> 12;
        val12 = value2 & 0xFFF;
        if (val12 >> 11) {
            val20++;
        }
        if ((val20 << 12) + sext(val12, 12) != value2) {
            printf("Test bench error: value2 = %8X, val20 = %8X, val12 = %8X\n", value2, val20, val12);
        }
        pointer = add_cmd(pointer, asm_lui(rs2, val20));
        pointer = add_cmd(pointer, asm_addi(rs2, rs2, val12));

        if (rs1 == 0) {
            value1 = 0;
        }
        if (rs2 == 0) {
            value2 = 0;
        }
        if (rs1 == rs2) {
            value1 = value2;
        }
        switch (test_type) {
            case TEST_ADD:
                pointer = add_cmd(pointer, asm_add(rd, rs1, rs2));
                expected = value1 + value2;
                test_case = "ADD";
                break;
            case TEST_SUB:
                pointer = add_cmd(pointer, asm_sub(rd, rs1, rs2));
                expected = value1 - value2;
                test_case = "SUB";
                break;
            case TEST_AND:
                pointer = add_cmd(pointer, asm_and(rd, rs1, rs2));
                expected = value1 & value2;
                test_case = "AND";
                break;
            case TEST_OR:
                pointer = add_cmd(pointer, asm_or(rd, rs1, rs2));
                expected = value1 | value2;
                test_case = "OR";
                break;
            case TEST_XOR:
                pointer = add_cmd(pointer, asm_xor(rd, rs1, rs2));
                expected = value1 ^ value2;
                test_case = "XOR";
                break;
            case TEST_SLL:
                pointer = add_cmd(pointer, asm_sll(rd, rs1, rs2));
                expected = value1 << (value2 & 0x1F);
                test_case = "SLL";
                break;
            case TEST_SRL:
                pointer = add_cmd(pointer, asm_srl(rd, rs1, rs2));
                expected = static_cast<uint32_t>(value1) >> (value2 & 0x1F);
                test_case = "SRL";
                break;
            case TEST_SRA:
                pointer = add_cmd(pointer, asm_sra(rd, rs1, rs2));
                expected = value1 >> (value2 & 0x1F);
                test_case = "SRA";
                break;
            case TEST_SLT:
                pointer = add_cmd(pointer, asm_slt(rd, rs1, rs2));
                expected = (value1 < value2) ? 1 : 0;
                test_case = "SLT";
                break;
            case TEST_SLTU:
                pointer = add_cmd(pointer, asm_sltu(rd, rs1, rs2));
                expected = (static_cast<uint32_t>(value1) < static_cast<uint32_t>(value2)) ? 1 : 0;
                test_case = "SLTU";
                break;
            default:
                if (verbose) {
                    printf("Undefined test case.\n");
                }
                return true;
        }
        pointer = add_cmd(pointer, asm_addi(A0, rd, 0));
        add_cmd(pointer, asm_jalr(ZERO, RA, 0));

        if (rd == 0) {
            expected = 0;
        }
        RiscvCpu cpu;
        error = cpu.run_cpu(mem, 0, verbose) != 0;
        int return_value = cpu.read_register(A0);
        error |= return_value != expected;
        if (error & verbose) {
            printf("RD: %d, RS1: %d, RS2: %d, Value1: %d(%08x), value2: %d(%03x)\n", rd, rs1, rs2, value1, value1,
                   value2, value2);
        }
        if (verbose) {
            print_error_message(test_case, error, expected, return_value);
        }
        return error;
    }

    void print_r_type_instruction_message(R_TYPE_TEST test_case, bool error) {
        map<R_TYPE_TEST, const string> test_name = {{TEST_ADD,  "ADD"},
                                                    {TEST_SUB,  "SUB"},
                                                    {TEST_AND,  "AND"},
                                                    {TEST_OR,   "OR"},
                                                    {TEST_XOR,  "XOR"},
                                                    {TEST_SLL,  "SLL"},
                                                    {TEST_SRL,  "SRL"},
                                                    {TEST_SRA,  "SRA"},
                                                    {TEST_SLT,  "SLT"},
                                                    {TEST_SLTU, "SLTU"}};
        printf("%s test %s.\n", test_name[test_case].c_str(), error ? "failed" : "passed");
    }

    bool test_r_type_loop(bool verbose = true) {
        bool total_error = false;
        R_TYPE_TEST test_sets[] = {TEST_ADD, TEST_SUB, TEST_AND, TEST_OR, TEST_XOR, TEST_SLL, TEST_SRL, TEST_SRA,
                                   TEST_SLT,
                                   TEST_SLTU};
        for (R_TYPE_TEST test_case: test_sets) {
            bool error = false;
            for (int i = 0; i < kUnitTestMax && !error; i++) {
                int32_t rd = rand() & 0x1F;
                int32_t rs1 = rand() & 0x1F;
                int32_t rs2 = rand() & 0x1F;
                int32_t value1 = static_cast<int32_t>(rand());
                int32_t value2 = static_cast<int32_t>(rand());
                bool test_error = test_r_type(test_case, rd, rs1, rs2, value1, value2, false);
                if (test_error && verbose) {
                    test_error = test_r_type(test_case, rd, rs1, rs2, value1, value2, true);
                }
                error |= test_error;
            }
            if (verbose) {
                print_r_type_instruction_message(test_case, error);
            }
            total_error |= error;
        }
        return total_error;
    }

    bool test_auipc(int32_t rd, int32_t val, int32_t offset, bool verbose) {
        uint8_t *pointer = mem;
        add_cmd(pointer, asm_jal(ZERO, offset));
        pointer = mem + offset;
        pointer = add_cmd(pointer, asm_auipc(rd, val));
        pointer = add_cmd(pointer, asm_addi(A0, rd, 0));
        pointer = add_cmd(pointer, asm_jalr(ZERO, RA, 0));

        int32_t expected = offset + (val << 12);
        if (rd == 0) {
            expected = 0;
        }
        RiscvCpu cpu;
        bool error = cpu.run_cpu(mem, 0, verbose) != 0;
        int return_value = cpu.read_register(A0);
        error |= return_value != expected;
        if (verbose) {
            print_error_message("AUIPC", error, expected, return_value);
        }
        return error;
    }

    bool test_auipc_loop(bool verbose) {
        bool error = false;
        for (int i = 0; i < kUnitTestMax && !error; i++) {
            int32_t value = static_cast<int32_t>(rand()) & 0x0FFFFF;
            int32_t offset = static_cast<uint32_t>(rand()) & 0x0FFF0;
            int32_t rd = rand() % 32;
            bool test_error = test_auipc(A1, 0, 16, false);
            if (test_error && verbose) {
                test_error |= test_auipc(A1, 0, 16, true);
            }
            error |= test_error;
        }
        if (verbose) {
            printf("AUIPC test %s.\n", error ? "failed" : "passed");
        }
        return error;
    }

    bool test_lui(int32_t val, bool verbose) {
        // LUI test code
        uint8_t *pointer = mem;
        pointer = add_cmd(pointer, asm_add(A0, ZERO, 0));
        pointer = add_cmd(pointer, asm_lui(A0, val >> 12));
        pointer = add_cmd(pointer, asm_jalr(ZERO, RA, 0));

        int32_t expected = val & 0xFFFFF000;
        RiscvCpu cpu;
        bool error = cpu.run_cpu(mem, 0, verbose) != 0;
        int return_value = cpu.read_register(A0);
        error |= return_value != expected;
        if (verbose) {
            print_error_message("LUI", error, expected, return_value);
        }
        return error;
    }

    bool test_lui_loop(bool verbose) {
        bool error = false;
        for (int i = 0; i < kUnitTestMax && !error; i++) {
            int32_t value = static_cast<int32_t>(rand());
            bool test_error = test_lui(value, false);
            if (test_error && verbose) {
                test_error = test_lui(value, true);
            }
            error |= test_error;
        }
        if (verbose) {
            printf("LUI test %s.\n", error ? "failed" : "passed");
        }
        return error;
    }

    bool test_load(uint32_t rd, uint32_t rs1, uint32_t offset0, uint32_t offset1, uint32_t val, bool verbose) {
        if (rs1 == ZERO) {
            offset0 = 0;
        }
        uint32_t address = offset0 + offset1;
        mem[address] = val & 0xff;
        mem[address + 1] = (val >> 8) & 0xff;
        mem[address + 2] = (val >> 16) & 0xff;
        mem[address + 3] = (val >> 24) & 0xff;
        // LW test code
        uint8_t *pointer = mem;
        pointer = add_cmd(pointer, asm_addi(rs1, ZERO, offset0 & 0x0FFF));
        pointer = add_cmd(pointer, asm_lui(rs1, offset0 >> 12));
        pointer = add_cmd(pointer, asm_lw(rd, rs1, offset1));
        pointer = add_cmd(pointer, asm_addi(A0, rd, 0));
        pointer = add_cmd(pointer, asm_jalr(ZERO, RA, 0));

        int32_t expected = val;
        RiscvCpu cpu;
        bool error = cpu.run_cpu(mem, 0, verbose) != 0;
        int return_value = cpu.read_register(A0);
        error |= return_value != expected;
        if (verbose) {
            print_error_message("LW", error, expected, return_value);
            if (error) {
                printf("rd: %d, rs1: %d, offset0: %d, offset1: %d, val: %d\n", rd, rs1, offset0, offset1, val);
            }
        }
        return error;
    }

    bool test_load_loop(bool verbose) {
        bool error = false;
        for (int i = 0; i < kUnitTestMax && !error; i++) {
            uint32_t rs1 = rand() % 32;
            uint32_t rd = A1;
            uint32_t offset0 = 0, offset1 = 0;
            while (offset0 + offset1 < 16 || offset0 + offset1 >= kMemSize - 4) {
                offset0 = rand() % kMemSize;
                offset1 = rand() & 0x0FFF;
                if (rs1 == ZERO) {
                    offset0 = 0;
                }
            }
            uint32_t val = rand() & 0x0FFFFFFFF;
            bool test_error = test_load(rd, rs1, offset0, offset1, val, false);
            if (test_error && verbose) {
                test_error = test_load(rd, rs1, offset0, offset1, val, true);
            }
            error |= test_error;
        }
        if (verbose) {
            printf("Load test %s", error ? "failed" : "passed");
        }
        return error;
    }

    bool test_sum(bool verbose) {
        load_assembler_sum(mem);
        constexpr int kExpectedValue = 55;
        RiscvCpu cpu;
        bool error = cpu.run_cpu(mem, 0, verbose) != 0;
        int return_value = cpu.read_register(A0);
        error |= return_value != kExpectedValue;
        if (verbose) {
            print_error_message("Summation", error, kExpectedValue, return_value);
        }
        return error;
    }

    bool test_sum_quiet(bool verbose) {
        bool error = test_sum(false);
        if (error & verbose) {
            error = test_sum(true);
        }
        return error;
    }

    bool test_sort(bool verbose) {
        load_assembler_sort(mem);

        constexpr int kArraySize = 100;
        constexpr int kArrayAddress = 512;
        for (int i = 0; i < kArraySize; i++) {
            int value = rand() % 1000;
            store_wd(mem + kArrayAddress + i * 4, value);
        }

        if (verbose) {
            printf("Before:\n");
            for (int i = 0; i < kArraySize; i++) {
                printf("%d\t", load_wd(mem + kArrayAddress + i * 4));
            }
            printf("\n");
        }

        RiscvCpu cpu;
        cpu.set_register(A0, kArrayAddress);
        cpu.set_register(A1, kArraySize);
        int error = cpu.run_cpu(mem, 0, verbose);
        bool error_flag = error != 0;

        if (error_flag) {
            printf("CPU exection error\n");
        }

        for (int i = 0; i < kArraySize - 1; i++) {
            error_flag |= load_wd(mem + kArrayAddress + i * 4) >
                          load_wd(mem + kArrayAddress + i * 4 + 4);
        }

        if (verbose) {
            printf("After:\n");
            for (int i = 0; i < kArraySize; i++) {
                int32_t data = load_wd(mem + kArrayAddress + i * 4);
                printf("%d\t", data);
            }
        }

        printf("\n");
        if (error_flag) {
            printf("Sort test failed\n");
        }
        return error_flag;
    }

    bool test_sort_quiet(bool verbose) {
        bool error = test_sort(false);
        if (error & verbose) {
            error = test_sort(true);
        }
        return error;
    }

} // namespace cpu_test

int main() {
    bool verbose = true;

    bool error = false;
    error |= cpu_test::test_i_type_loop(verbose);
    error |= cpu_test::test_r_type_loop(verbose);
    error |= cpu_test::test_lui_loop(verbose);
    error |= cpu_test::test_auipc_loop(verbose);
//    error |= cpu_test::test_load_loop(verbose);
    error |= cpu_test::test_sum_quiet(verbose);
    error |= cpu_test::test_sort_quiet(verbose);

    if (error) {
        printf("\nCPU Test failed.\n");
    } else {
        printf("\nAll CPU Tests passed.\n");
    }
    return error;
}