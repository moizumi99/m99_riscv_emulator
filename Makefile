CXX	= g++
CPPFLAGS = -O3 -Wall
# TARGET = RISCV_Emulator
# OBJS = RISCV_Emulator.o RISCV_cpu.o load_assembler.o
TESTTARGETS = load_assembler
TEST_OBJS = load_assembler.o load_assembler_test.o bit_tools.o instruction_encdec.o

.PHONY: all
	all: $(TARGET) $(TESTTARGETS)

# $(TARGET): $(OBJS)
# 	$(CXX) -o $(TARGET) $(OBJS)

%.cpp.o.o: %.cpp
	$(CXX) -c $< -o $@ $(CPPFLAF)

.PHONY: test
test: load_assembler
	./load_assembler

# load_assembler: $(TEST_OBJS)
#  	$(CXX) -o $(TESTTARGETS) $(TEST_OBJS)

load_assembler: $(TEST_OBJS)
	g++ -o load_assembler $(TEST_OBJS)

.PHONY: clean
clean:
	rm -rf *.o $(TARGET)