// CPU_emulator.c  
#include <stdio.h>

#define MOV		 0
#define ADD		 1
#define SUB	     2
#define AND	     3
#define OR	     4
#define SL	     5
#define SR	     6
#define SRA	     7
#define LDL		 8
#define LDH		 9
#define CMP		10
#define JE		11
#define JMP		12
#define LD		13
#define ST		14
#define HLT		15
#define REG0	0
#define REG1	1
#define REG2	2
#define REG3	3
#define REG4	4
#define REG5	5
#define REG6	6
#define REG7	7
// �z��̐錾
short	reg[8];
short	rom[256];
short	ram[256];

void	assembler(void);

short mov(short, short);
short add(short, short);
short sub(short, short);
short and(short, short);
short or (short, short);
short sl(short);
short sr(short);
short sra(short);
short ldl(short, short);
short ldh(short, short);
short cmp(short, short);
short je(short);
short jmp(short);
short ld(short, short);
short st(short, short);
short hlt(void);
short op_code(short);
short op_regA(short);
short op_regB(short);
short op_data(short);
short op_addr(short);

void main(void) {
	// �ϐ��̒�`
	short	pc;			// �v���O�����J�E���^ 
	short	ir;			// �C���X�g���N�V�������W�X�^
	short	flag_eq;	// ��r�p�t���O

	assembler();

	pc = 0;
	flag_eq = 0;

	do {

		ir = rom[pc];
		printf(" %5d  %5x  %5d  %5d  %5d  %5d\n",
			pc, ir, reg[0], reg[1], reg[2], reg[3]);

		pc = pc + 1;

		switch (op_code(ir)) {
			case MOV:	reg[op_regA(ir)] = reg[op_regB(ir)];
						break;
			case ADD:	reg[op_regA(ir)] = reg[op_regA(ir)] + reg[op_regB(ir)];
						break;
			case SUB:	reg[op_regA(ir)] = reg[op_regA(ir)] - reg[op_regB(ir)];
						break;
			case AND:	reg[op_regA(ir)] = reg[op_regA(ir)] & reg[op_regB(ir)];
						break;
			case OR:	reg[op_regA(ir)] = reg[op_regA(ir)] | reg[op_regB(ir)];
						break;
			case SL:	reg[op_regA(ir)] = reg[op_regA(ir)] << 1;
						break;
			case SR:	reg[op_regA(ir)] = reg[op_regA(ir)] >> 1;
						break;
			case SRA:	reg[op_regA(ir)] = (reg[op_regA(ir)] & 0x8000) | (reg[op_regA(ir)] >> 1);
						break;
			case LDL:	reg[op_regA(ir)] = (reg[op_regA(ir)] & 0xff00) | (op_data(ir) & 0x00ff);
						break;
			case LDH:	reg[op_regA(ir)] = (op_data(ir) << 8) | (reg[op_regA(ir)] & 0x00ff);
						break;
			case CMP:	if (reg[op_regA(ir)] == reg[op_regB(ir)]) {
							flag_eq = 1;
						}else {
							flag_eq = 0;
						}
						break;
			case JE:	if (flag_eq == 1) pc = op_addr(ir);
						break;
			case JMP:	pc = op_addr(ir);
						break;
			case LD:	reg[op_regA(ir)] = ram[op_addr(ir)];
						break;
			case ST:	ram[op_addr(ir)] = reg[op_regA(ir)];
						break;
			default:
						break;
		}
	} while (op_code(ir) != HLT);

	printf("ram[64] = %d \n", ram[64]);
}

void assembler(void) {
	// 1+2+�c+10=55�̌v�Z��
	rom[0] = ldh(REG0, 0);			// REG0(H) ��  0
	rom[1] = ldl(REG0, 0);			// REG0(L) ��  0
	rom[2] = ldh(REG1, 0);			// REG1(H) ��  0
	rom[3] = ldl(REG1, 1);			// REG1(L) ��  1
	rom[4] = ldh(REG2, 0);			// REG2(H) ��  0
	rom[5] = ldl(REG2, 0);			// REG2(L) ��  0
	rom[6] = ldh(REG3, 0);			// REG3(H) ��  0
	rom[7] = ldl(REG3, 10);			// REG3(L) ��  10
	rom[8] = add(REG2, REG1);		// REG2 �� REG2 + REG1
	rom[9] = add(REG0, REG2);		// REG0 �� REG0 + REG2
	rom[10] = st(REG0, 64);			// REG0��������(I/O)��64�Ԓn�ɕۑ�
	rom[11] = cmp(REG2, REG3);		// REG2��REG3���r
	rom[12] = je(14);				// ��v������14�Ԓn�ɃW�����v
	rom[13] = jmp(8);				// ��������8�Ԓn�ɃW�����v
	rom[14] = hlt();				// CPU�̒�~

}

// �֐�mov(move)�{��
short mov(short ra, short rb) {
	return ((MOV << 11) | (ra << 8) | (rb << 5));
}
// �֐�add(addition)�{��
short add(short ra, short rb) {
	return ((ADD << 11) | (ra << 8) | (rb << 5));
}
// �֐�sub(subtraction)�{��
short sub(short ra, short rb) {
	return ((SUB << 11) | (ra << 8) | (rb << 5));
}
// �֐�and(logical and)�{��
short and(short ra, short rb) {
	return ((AND << 11) | (ra << 8) | (rb << 5));
}
// �֐�or(logical or)�{��
short or (short ra, short rb) {
	return ((OR << 11) | (ra << 8) | (rb << 5));
}
// �֐�sl(shift left)�{��
short sl(short ra) {
	return ((SL << 11) | (ra << 8));
}
// �֐�sr(shift right)�{��
short sr(short ra) {
	return ((SR << 11) | (ra << 8));
}
// �֐�sra(shift right arithmetic)�{��
short sra(short ra) {
	return ((SRA << 11) | (ra << 8));
}
// �֐�ldl(load immediate value low)�{��
short ldl(short ra, short ival) {
	return ((LDL << 11) | (ra << 8) | (ival & 0x00ff));
}

// �֐�ldh(load immediate value high)�{��
short ldh(short ra, short ival) {
	return ((LDH << 11) | (ra << 8) | (ival & 0x00ff));
}

// �֐�cmp(compare)�{��
short cmp(short ra, short rb) {
	return ((CMP << 11) | (ra << 8) | (rb << 5));
}

// �֐�je(jump equal)�{��
short je(short addr) {
	return ((JE << 11) | (addr & 0x00ff));
}

// �֐�jmp(jump)�{��
short jmp(short addr) {
	return ((JMP << 11) | (addr & 0x00ff));
}

// �֐�ld(load memory)�{��
short ld(short ra, short addr) {
	return ((LD << 11) | (ra << 8) | (addr & 0x00ff));
}

// �֐�st(store memory)�{��
short st(short ra, short addr) {
	return ((ST << 11) | (ra << 8) | (addr & 0x00ff));
}

// �֐�hlt(halt)�{��
short hlt(void) {
	return (HLT << 11);
}

// �֐�op_code�{��
short op_code(short ir) {
	return  (ir >> 11);
}

// �֐�op_regA�{��
short op_regA(short ir) {
	return ((ir >> 8) & 0x0007);
}

// �֐�op_regB�{��
short op_regB(short ir) {
	return ((ir >> 5) & 0x0007);
}

// �֐�op_data�{��
short op_data(short ir) {
	return (ir & 0x00ff);
}

// �֐�op_addr�{��
short op_addr(short ir) {
	return (ir & 0x00ff);
}
