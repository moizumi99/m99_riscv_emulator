CXX	= g++
CPPFLAGS = -Wall -g
TARGET = RISCV_Emulator
CPU_OBJS = RISCV_cpu.o load_assembler.o assembler.o bit_tools.o \
instruction_encdec.o memory_wrapper.o system_call_emulator.o pte.o Mmu.o
OBJS = RISCV_Emulator.o $(CPU_OBJS)
TEST_TARGETS = cpu_test pte_test
WRAPPER_TESTS = memory_wrapper_test load_assembler_test

.PHONY: all
	all: $(TARGET) $(TEST_TARGETS)

$(TARGET): $(OBJS)
	$(CXX) -o $(TARGET) $(OBJS)

load_assembler_test: load_assembler.o assembler.o load_assembler_test.o bit_tools.o instruction_encdec.o memory_wrapper.o
	$(CXX) $(CPPFLAG) -o $@ $^

cpu_test: cpu_test.o $(CPU_OBJS)
	$(CXX) -o cpu_test cpu_test.o $(CPU_OBJS)

memory_wrapper_test: memory_wrapper.o memory_wrapper_test.o
	$(CXX) memory_wrapper_test.o memory_wrapper.o -o memory_wrapper_test

pte_test: pte.o pte_test.o bit_tools.o
	$(CXX) pte_test.o pte.o bit_tools.o -o pte_test

.cpp.o:
	$(CXX) -c $< -o $@ $(CPPFLAGS)

.PHONY: core_test
core_test:  $(TEST_TARGETS) $(TARGET)
	./cpu_test
	./pte_test
	./rv32ui-p-tests.sh
	./rv32ui-v-tests.sh
	./rv64ui-p-tests.sh
	./rv64ui-v-tests.sh

.PHONY: wrapper_test
wrapper_test: $(WRAPPER_TESTS)
	./load_assembler_test
	./memory_wrapper_test

.PHONY: test
test: wrapper_test core_test

.PHONY: clean
clean:
	rm -rf *.o $(TARGET) $(TEST_TARGETS) $(WRAPPER_TESTS)
