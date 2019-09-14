
#include "RISCV_cpu.h"
#include "bit_tools.h"
#include "load_assembler.h"
#include "assembler.h"
#include <iostream>

using namespace std;

namespace cpu_test {

    uint8_t mem[0x010000];

    bool test_and(bool verbose){
        constexpr int32_t kValue1 = 0x66666666;
        constexpr int32_t kValue2 = 0xF0F0F0F0;
        uint8_t *pointer = mem;
        pointer = add_cmd(pointer, asm_addi(T0, ZERO, kValue1 & 0xFFF));
        pointer = add_cmd(pointer, asm_lui(T0, (kValue1 >> 12) & 0xFFFFF));
        pointer = add_cmd(pointer, asm_addi(T1, ZERO, kValue2 & 0xFFF));
        pointer = add_cmd(pointer, asm_lui(T1, (kValue2 >> 12) & 0xFFFFF));
        pointer = add_cmd(pointer, asm_and(A0, T0, T1));
        pointer = add_cmd(pointer, asm_jalr(ZERO, RA, 0));

        constexpr int kExpectedValue = kValue1 & kValue2;
        int error = run_cpu(mem, 0, verbose);
        bool error_flag = error != 0;
        int return_value = read_register(A0);
        error_flag |= return_value != kExpectedValue;
        if (error_flag & verbose) {
            printf("AND test failed\n");
        }
        return error_flag;
    }

    bool test_and_quiet(bool verbose = true) {
        bool error = test_and(false);
        if (error && verbose) {
            error = test_and(true);
        }
        return error;
    }

    bool test_lui(bool verbose) {
        // LUI test code
        uint8_t *pointer = mem;
        pointer = add_cmd(pointer, asm_add(A0, ZERO, 0));
        pointer = add_cmd(pointer, asm_lui(A0, -300));
        pointer = add_cmd(pointer, asm_jalr(ZERO, RA, 0));

        constexpr int kExpectedValue = -300 * 4096;
        int error = run_cpu(mem, 0, verbose);
        bool error_flag = error != 0;
        int return_value = read_register(A0);
        error_flag |= return_value != kExpectedValue;
        if (error_flag && verbose) {
            printf("LUI test failed\n");
        }
        return error_flag;
    }

    bool test_lui_quiet(bool verbose) {
        bool error = test_lui(false);
        if (error && verbose) {
            error = test_lui(true);
        }
        return error;
    }
    bool test_sum(bool verbose) {
        load_assembler_sum(mem);
        constexpr int kExpectedValue = 55;
        int error = run_cpu(mem, 0, verbose);
        bool error_flag = error != 0;
        int return_value = read_register(A0);
        error_flag |= return_value != kExpectedValue;
        if (error_flag) {
            printf("Sum test failed\n");
        }
        return error_flag;
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

        set_register(A0, kArrayAddress);
        set_register(A1, kArraySize);
        int error = run_cpu(mem, 0, false);
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
    bool verbose = false;

    bool error = false;
    error |= cpu_test::test_and_quiet(verbose);
    error |= cpu_test::test_lui_quiet(verbose);
    error |= cpu_test::test_sum_quiet(verbose);
    error |= cpu_test::test_sort_quiet(verbose);

    if (!error) {
        printf("All CPU Tests passed.\n");
    }
    return error;
}