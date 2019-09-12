
#include "RISCV_cpu.h"
#include "bit_tools.h"
#include "load_assembler.h"
#include <iostream>

using namespace std;

namespace cpu_test {

    uint8_t mem[0x010000];

    bool test_and(bool verbose) {
        load_assembler_and(mem);
        constexpr int kExpectedValue = 0b11111111111111111111101000001010;
        int error = run_cpu(mem, 0, verbose);
        bool error_flag = error != 0;
        int return_value = read_register(A0);
        error_flag |= return_value != kExpectedValue;
        if (error_flag) {
            printf("AND test failed\n");
        }
        return error_flag;
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

} // namespace cpu_test

int main() {
    bool verbose = false;

    bool error = false;
    error |= cpu_test::test_and(verbose);
    error |= cpu_test::test_sum(verbose);
    error |= cpu_test::test_sort(verbose);

    if (!error) {
        printf("All CPU Tests passed.\n");
    }
    return error;
}