#pragma once

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define MEM_SIZE 4096
#define MAX_WORD_SIZE 50
#define LINE_LENGTH 8
#define NUM_OF_REGS 16
#define NUM_OF_IOREGS 18
#define NUM_OF_SECTORS 128
#define SECTOR_SIZE 128
#define DISK_SIZE 16384		//128*128
#define NUM_OF_FILES 11
#define CYCLES_PER_SECOND 256
#define MAX_32_BIT_NUM 0xFFFFFFFF
#define DMA_TIME 1024

typedef enum Register {
	$zero, $imm, $v0, $a0, $a1, $t0, $t1, $t2, $t3, $s0, $s1, $s2, $gp, $sp, $fp, $ra
}Register;

typedef enum IORegister {
	irq0enable, irq1enable, irq2enable, irq0status, irq1status, irq2status, irqhandler,
	irqreturn, clks, leds, display, timerenable, timercurrent, timermax, diskcmd, disksector,
	diskbuffer, diskstatus
}IORegister;

enum INSTRUCTION_NAMES {
	add, sub, and, or , sll, sra, srl, beq, bne, blt, bgt, ble, bge, jal, lw, sw, reti, in, out, halt
};

enum simulation_files {
	MEMIN, DISKIN, IRQ2IN, MEMOUT, REGOUT, TRACE,
	HWREGTRACE, CYCLES, LEDS, DISPLAY, DISKOUT
};


//struct containing all the variables the simulator needs
//that way it's easier to pass the different parameters to functions
//by just passing a pointer to the struct
typedef struct simulator {
	int *memory;  //pointer to the array of memory image
	int *disk;//pointer to the array of disk image
	int instruction; //current instruction (represented as number, that contains op,rd,rs,rt,imm)
	int PC; //current PC
	int op; //current opcode
	int rd;	//current rd
	int rt;	//current rt
	int rs;	//current rs
	int Registers[NUM_OF_REGS]; //the registers array,
					//where each element is the current value inside the register
	unsigned IORegisters[NUM_OF_IOREGS]; //the IORegisters array,
							//where each element is the current value inside the register
	int irq;
	int next_irq2;
	unsigned DMA_starting_cycle;	//the cycle at which the writing\reading to\from disk started
	unsigned timer_starting_cycle;
	int currently_handling_interruption;	//=1 if in the currnet cycle the simulator is handling an interruption
	int PC_updated;	//=1 if PC was updated during the current cycle, =0 otherwise


	FILE* files[NUM_OF_FILES];	//array to contain the file pointers:
	//memin, diskin, irq2in, memout, regout, trace,
	//hwregtrace, cycles, leds, display, diskout
}simulator;


const char* register_names[] = { "$zero", "$imm", "$v0", "$a0", "$a1", "$t0", "$t1", "$t2",
"$t3", "$s0", "$s1", "$s2", "$gp", "$sp", "$fp", "$ra" };

const char* IORegister_names[] = { "irq0enable", "irq1enable", "irq2enable", "irq0status", "irq1status",
		"irq2status", "irqhandler", "irqreturn", "clks", "leds", "display", "timerenable", "timercurrent",
		"timermax", "diskcmd", "disksector","diskbuffer", "diskstatus" };

const char* file_names[] = { "memin", "diskin", "irq2in", "memout", "regout", "trace",
	"hwregtrace", "cycles", "leds", "display", "diskout" };


const char* instruction_names[] = {
	"add", "sub", "and", "or" , "sll", "sra", "srl", "beq", "bne",
	"blt", "bgt", "ble", "bge", "jal", "lw", "sw", "reti", "in", "out", "halt"
};

void write_to_file_from_array(FILE* file, int array[], int array_size);
void read_file_to_array(FILE *file_to_read, int array[], int max_length);
void write_to_trace_file(simulator* sim);
void write_to_hwregtrace_file(simulator* sim);
void write_line_of_cycle_data_to_file(simulator* sim);
int instruction_fetch(simulator* sim);
void instruction_partition(simulator* sim);
void execute_instruction(simulator* sim);
void write_to_disk_sector(simulator* sim);
void get_next_irq2(simulator *sim);
void read_from_disk_sector(simulator* sim);
void check_and_handle_irq(simulator* sim);
void handle_IO(simulator* sim);
void handle_diskcmd(simulator *sim);
void initialize_simulator(simulator* sim, const char *argv[], int argc, const char* file_modes[NUM_OF_FILES]);
void run_simulation(simulator* sim);
void end_simulation(simulator* sim);
void handle_timer(simulator* sim);
void update_PC(simulator* sim);
void check_disk(simulator* sim);