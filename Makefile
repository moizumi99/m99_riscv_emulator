CXX	= g++
CPPFLAGS = -Wall -g
TARGET = RISCV_Emulator
CPU_OBJS = RISCV_cpu.o load_assembler.o assembler.o bit_tools.o instruction_encdec.o
OBJS = RISCV_Emulator.o $(CPU_OBJS)
TEST_TARGETS = load_assembler_test cpu_test

.PHONY: all
	all: $(TARGET) $(TEST_TARGETS)

$(TARGET): $(OBJS)
	$(CXX) -o $(TARGET) $(OBJS)

load_assembler_test: load_assembler.o assembler.o load_assembler_test.o bit_tools.o instruction_encdec.o
	$(CXX) $(CPPFLAG) -o $@ $^

cpu_test: cpu_test.o $(CPU_OBJS)
	$(CXX) -o cpu_test cpu_test.o $(CPU_OBJS)
	
.cpp.o:
	$(CXX) -c $< -o $@ $(CPPFLAGS)

.PHONY: test
test:  $(TEST_TARGETS)
	./load_assembler_test
	./cpu_test

.PHONY: clean
clean:
	rm -rf *.o $(TARGET) $(TEST_TARGETS)