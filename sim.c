#include "sim_core.h"


int main(int argc, char* argv[]) {
	simulator sim;
	const char* file_modes[] = { "r","r","r","w","w","w","w","w","w","w","w" };
	initialize_simulator(&sim, argv, argc, file_modes);
	run_simulation(&sim);
	return 0;
}





//~~~~~~~~~~~~~~~~~~~FILES CODE~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


//opens file found in file_path, where file_mode is the file mode ("r" ,"w", ..), 
//and assigns the file to the file pointer *file
void open_file(const char* file_path, const char* file_mode, FILE** file) {

	FILE* local_file = NULL;
	if ((local_file = fopen(file_path, file_mode)) == NULL) {
		printf("Error: couldn't open file \"%s\"", file_path);
		exit(1);
	}
	*file = local_file;
	printf("%s opened successfully\n\n", file_path);
}



/* file_to_read: pointer to text file we want to read
	array[]: array of integers to read file content to
	max_length: maximum number of lines in text file

	function reads file line by line, converts each line into an int
	and writes it to array[] */
void read_file_to_array(FILE *file_to_read, int array[], int max_length) {

	if (file_to_read == NULL) {
		printf("ERROR: file pointer is null!");
		exit(1);
	}

	int i = 0;
	char word[MAX_WORD_SIZE];
	while ((i < max_length) && !feof(file_to_read)) {
		if (fgets(word, MAX_WORD_SIZE, file_to_read) == NULL) {
			break;
		}
		array[i] = strtol(word, NULL, 16);
		i++;
	}

	if (i < max_length) {
		while (i < max_length){
			array[i] = 0x00000000;
			i++;
		}
	
	}
	printf("file array created successfully\n\n");
}

// file: a pointer to the text file we want to write to
//array: array of integers we want to write from
//array_size: size of array
// the function writes the contents of array[] into the text file, each element in one line in 8 digit hexidecimal format
void write_to_file_from_array(FILE* file, int array[], int array_size) {

	if (file == NULL) {
		printf("ERROR: file pointer is null!");
		exit(1);
	}

	int i = 0;
	while (i < array_size) {
		fprintf(file, "%.8X\n", array[i]);
		i++;
		// writes the number inside array into file in hexidecimal form
	}
}

/*
the function writes to the trace file the current values of PC, the instruction and the registers
in hexidecimal in the form:
PC INST R0 R1 R2 R3 R4 R5 R6 R7 R8 R9 R10 R11 R12 R13 R14 R15*/
void write_to_trace_file(simulator* sim) {

	long PC = sim->PC;
	FILE* trace = sim->files[TRACE];
	if (trace == NULL) {
		printf("ERROR: file pointer is null!");
		exit(1);
	}

	fprintf(trace, "%.8X %.8X", PC, sim->instruction);
	int reg_num = 0;
	while (reg_num < NUM_OF_REGS) {
		fprintf(trace, " %.8X", sim->Registers[reg_num]);
		reg_num++;
	}
	fprintf(trace, "%c", '\n');
}

/*
the fuction prints a line in hwregtrace in the form : CYCLE READ/WRITE NAME DATA*/
void write_to_hwregtrace_file(simulator* sim) {
	FILE *hwregtrace = sim->files[HWREGTRACE];
	long cycle = sim->IORegisters[clks];
	IORegister name = sim->Registers[sim->rs] + sim->Registers[sim->rt];
	unsigned data = sim->IORegisters[name];
	if (hwregtrace == NULL) {
		printf("ERROR: file pointer is null!");
		exit(1);
	}

	const char* action = (sim->op == in) ? "READ" : "WRITE"; //if instruction is in READ, if out WRITE

	fprintf(hwregtrace, "%u %s %s %.8X\n", cycle, action, IORegister_names[name], data);
}

/*
the function writes the cycle at which data was changed and next to it the number inside the IORegister
seperated by a space, in 8 digit hexidecimal format	*/
void write_line_of_cycle_data_to_file(simulator* sim) {
	unsigned cycle = sim->IORegisters[clks];
	IORegister IOR_name = sim->Registers[sim->rt] + sim->Registers[sim->rs];
	unsigned data = sim->IORegisters[IOR_name];

	switch (IOR_name) {
	case leds:
		fprintf(sim->files[LEDS], "%u %.8X\n", cycle, data);
		break;
	case display:
		fprintf(sim->files[DISPLAY], "%u %.8X\n", cycle, data);
		break;
	}
}




//~~~~~~~~~~~~~~~~~~~~~~INSTRUCTIONS CODE~~~~~~~~~~~~~~~~~~~~~~~~~~~~



//the function fetches next instruction from memory (sim->memory[sim->PC]) and puts it in sim->instruction
int instruction_fetch(simulator* sim) {
	int PC = sim->PC;
	// return 1 if PC is out of range
	if (PC >= MEM_SIZE || PC < 0)
		return 1;

	// else fetch instruction
	sim->instruction = sim->memory[PC];
	return 0;		//success
}

//partitions instruction into: op, rd, rs, rt ,imm and places each in the appropriate field inside the sim struct
void instruction_partition(simulator* sim) {
	int inst = sim->instruction;
	/* bitwise shift insrtuction by 24 then ANDing with 0xff to get the upper 8 bits (opcode) */
	sim->op = (sim->instruction >> 24) & 0x000000ff;

	/* bitwise shift instruction by 20 then ANDing with 0xf to extract the rd register only */
	sim->rd = (sim->instruction >> 20) & 0x0000000f;

	/* bitwise shift instruction by 16 then ANDing with 0xf to extract the rs register only */
	sim->rs = (sim->instruction >> 16) & 0x0000000f;

	/* bitwise shift instruction by 12 then ANDing with 0xf to extract the rt register only */
	sim->rt = (sim->instruction >> 12) & 0x0000000f;

	/* ANDing instruction with 0xfff to get the first 12 bits (immediate)*/
	sim->Registers[$imm] = sim->instruction & 0x00000fff;
	//shift *imm 11 times to the left to get its sign bit
	int sign = sim->Registers[$imm] >> 11;
	if (sign == 1) {	// immediate is negative
		sim->Registers[$imm] = sim->Registers[$imm] | 0xfffff000;  // extend by ones
	}	// else keep it as it is (zeros to the left)
}


//executes the instruction inside sim->instruction
void execute_instruction(simulator* sim) {

	int op = sim->op;
	int rd = sim->rd;
	int rs = sim->rs;
	int rt = sim->rt;
	int *R = sim->Registers;

	int rd_not_zero_or_imm = (rd != $zero && rd != $imm);

	switch (op) {

	case add:		//addition
		if (rd_not_zero_or_imm)
			R[rd] = R[rs] + R[rt];
		break;

	case sub:		//subtraction
		if (rd_not_zero_or_imm)
			R[rd] = R[rs] - R[rt];
		break;

	case and:		//bitwise and
		if (rd_not_zero_or_imm)
			R[rd] = R[rs] & R[rt];
		break;

	case or :		//bitwise or
		if (rd_not_zero_or_imm)
			R[rd] = R[rs] | R[rt];
		break;

	case sll:		//shift left logical
		if (rd_not_zero_or_imm)
			R[rd] = (R[rs] << R[rt]);
		break;

	case sra:		//shift right arithmetic
		if (rd_not_zero_or_imm)
			R[rd] = (R[rs] >> R[rt]);
		break;

	case srl:		//shift right logical
		if (rd_not_zero_or_imm) {
			R[rd] = ((unsigned)R[rs] >> R[rt]);
		}
		break;

	case beq:		//branch if equal
		printf("\nR[rs] = %d, R[rt]= %d\n", R[rs], R[rt]);
		if (R[rs] == R[rt]) {
			sim->PC = (R[rd] & 0xFFF);
			sim->PC_updated = 1;
		}
		break;

	case bne:		//branch if not equal
		if (R[rs] != R[rt]) {
			sim->PC = (R[rd] & 0xFFF);
			sim->PC_updated = 1;
		}
		break;

	case blt:		//branch if less than
		if (R[rs] < R[rt]) {
			sim->PC = (R[rd] & 0xFFF);
			sim->PC_updated = 1;
		}
		break;


	case bgt:	//branch if greater than
		if (R[rs] > R[rt]) {
			sim->PC = (R[rd] & 0xFFF);
			sim->PC_updated = 1;
		}
		break;

	case ble:	//branch if less or equal
		if (R[rs] <= R[rt]){
			sim->PC = (R[rd] & 0xFFF);
			sim->PC_updated = 1;
		}
		break;

	case bge:	//branch if greater or equal
		if (R[rs] >= R[rt]){
			sim->PC = (R[rd] & 0xFFF);
			sim->PC_updated = 1;
		}
		break;

	case jal:	//jump and link
		R[$ra] = sim->PC + 1;
		sim->PC = (R[rd] & 0xFFF);
		sim->PC_updated = 1;
		break;

	case lw:	//load word
		if (rd_not_zero_or_imm && R[rs] + R[rt] >= 0 && R[rs] + R[rt] < MEM_SIZE)
			R[rd] = sim->memory[R[rs] + R[rt]];
		break;

	case sw:	//store word 
		if (R[rs] + R[rt] >= 0 && R[rs] + R[rt] < MEM_SIZE)
			sim->memory[R[rs] + R[rt]] = R[rd];
		break;

	case reti:
		sim->PC = sim->IORegisters[irqreturn];  //*PC = IORegisters[7]
		sim->PC_updated = 1;
		break;

	case in: case out:
		handle_IO(sim);
		break;
	}
}




//~~~~~~~~~~~~~~~~IO CODE~~~~~~~~~~~~~~~~~~~~~



void write_to_disk_sector(simulator* sim) {
	int buffer = sim->IORegisters[diskbuffer];
	int disk_address = sim->IORegisters[disksector] * SECTOR_SIZE;
	for (int i = 0; i < SECTOR_SIZE; i++) {
		if (buffer >= 0 && buffer < MEM_SIZE && disk_address >= 0 && disk_address < DISK_SIZE) {
			//check if buffer and disk address in range
			//if in range:
			sim->disk[disk_address] = sim->memory[buffer];
			buffer++, disk_address++;
		}
	}
}

void read_from_disk_sector(simulator* sim) {
	int buffer_address = sim->IORegisters[diskbuffer];
	int disk_address = sim->IORegisters[disksector] * SECTOR_SIZE;
	for (int i = 0; i < SECTOR_SIZE; i++) {
		if (buffer_address >= 0 && buffer_address < MEM_SIZE && disk_address >= 0 && disk_address < DISK_SIZE) {
			//check if buffer and disk address in range
			//if in range:
			sim->memory[buffer_address] = sim->disk[disk_address];
			buffer_address++, disk_address++;
		}
	}
}

//gets the next cycle at which irq2 occurs from the irq2in file
void get_next_irq2(simulator *sim) {
	char buffer[50];
	if (fgets(buffer, 50, sim->files[IRQ2IN]) != 0){
		sim->next_irq2 = (unsigned)strtol(buffer, NULL, 10);
		if (sim->next_irq2 <= sim->IORegisters[clks])
			get_next_irq2(sim);
	}
}

void check_disk(simulator* sim) {
	if (sim->IORegisters[diskcmd] && sim->IORegisters[clks] - sim->DMA_starting_cycle == DMA_TIME) {
		sim->IORegisters[diskstatus] = 0;
		sim->IORegisters[diskcmd] = 0;
		sim->IORegisters[irq1status] = 1;
	}
}

//checks if irq==1 and updates PC accordingly, checks each of irq0, irq1, irq2 and updates their statuses accordingly
void check_and_handle_irq(simulator* sim) {

	unsigned *IORegisters = sim->IORegisters;
	sim->irq = (IORegisters[irq0enable] && IORegisters[irq0status]) ||
		(IORegisters[irq1enable] && IORegisters[irq1status]) ||
		(IORegisters[irq2enable] && IORegisters[irq2status]);
	if (sim->irq == 1 && sim->currently_handling_interruption != 1) {
		sim->IORegisters[irqreturn] = sim->PC;
		sim->PC = IORegisters[irqhandler];
		sim->PC_updated = 1;
		sim->currently_handling_interruption = 1;
	}
	if (IORegisters[irq0enable] && IORegisters[timerenable]){ // if irq0 and timer are enabled
		if (IORegisters[timercurrent] == IORegisters[timermax]) { //check for irq0
			IORegisters[irq0status] = 1;
			IORegisters[timercurrent] = 0;
		}
	}
		
	if (IORegisters[diskstatus] && IORegisters[irq1enable] && sim->IORegisters[clks] - sim->DMA_starting_cycle == DMA_TIME){
		IORegisters[irq1status] = 1;
	}
	if (IORegisters[irq2enable]){
		if (IORegisters[clks] == sim->next_irq2) { //check for irq2
			IORegisters[irq2status] = 1;
			get_next_irq2(sim);
		}
	}
}

//handles in out instructions, and writes to the IO files accordingly
void handle_IO(simulator* sim) {
	int op = sim->op, rd = sim->rd, rs = sim->rs, rt = sim->rt;
	IORegister IOR_num = sim->Registers[rt] + sim->Registers[rs];
	switch (op) {
	case out:
		switch (IOR_num) {

		case irq0enable: case irq1enable: case irq2enable: case timerenable:
			sim->IORegisters[IOR_num] = sim->Registers[rd] & 1;	// 1 bit
			if (IOR_num == timerenable && sim->Registers[rd] == 1)		//timer was enabled
				sim->timer_starting_cycle = sim->IORegisters[clks];
			break;
		case irq0status: case irq1status: case irq2status:
			if (sim->Registers[rd] == 0) {
				sim->IORegisters[IOR_num] = 0;
				sim->currently_handling_interruption = 0;
			}

		case diskcmd:
			sim->IORegisters[IOR_num] = sim->Registers[rd] & 0x3;	//2 bits
			handle_diskcmd(sim);
			break;

		case irqhandler: case irqreturn: case diskbuffer:
			sim->IORegisters[IOR_num] = sim->Registers[rd] & 0xFFF;	//12 bits
			break;

		case disksector:
			sim->IORegisters[IOR_num] = sim->Registers[rd] & 0x7F;	//7 bits
			break;

		case leds: case display:
			sim->IORegisters[IOR_num] = sim->Registers[rd];	//32 bits
			write_line_of_cycle_data_to_file(sim);
			break;
		case timermax:
			sim->IORegisters[IOR_num] = sim->Registers[rd];	//32 bits
		}
		break;
	case in:
		switch (IOR_num) {
		case leds: case display:
			sim->Registers[rd] = sim->IORegisters[IOR_num];
			break;
		case diskcmd:
			handle_diskcmd(sim);
			break;
		default:
			sim->Registers[rd] = sim->IORegisters[IOR_num];
			break;
		}

	}
	write_to_hwregtrace_file(sim);
}

// handles writing and reading from the disk to the buffer
void handle_diskcmd(simulator* sim) {
	if (sim->IORegisters[diskcmd] == 1 && sim->IORegisters[diskstatus] == 0) {
		sim->DMA_starting_cycle = sim->IORegisters[clks];
		read_from_disk_sector(sim);
	} 
	if (sim->IORegisters[diskcmd] == 2 && sim->IORegisters[diskstatus] == 0) {
		sim->DMA_starting_cycle = sim->IORegisters[clks];
		write_to_disk_sector(sim);
	}
}

void handle_timer(simulator* sim) {
	int instructions_performed_since_timer_enabled = sim->IORegisters[clks] - sim->timer_starting_cycle;
	if (instructions_performed_since_timer_enabled > 0) {
		if (instructions_performed_since_timer_enabled % CYCLES_PER_SECOND == 0) {
			//a second has passed since timercurrent was last updated
			sim->IORegisters[timercurrent]++;	//update timercurrent}

		}
	}
}




//~~~~~~~~~~~~~~~~~~~SIMULATION CODE~~~~~~~~~~~~~~~~~~~~


void update_PC(simulator* sim) {
	if (!sim->PC_updated) {
		sim->PC++;
	}
	else sim->PC_updated = 0;
}

// input arguments:
// sim: a pointer to a simulator struct
// argv[]: the command line arguments (sim.exe memin.txt diskin.txt irq2in.txt memout.txt regout.txt trace.txt
// hwregtrace.txt cycles.txt leds.txt display.txt diskout.txt)
// argc: command line argument count
// file modes[]: an array containing the mode we want to open each file in ("r" or "w")
//the functions opens the different files and intitializes the simulator structs and the variables inside it
void initialize_simulator(simulator* sim, const char *argv[], int argc, const char* file_modes[NUM_OF_FILES]) {
	int i;
	sim->memory = (long*)malloc((MEM_SIZE + 1) * sizeof(long));
	if (sim->memory == NULL) { printf("ERROR: memory allocation failed!\n"); exit(1); }
	sim->disk = (long*)malloc((DISK_SIZE + 1) * sizeof(long));
	if (sim->disk == NULL) { printf("ERROR: memory allocation failed!\n"); exit(1); }
	for (i = 0; i < MEM_SIZE; i++) {
		sim->memory[i] = 0;
	}
	for (i = 0; i < DISK_SIZE; i++) {
		sim->disk[i] = 0;
	}
	for (i = 0; i < NUM_OF_REGS; i++) {
		sim->Registers[i] = 0;
	}
	sim->Registers[$sp] = MEM_SIZE - 1;
	sim->Registers[$gp] = MEM_SIZE / 2;
	for (i = 0; i < NUM_OF_IOREGS; i++) {
		sim->IORegisters[i] = 0;
	}
	sim->PC = 0;
	sim->PC_updated = 0;
	sim->irq = 0;
	for (int i = 0; i < argc - 1; i++) {
		open_file(argv[i + 1], file_modes[i], &sim->files[i]);
	}
	read_file_to_array(sim->files[DISKIN], sim->disk, DISK_SIZE);
	read_file_to_array(sim->files[MEMIN], sim->memory, MEM_SIZE);
	get_next_irq2(sim);
	printf("simulator initialized\n\n");
}

//simulates the simulator from start to finish (simulates each cycle)
void run_simulation(simulator* sim) {

	while (1) {
		if (sim->PC < 0 || sim->PC >= MEM_SIZE) break;
		printf("\n CYCLE %d\n", sim->IORegisters[clks]);
		check_and_handle_irq(sim);
		check_disk(sim);
		if (instruction_fetch(sim) != 0) { break; }; //if can't fetch instruction because of invalid PC =>break
		instruction_partition(sim);
		write_to_trace_file(sim);
		if (sim->op == halt) {
			sim->IORegisters[clks] = (sim->IORegisters[clks] == MAX_32_BIT_NUM) ? 0 : sim->IORegisters[clks] + 1;
			break;
		}
		execute_instruction(sim);
		if (sim->IORegisters[timerenable]) handle_timer(sim);
		sim->IORegisters[clks] = (sim->IORegisters[clks] == MAX_32_BIT_NUM) ? 0 : sim->IORegisters[clks] + 1;
		update_PC(sim);
	}
	end_simulation(sim);
	printf("SIMULATION SUCCESSFUL!!");
}

//ends simulation, writes the memory array and the disk array to memout file and diskout file accordingly
// frees allocated memory and closes all open files
void end_simulation(simulator* sim) {

	write_to_file_from_array(sim->files[MEMOUT], sim->memory, MEM_SIZE);
	printf("writing to memout.txt successful\n\n");
	write_to_file_from_array(sim->files[DISKOUT], sim->disk, DISK_SIZE);
	printf("writing to diskout.txt successful\n\n");
	write_to_file_from_array(sim->files[REGOUT], sim->Registers + 2, 14);
	fprintf(sim->files[CYCLES], "%u", sim->IORegisters[clks]);
	printf("writing to regout.txt successful\n\n");
	//we want to write only R2-R15 therefore the array starts from R2 until R15 (Registers +2). 
	//and the size is 14 (there are 14 Registers from R2 to R15)
	free(sim->memory);
	free(sim->disk);
	for (int i = 0; i < NUM_OF_FILES; i++) {
		if (fclose(sim->files[i]) != 0) {
			printf("ERROR: failure to closr files!");
			exit(1);
		}
	}
}