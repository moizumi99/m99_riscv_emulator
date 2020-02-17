CXX	= g++
CPPFLAGS = -Wall -g
TARGET = RISCV_Emulator
CPU_OBJS = RISCV_cpu.o load_assembler.o assembler.o bit_tools.o instruction_encdec.o memory_wrapper.o system_call_emulator.o pte.o
OBJS = RISCV_Emulator.o $(CPU_OBJS)
TEST_TARGETS = load_assembler_test cpu_test pte_test

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

.PHONY: test
test:  $(TEST_TARGETS) $(TARGET)
	./load_assembler_test
	./cpu_test
	./memory_wrapper_test
	./pte_test

.PHONY: clean
clean:
	rm -rf *.o $(TARGET) $(TEST_TARGETS)