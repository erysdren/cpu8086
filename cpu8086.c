
//
//
// references
//
// https://yassinebridi.github.io/asm-docs/8086_instruction_set.html
// http://www.mlsite.net/8086/
// http://www.ablmcc.edu.hk/~scy/CIT/8086_bios_and_dos_interrupts.htm
// https://www.ic.unicamp.br/~celio/mc404/opcodes.html
// http://spike.scu.edu.au/~barry/interrupts.html
// https://en.wikipedia.org/wiki/Intel_8086
//
//

//
//
// headers
//
//

// std
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>

//
//
// types
//
//

// opcode
typedef uint8_t opcode_t;
typedef uint8_t interrupt_t;
typedef uint8_t *(*cpu_opcode_t)(uint8_t *);
typedef void (*cpu_interupt_t)(void);

//
//
// macros
//
//

// limits
#define MEMORY 0x10000
#define PROGRAM 0x0100
#define STACK 0x2000
#define NUM_OPS 256
#define NUM_REGS 28
#define NUM_INTERRUPTS 256
#define NUM_VECTORS 16
#define VECTOR_BASE 0x20

// utils
#define UNUSED(x) ((void)(x))

// get 8-bit register
#define REG8(i) (cpu_regs[i])

// get 16-bit register
#define REG16(i) (*(uint16_t *)&cpu_regs[i])

// register macros
#define REG_AX REG16(0)
#define REG_AH REG8(0)
#define REG_AL REG8(1)
#define REG_BX REG16(2)
#define REG_BH REG8(2)
#define REG_BL REG8(3)
#define REG_CX REG16(4)
#define REG_CH REG8(4)
#define REG_CL REG8(5)
#define REG_DX REG16(6)
#define REG_DH REG8(6)
#define REG_DL REG8(7)
#define REG_SI REG16(8)
#define REG_DI REG16(10)
#define REG_BP REG16(12)
#define REG_SP REG16(14)
#define REG_IP REG16(16)
#define REG_CS REG16(18)
#define REG_DS REG16(20)
#define REG_ES REG16(22)
#define REG_SS REG16(24)

// helpers
#define OP(name) uint8_t *op_##name(uint8_t *p)
#define INT(name) void int_##name(void)

//
//
// forward defs
//
//

int error(const char *s, ...);
void message(const char *s, ...);
void print(char *string, char terminator, FILE *stream);
void *load(char *filename, size_t *len);

//
//
// globals
//
//

// memory buffer
uint8_t cpu_memory[MEMORY];
uint8_t *cpu_program = &cpu_memory[PROGRAM];
uint8_t *cpu_stack = &cpu_memory[MEMORY - 1];
uint8_t cpu_regs[NUM_REGS];
size_t cpu_program_len;
bool cpu_quit = false;
interrupt_t cpu_int_vec;
char cpu_program_name[256];

// tables
cpu_interupt_t cpu_interrupts[NUM_VECTORS][NUM_INTERRUPTS];
cpu_opcode_t cpu_opcodes[NUM_OPS];

// push, pop, mov helpers
static inline void cpu_push(uint16_t x) { REG_SP += 2; *(cpu_stack - REG_SP) = x; }
static inline uint16_t cpu_pop(void) { uint16_t ret = *(cpu_stack - REG_SP); REG_SP -= 2; return ret; }
static inline void cpu_mov1(uint8_t op)
{
	switch (op)
	{
		case 0xE5: REG_BP = REG_SP; break;
		case 0xEC: REG_SP = REG_BP; break;
		default: break;
	}
}
static inline void cpu_mov2(uint8_t op1, uint8_t op2)
{
	switch (op1)
	{
		case 0x56: REG_DX = op2; break;
		default: break;
	}
}

// interrupt functions
INT(err) { error("invalid interrupt 0x%02x with ah: 0x%02x\n", cpu_int_vec, REG_AH); }
INT(20h) { message("%s exited", cpu_program_name); cpu_quit = true; }
INT(21h_000) { message("%s exited", cpu_program_name); cpu_quit = true; }
INT(21h_001) { REG_AL = (char)getc(stdin); }
INT(21h_002) { REG_AL = (char)getc(stdout); putc((char)REG_DL, stdout); }
INT(21h_005) { putc((char)REG_DL, stdout); }
INT(21h_006) { putc((char)REG_DL, stdout); REG_AL = (char)getc(stdout); }
INT(21h_007) { REG_AL = (char)getc(stdin); }
INT(21h_008) { REG_AL = (char)getc(stdin); }
INT(21h_009) { print((char *)&cpu_memory[REG_DX], '$', stdout); }
INT(21h_076) { message("%s exited with return code %u", cpu_program_name, REG_AL); cpu_quit = true; }

// opcode functions
OP(err) /* err     */ { error("invalid opcode 0x%02x, 0x%02x at offset 0x%02x", *(p - 1), *p, (p - 1) - cpu_program); return p; }
OP(080) /* push ax */ { cpu_push(REG_AX); printf("sent ax %u\n", REG_AX); return p; }
OP(083) /* push bx */ { cpu_push(REG_BX); printf("sent bx %u\n", REG_BX); return p; }
OP(085) /* push bp */ { cpu_push(REG_BP); printf("sent bp %u\n", REG_BP); return p; }
OP(088) /* pop ax  */ { REG_AX = cpu_pop(); printf("ax recieved %u\n", REG_AX); return p; }
OP(090) /* pop dx  */ { REG_DX = cpu_pop(); printf("dx recieved %u\n", REG_DX); return p; }
OP(091) /* pop bx  */ { REG_BX = cpu_pop(); printf("bx recieved %u\n", REG_BX); return p; }
OP(093) /* pop bp  */ { REG_BP = cpu_pop(); printf("bp recieved %u\n", REG_BP); return p; }
OP(104) /* push    */ { cpu_push(*(uint16_t *)p); printf("sent %u\n", *(uint16_t *)p); return p + 2; }
OP(137) /* mov     */ { cpu_mov1(*p); return p + 1; }
OP(139) /* mov     */ { cpu_mov2(*p, *(p + 1)); return p + 2; }
OP(144) /* nop     */ { return p; }
OP(180) /* mov ah  */ { REG_AH = *p; return p + 1; }
OP(186) /* mov dx  */ { REG_DX = *(uint16_t *)p; return p + 2; }
OP(195) /* ret     */ { UNUSED(p); uint16_t ip = cpu_pop(); return cpu_program + ip; }
OP(205) /* int     */ { cpu_int_vec = *p - VECTOR_BASE; cpu_interrupts[cpu_int_vec][REG_AH](); return p + 1; }
OP(232) /* call    */ { cpu_push((p + 2) - cpu_program); return p + *(int8_t *)p + 2; }
OP(235) /* jmp     */ { return p + *(int8_t *)p + 1; }

//
//
// functions
//
//

//
// cpu_init
//

void cpu_init(void)
{
	// init interrupt table
	for (int v = 0; v < NUM_VECTORS; v++)
	{
		for (int i = 0; i < NUM_INTERRUPTS; i++)
		{
			if (v == 0) cpu_interrupts[v][i] = &int_20h;
			else if (v == 1 && i == 0) cpu_interrupts[v][i] = &int_21h_000;
			else if (v == 1 && i == 9) cpu_interrupts[v][i] = &int_21h_009;
			else if (v == 1 && i == 76) cpu_interrupts[v][i] = &int_21h_076;
			else cpu_interrupts[v][i] = &int_err;
		}
	}

	// init opcode table
	for (int i = 0; i < NUM_OPS; i++)
	{
		if (i == 80) cpu_opcodes[i] = &op_080;
		else if (i == 83) cpu_opcodes[i] = &op_083;
		else if (i == 85) cpu_opcodes[i] = &op_085;
		else if (i == 88) cpu_opcodes[i] = &op_088;
		else if (i == 90) cpu_opcodes[i] = &op_090;
		else if (i == 91) cpu_opcodes[i] = &op_091;
		else if (i == 93) cpu_opcodes[i] = &op_093;
		else if (i == 104) cpu_opcodes[i] = &op_104;
		else if (i == 137) cpu_opcodes[i] = &op_137;
		else if (i == 139) cpu_opcodes[i] = &op_139;
		else if (i == 144) cpu_opcodes[i] = &op_144;
		else if (i == 180) cpu_opcodes[i] = &op_180;
		else if (i == 186) cpu_opcodes[i] = &op_186;
		else if (i == 195) cpu_opcodes[i] = &op_195;
		else if (i == 205) cpu_opcodes[i] = &op_205;
		else if (i == 232) cpu_opcodes[i] = &op_232;
		else if (i == 235) cpu_opcodes[i] = &op_235;
		else cpu_opcodes[i] = &op_err;
	}

	// set stack pointer
	REG_SP = 0;
}

//
// cpu_run
//

void cpu_run(void *program, size_t len)
{
	// variables
	size_t i;
	opcode_t op;
	uint8_t *ptr;

	// check function size size
	if (len > MEMORY - PROGRAM) error("program size %lu too big", len);

	// copy program into memory
	memcpy(cpu_program, program, len);
	cpu_program_len = len;

	// init and run parser
	i = 0;
	ptr = cpu_program;
	while (i < MEMORY && i < cpu_program_len)
	{
		if (cpu_quit == true) break;
		op = *ptr;
		ptr = cpu_opcodes[op](++ptr);
		i++;
	}
}

//
// load
//

void *load(char *filename, size_t *len)
{
	FILE *file;
	size_t filelen;
	void *filebuffer;

	file = fopen(filename, "rb");
	if (!file) return NULL;
	fseek(file, 0, SEEK_END);
	filelen = ftell(file);
	fseek(file, 0, SEEK_SET);
	filebuffer = malloc(filelen);
	if (!filebuffer) return NULL;
	fread(filebuffer, filelen, 1, file);
	fclose(file);
	if (len) *len = filelen;
	return filebuffer;
}

//
// print
//

void print(char *string, char terminator, FILE *stream)
{
	int runaway = 0;
	char *pointer = string;
	while (*pointer != terminator)
	{
		fwrite(pointer, sizeof(char), 1, stream);
		pointer++;
		if (++runaway > MEMORY - PROGRAM) break;
	}
	fflush(stream);
}

//
// message
//

void message(const char *s, ...)
{
	va_list ap;

	va_start(ap, s);
	fprintf(stdout, "> cpu8086: ");
	vfprintf(stdout, s, ap);
	fprintf(stdout, "\n");
	va_end(ap);
}

//
// error
//

int error(const char *s, ...)
{
	va_list ap;

	va_start(ap, s);
	fprintf(stderr, "> cpu8086 error: ");
	vfprintf(stderr, s, ap);
	fprintf(stderr, "\n");
	va_end(ap);

	exit(EXIT_FAILURE);

	return EXIT_FAILURE;
}

//
// main
//

int main(int argc, char *argv[])
{
	// variables
	void *program;
	size_t len;

	// check validitiy
	if (argc < 2) return error("must provide input file");

	// copy name
	strcpy(cpu_program_name, argv[1]);

	// load program
	message("loadng %s", cpu_program_name);
	program = load(cpu_program_name, &len);
	if (program == NULL) return error("failed to load %s", cpu_program_name);

	// init cpu
	message("placing %s at address 0x%04x", cpu_program_name, PROGRAM);
	cpu_init();

	// run cpu
	message("executing %s", cpu_program_name);
	cpu_run(program, len);

	// free program
	free(program);

	// exit
	return EXIT_SUCCESS;
}
