CXX	= g++
CPPFLAGS = -Wall -O3 -I.
TARGET = RISCV_Emulator
CPU_OBJS = RISCV_cpu.o bit_tools.o \
instruction_encdec.o memory_wrapper.o system_call_emulator.o pte.o Mmu.o \
Disassembler.o PeripheralEmulator.o
OBJS = RISCV_Emulator.o $(CPU_OBJS)
TEST_DIR = tests
TEST_TARGETS = $(TEST_DIR)/cpu_test $(TEST_DIR)/pte_test
WRAPPER_TESTS = $(TEST_DIR)/memory_wrapper_test $(TEST_DIR)/load_assembler_test

.PHONY: all
	all: $(TARGET) $(TEST_TARGETS)

$(TARGET): $(OBJS)
	$(CXX) -o $(TARGET) $(OBJS)

$(TEST_DIR)/load_assembler_test: $(TEST_DIR)/load_assembler.o $(TEST_DIR)/assembler.o \
$(TEST_DIR)/load_assembler_test.o bit_tools.o instruction_encdec.o memory_wrapper.o
	$(CXX) $(CPPFLAG) -o $@ $^

$(TEST_DIR)/cpu_test: $(TEST_DIR)/cpu_test.o $(CPU_OBJS) $(TEST_DIR)/load_assembler.o \
$(TEST_DIR)/assembler.o
	$(CXX) $(CPPFLAG) -o $@ $^

$(TEST_DIR)/memory_wrapper_test: memory_wrapper.o $(TEST_DIR)/memory_wrapper_test.o
	$(CXX) $(CPPFLAG) -o $@ $^

$(TEST_DIR)/pte_test: pte.o $(TEST_DIR)/pte_test.o bit_tools.o
	$(CXX) $(CPPFLAG) -o $@ $^

.cpp.o:
	$(CXX) -c $< -o $@ $(CPPFLAGS)

.PHONY: core_test
core_test:  $(TEST_TARGETS) $(TARGET)
	$(TEST_DIR)/cpu_test
	$(TEST_DIR)/pte_test
	$(TEST_DIR)/rv32ui-p-tests.sh
	$(TEST_DIR)/rv32ui-v-tests.sh
	$(TEST_DIR)/rv64ui-p-tests.sh
	$(TEST_DIR)/rv64ui-v-tests.sh
	$(TEST_DIR)/rv32um-p-tests.sh
	$(TEST_DIR)/rv32um-v-tests.sh
	$(TEST_DIR)/rv64um-p-tests.sh
	$(TEST_DIR)/rv64um-v-tests.sh
	$(TEST_DIR)/rv32ua-p-tests.sh
	$(TEST_DIR)/rv32ua-v-tests.sh
	$(TEST_DIR)/rv64ua-p-tests.sh
	$(TEST_DIR)/rv64ua-v-tests.sh
	$(TEST_DIR)/rvuc-tests.sh

.PHONY: wrapper_test
wrapper_test: $(WRAPPER_TESTS)
	$(TEST_DIR)/load_assembler_test
	$(TEST_DIR)/memory_wrapper_test

.PHONY: test
test: wrapper_test core_test

.PHONY: clean
clean:
	rm -rf *.o $(TARGET) $(TEST_TARGETS) $(WRAPPER_TESTS)
