CXX	= g++
CPPFLAGS = -Wall -g
TARGET = RISCV_Emulator
OBJS = RISCV_Emulator.o RISCV_cpu.o load_assembler.o bit_tools.o instruction_encdec.o
TESTTARGETS = load_assembler_test
TEST_OBJS = load_assembler.o load_assembler_test.o bit_tools.o instruction_encdec.o

.PHONY: all
	all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) -o $(TARGET) $(OBJS)

%.cpp.o.o: %.cpp
	$(CXX) -c $< -o $@ $(CPPFLAF)

.PHONY: test
test:  $(TESTTARGETS)
	./$<

load_assembler_test: $(TEST_OBJS)
	g++ -o load_assembler_test $(TEST_OBJS)

.PHONY: clean
clean:
	rm -rf *.o $(TARGET)