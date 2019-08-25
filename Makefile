CXX	= g++
CPPFLAGS = -O3 -Wall
TARGET = RISCV_Emulator
OBJS = RISCV_Emulator.o RISCV_cpu.o assembler.o

.PHONY: all
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) -o $(TARGET) $(OBJS)

%.cpp.o.o: %.cpp
	$(CXX) -c $< -o $@ $(CPPFLAF)

.PHONY: clean
clean:
	rm -rf *.o $(TARGET)