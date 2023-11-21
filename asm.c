#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define LABEL_LENGTH 50//label name length
#define REG 7//for word struct
#define IMM 50//for word struct
#define OPCODE_SIZE 6//for word struct
#define MAX_SIZE_INSTRUCTION 500
#define MAX_SIZE_LB_ARRAY 300//max number of labels
#define WORD_MAX 4096    // max number of instructions
#define MEM_ADDR 300// memory array max number of chars
#define MEM_DATA 300//mem data array max number of chars

struct label {
	char lb_name[LABEL_LENGTH];
	int lb_row;// the row in which the first instruction of this label starts


};
struct word {
	char opcode[OPCODE_SIZE];
	char rd[REG];
	char rs[REG];
	char rt[REG];
	char imm[IMM];
	long imm_num;
	long mem;// !=NULL only in case the word is data of .word

};

int is_instruction(char word[]) {// return 1 if instruction and zero else
	char* check = 0;
	char* check1 = 0;

	if ((check = strchr(word, ',')) != NULL) {
		if ((check1 = strchr(word, '#')) != NULL) {
			if (check < check1) {
				return 1;
			}
			return 0;
		}
		return 1;


	}
	return 0;
}
int find_instruction(int row, FILE* file1) {
	char inst[MAX_SIZE_INSTRUCTION] = { 0 };
	fgets(inst, MAX_SIZE_INSTRUCTION, file1);
	while (is_instruction(inst) == 0) {// not an instruction
		while (fgetc(file1) != '\n') {

		}
		row++;
		fgets(inst, MAX_SIZE_INSTRUCTION, file1);
		continue;
		//new line

	}
	return row;
}
int get_labels(struct label arr[], FILE* file1) {// gives number of labels
	int count = 0;//counts number of labels in file
	char* colon = 0;//first occurence of : (label)
	char* check = 0; //check if label isn't comment

	int iter = 0; //iterates through array label element name
	int row = -1; //number of row in file
	int inst_row = -1;//number of instruction

	char* marker = 0;
	char word[MAX_SIZE_INSTRUCTION] = { 0 };//contains insturction string
	while (!feof(file1)) {

		if (fgets(word, MAX_SIZE_INSTRUCTION, file1) != NULL) {
			row++;//not an empty row
			if (is_instruction(word) == 1) {//return 1 if row is an instruction
				inst_row++;
				if ((colon = strchr(word, ':')) != NULL) {//יתכן שיש גם לייבל-מוודא שישנם נקודתיים
					if ((check = strchr(word, '#')) != NULL) {//יש נקודתיים- מוודא שיש סולמית על מנת לוודא שזו לא הערה
						if (colon < check) {
							//found real label+comment+instruction
							arr[count].lb_row = inst_row;// מעדכן את הכתובת של הלייבל

							marker = word;//marker equals to the initial address of the word, prepare for copying
							while (marker < colon) {
								arr[count].lb_name[iter] = *marker;
								marker++;
								iter++;

							}
							count++;
							marker = 0;
							iter = 0;


						}



						continue;// found instruction +comment

					}
					else {//label +instruction only

						arr[count].lb_row = inst_row;// מעדכן את הכתובת של הלייבל

						marker = word;//marker equals to the initial address of the word, prepare for copying label name
						while (marker < colon) {
							arr[count].lb_name[iter] = *marker;
							marker++;
							iter++;

						}
						count++;
						marker = 0;
						iter = 0;


					}

				}
				continue;//only instruction
			}
			
			else if ((colon = strchr(word, ':')) != NULL) {//not an instruction, checks if label
				if ((check = strchr(word, '#')) != NULL) {//check if is label because of :
					if (colon < check) {
						//found real label+comment+no instruction->go find instruction downwards
						inst_row++;
						row = find_instruction(row, file1);//moves file and row to the relevant place
						arr[count].lb_row = inst_row;

						marker = word;//marker equals to the initial address of the word, prepare for copying
						while (marker < colon) {
							arr[count].lb_name[iter] = *marker;
							marker++;
							iter++;

						}
						count++;
						marker = 0;
						iter = 0;


					}




					// found only comment or .word

				}
				else {//label for sure without instruction-> go find instruction downwards
					inst_row++;
					row = find_instruction(row, file1);
					arr[count].lb_row = inst_row;
					marker = word;//marker equals to the initial address of the word
					while (marker < colon) {
						arr[count].lb_name[iter] = *marker;
						marker++;
						iter++;
					}
					count++;
					marker = 0;
					iter = 0;
				}
			}
			//no instruction+no comment+no label-> empty line or .word{!!}
		}
		// no label in this row-> move next
		continue;
	}
	
	return count;
}
int is_word(char word[]) {//checks if is .word
	char* check1 = 0;
	char* check2 = 0;
	if ((check1 = strchr(word, '.')) != NULL) {
		if ((check2 = strchr(word, '#')) != NULL) {
			if (check1 < check2) {
				return 1;//is word
			}
			else return 0;//is comment
		}
		else return 1;//is word
	}
	else return 0; // is comment
}
void instruction_to_wd_arr(struct label lb_arr[], struct word wd_arr[], char* colon, int inst) {
	char lb_row_str[3] = { 0 };// contains the string address of the label
	char* pend;//for strtol
	int iter = 0;
	wd_arr[inst].mem = NULL;//runover word pointed to this address (nullify it)- empty field. this is an instruction only.
	while (*colon == '\t' || *colon == ' ') {
		colon++;
	}

	while ((*colon) != ' ' && (*colon) != '\t') {// copy opcode
		wd_arr[inst].opcode[iter] = *colon;
		colon++;
		iter++;
	}
	while (*colon != '$') {
		colon++;
	}
	//מצביע ל$
	iter = 0;

	while (*colon != ',' && *colon != '\t' && *colon != ' ') {//copy rd
		wd_arr[inst].rd[iter] = *colon;
		colon++;
		iter++;
	}

	while (*colon != '$') {
		colon++;
	}
	iter = 0;

	while (*colon != ',' && *colon != '\t' && (*colon) != ' ') {//copy rs
		wd_arr[inst].rs[iter] = *colon;
		colon++;
		iter++;
	}

	while (*colon != '$') {
		colon++;
	}
	iter = 0;
	while (*colon != ',' && *colon != '\t' && *colon!= ' ') {//copy rt
		wd_arr[inst].rt[iter] = *colon;
		colon++;
		iter++;
	}

	iter = 0;
	while (*colon == ' ' || *colon == '\t' || *colon == ',') {
		colon++;
	}

	if (*colon == '0' || *colon == '1' || *colon == '2' || *colon == '3' || *colon == '4' || *colon == '5' || *colon == '6' || *colon == '7' || *colon == '8' || *colon == '9' || *colon == '-') {//if is number in any base
		while ((*colon) != ' ' && (*colon) != '\t' && (*colon) != '\n' && *colon != '\0' && (*colon) != '#') {
			wd_arr[inst].imm[iter] = *colon;
			colon++;
			iter++;
		}
		wd_arr[inst].imm_num = strtol(wd_arr[inst].imm, &pend, 0);  

	}
	else {// LABEL
		iter = 0;
		while ((*colon) != ' ' && (*colon) != '\t' && (*colon) != '\n' && *colon != '\0' && (*colon) != '#') {
			wd_arr[inst].imm[iter] = *colon;
			colon++;
			iter++;
		}
		//SEARCH FOR LABEL IN lb_Arr
		for (iter = 0;iter <= (MAX_SIZE_LB_ARRAY);iter++) {// doing iterations on label array
			if (strcmp(wd_arr[inst].imm, lb_arr[iter].lb_name) == 0) {//found the label ->put in wd_arr[inst].imm
				wd_arr[inst].imm_num = lb_arr[iter].lb_row;
				break;
			}
			
			continue;//continue searching

		}
		//done replacing label with hexdecimal address strings

	}



	//no address;
}
void change_labels(struct label lb_arr[], FILE* file1, struct word wd_arr[], FILE* file2) {
	
	long to_add;
	char* pend;//for strtol
	char* colon = 0;//first occurence of : (label)
	char* check = 0; //check if label isn't comment
	int iter = 0; //iterates through array label element name
	
	int inst = -1; //number of instruction
	char* dot = 0;
	char addr[MEM_ADDR] = { 0 };
	char data[MEM_DATA] = { 0 };
	char word[MAX_SIZE_INSTRUCTION] = { 0 };//contains insturction string
	while (!feof(file1)) {
		if (fgets(word, MAX_SIZE_INSTRUCTION, file1) != NULL) {
			if (is_instruction(word) == 1) {
				inst++;
				if ((colon = strchr(word, ':')) != NULL) {
					if ((check = strchr(word, '#')) != NULL) {
						if (colon < check) {
							colon++;
						//instruction+label+comment
							while (*colon == '\t' || *colon == ' ') {// promotes pointer
								colon++;
							}//Pointing at instruction beginning
							instruction_to_wd_arr(lb_arr, wd_arr, colon, inst);// split instruction to wd_Arr+replace address

						}
						
						else {
							colon = word;
							instruction_to_wd_arr(lb_arr, wd_arr, colon, inst);//is instruction+comment
						} 

					}
					else instruction_to_wd_arr(lb_arr, wd_arr, colon, inst);//is label+instruction
				}
				
				else {//is instruction only
					colon = word;
					instruction_to_wd_arr(lb_arr, wd_arr, colon, inst);
				}
			}

			else if (is_word(word)) {//is .word OR comment

				dot = strchr(word, '.');
				while (*dot != '0' && *dot != '1' && *dot != '2' && *dot != '3' && *dot != '4' && *dot != '5' && *dot != '6' && *dot != '7' && *dot != '8' && *dot != '9') {
					dot++;

					continue;
				}//pointing to first element


				while (*dot != ' ' && *dot != '\t') {
					addr[iter] = *dot;
					dot++;
					iter++;
				}
				iter = 0;

				while (*dot != '0' && *dot != '1' && *dot != '2' && *dot != '3' && *dot != '4' && *dot != '5' && *dot != '6' && *dot != '7' && *dot != '8' && *dot != '9' && *dot != '-') {
					dot++;
				}

				//first element


				while (*dot != ' ' && *dot != '#' && *dot != '\0' && *dot != '\n' && *dot != '\t') {
					data[iter] = *dot;
					dot++;
					iter++;
				}
				to_add = strtol(data, &pend, 0);
				wd_arr[strtol(addr, &pend, 0)].mem = to_add;// change string address to integer address /if there's a problem start check from here
				{//runover instruction pointed to this address (nullify it- empty fields). only data of .word
					memset(wd_arr[strtol(addr, &pend, 0)].opcode, NULL, sizeof(wd_arr[strtol(addr, &pend, 0)].opcode));
					memset(wd_arr[strtol(addr, &pend, 0)].rd, NULL, sizeof(wd_arr[strtol(addr, &pend, 0)].rd));
					memset(wd_arr[strtol(addr, &pend, 0)].rs, NULL, sizeof(wd_arr[strtol(addr, &pend, 0)].rs));
					memset(wd_arr[strtol(addr, &pend, 0)].rt, NULL, sizeof(wd_arr[strtol(addr, &pend, 0)].rt));
					memset(wd_arr[strtol(addr, &pend, 0)].imm, NULL, sizeof(wd_arr[strtol(addr, &pend, 0)].imm));
					wd_arr[strtol(addr, &pend, 0)].imm_num = NULL;
				}
				
			
			
				
				 
				iter = 0;
				memset(addr, 0, sizeof(addr));//reset array to 0
				memset(data, 0, sizeof(data));//reset array to 0
				
				
			




			}
			continue;//is comment
		}
		continue;//NULL
	}
}
void go_down(FILE* file2) {
	char dummy[10] = { 0 };
	fgets(dummy, 10, file2);
}
void fromWordsTotxt(struct word wd_arr[], FILE* file2, FILE* file1) {

	int i = 0;
	for (i = 0;i <= WORD_MAX;i++) {
		if (wd_arr[i].opcode[0] =='a'|| wd_arr[i].opcode[0] == 'b'|| wd_arr[i].opcode[0] == 'h'|| wd_arr[i].opcode[0] == 'i'|| wd_arr[i].opcode[0] == 'j'|| wd_arr[i].opcode[0] == 'l'|| wd_arr[i].opcode[0] == 'r'|| wd_arr[i].opcode[0] == 's'|| wd_arr[i].opcode[0]=='o') {// if there is a word<-> didnt reach end
			if (strcmp(wd_arr[i].opcode, "add") == 0) {//opcode
				fprintf(file2, "00");
				
			}
			else if (strcmp(wd_arr[i].opcode, "sub") == 0) {
				fprintf(file2, "01");
				
			}
			else if (strcmp(wd_arr[i].opcode, "and") == 0) {
				fprintf(file2, "02");
				
			}
			else if (strcmp(wd_arr[i].opcode, "or") == 0) {
				fprintf(file2, "03");
				
			}
			else if (strcmp(wd_arr[i].opcode, "sll") == 0) {
				fprintf(file2, "04");
				
			}
			else if (strcmp(wd_arr[i].opcode, "sra") == 0) {
				fprintf(file2, "05");
				
			}
			else if (strcmp(wd_arr[i].opcode, "srl") == 0) {
				fprintf(file2, "06");
				
			}
			else if (strcmp(wd_arr[i].opcode, "beq") == 0) {
				fprintf(file2, "07");
				
			}
			else if (strcmp(wd_arr[i].opcode, "bne") == 0) {
				fprintf(file2, "08");
				
			}
			else if (strcmp(wd_arr[i].opcode, "blt") == 0) {
				fprintf(file2, "09");
				
			}
			else if (strcmp(wd_arr[i].opcode, "bgt") == 0) {
				fprintf(file2, "0A");
				
			}
			else if (strcmp(wd_arr[i].opcode, "ble") == 0) {
				fprintf(file2, "0B");
				
			}
			else if (strcmp(wd_arr[i].opcode, "bge") == 0) {
				fprintf(file2, "0C");
				
			}
			else if (strcmp(wd_arr[i].opcode, "jal") == 0) {
				fprintf(file2, "0D");
				
			}
			else if (strcmp(wd_arr[i].opcode, "lw") == 0) {
				fprintf(file2, "0E");
				
			}
			else if (strcmp(wd_arr[i].opcode, "sw") == 0) {
				fprintf(file2, "0F");
				
			}
			else if (strcmp(wd_arr[i].opcode, "reti") == 0) {
				fprintf(file2, "10");
				
			}
			else if (strcmp(wd_arr[i].opcode, "in") == 0) {
				fprintf(file2, "11");
				
			}
			else if (strcmp(wd_arr[i].opcode, "out") == 0) {
				fprintf(file2, "12");
				
			}
			else if (strcmp(wd_arr[i].opcode, "halt") == 0) {
				fprintf(file2, "13");
				
			}


			//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//rd
			if (strcmp(wd_arr[i].rd, "$zero") == 0) {
				fprintf(file2, "0");
				
			}
			else if (strcmp(wd_arr[i].rd, "$imm") == 0) {
				fprintf(file2, "1");
				
			}
			else if (strcmp(wd_arr[i].rd, "$v0") == 0) {
				fprintf(file2, "2");
				
			}
			else if (strcmp(wd_arr[i].rd, "$a0") == 0) {
				fprintf(file2, "3");
				
			}
			else if (strcmp(wd_arr[i].rd, "$a1") == 0) {
				fprintf(file2, "4");
				
			}
			else if (strcmp(wd_arr[i].rd, "$t0") == 0) {
				fprintf(file2, "5");
				
			}
			else if (strcmp(wd_arr[i].rd, "$t1") == 0) {
				fprintf(file2, "6");
				
			}
			else if (strcmp(wd_arr[i].rd, "$t2") == 0) {
				fprintf(file2, "7");
				
			}
			else if (strcmp(wd_arr[i].rd, "$t3") == 0) {
				fprintf(file2, "8");
				
			}
			else if (strcmp(wd_arr[i].rd, "$s0") == 0) {
				fprintf(file2, "9");
				
			}
			else if (strcmp(wd_arr[i].rd, "$s1") == 0) {
				fprintf(file2, "A");
				
			}
			else if (strcmp(wd_arr[i].rd, "$s2") == 0) {
				fprintf(file2, "B");
				
			}
			else if (strcmp(wd_arr[i].rd, "$gp") == 0) {
				fprintf(file2, "C");
				
			}
			else if (strcmp(wd_arr[i].rd, "$sp") == 0) {
				fprintf(file2, "D");
				
			}
			else if (strcmp(wd_arr[i].rd, "$fp") == 0) {
				fprintf(file2, "E");
				
			}
			else if (strcmp(wd_arr[i].rd, "$ra") == 0) {
				fprintf(file2, "F");
				
			}
			//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~`//rs


			if (strcmp(wd_arr[i].rs, "$zero") == 0) {
				fprintf(file2, "0");
				
			}
			else if (strcmp(wd_arr[i].rs, "$imm") == 0) {
				fprintf(file2, "1");
				
			}
			else if (strcmp(wd_arr[i].rs, "$v0") == 0) {
				fprintf(file2, "2");
				
			}
			else if (strcmp(wd_arr[i].rs, "$a0") == 0) {
				fprintf(file2, "3");
				
			}
			else if (strcmp(wd_arr[i].rs, "$a1") == 0) {
				fprintf(file2, "4");
				
			}
			else if (strcmp(wd_arr[i].rs, "$t0") == 0) {
				fprintf(file2, "5");
				
			}
			else if (strcmp(wd_arr[i].rs, "$t1") == 0) {
				fprintf(file2, "6");
				
			}
			else if (strcmp(wd_arr[i].rs, "$t2") == 0) {
				fprintf(file2, "7");
				
			}
			else if (strcmp(wd_arr[i].rs, "$t3") == 0) {
				fprintf(file2, "8");
				
			}
			else if (strcmp(wd_arr[i].rs, "$s0") == 0) {
				fprintf(file2, "9");
				
			}
			else if (strcmp(wd_arr[i].rs, "$s1") == 0) {
				fprintf(file2, "A");
				
			}
			else if (strcmp(wd_arr[i].rs, "$s2") == 0) {
				fprintf(file2, "B");
				
			}
			else if (strcmp(wd_arr[i].rs, "$gp") == 0) {
				fprintf(file2, "C");
				
			}
			else if (strcmp(wd_arr[i].rs, "$sp") == 0) {
				fprintf(file2, "D");
				
			}
			else if (strcmp(wd_arr[i].rs, "$fp") == 0) {
				fprintf(file2, "E");
				
			}
			else if (strcmp(wd_arr[i].rs, "$ra") == 0) {
				fprintf(file2, "F");
				
			}
			//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~rt
			if (strcmp(wd_arr[i].rt, "$zero") == 0) {
				fprintf(file2, "0");
				
			}
			else if (strcmp(wd_arr[i].rt, "$imm") == 0) {
				fprintf(file2, "1");
				
			}
			else if (strcmp(wd_arr[i].rt, "$v0") == 0) {
				fprintf(file2, "2");
				
			}
			else if (strcmp(wd_arr[i].rt, "$a0") == 0) {
				fprintf(file2, "3");
				
			}
			else if (strcmp(wd_arr[i].rt, "$a1") == 0) {
				fprintf(file2, "4");
				
			}
			else if (strcmp(wd_arr[i].rt, "$t0") == 0) {
				fprintf(file2, "5");
				
			}
			else if (strcmp(wd_arr[i].rt, "$t1") == 0) {
				fprintf(file2, "6");
				
			}
			else if (strcmp(wd_arr[i].rt, "$t2") == 0) {
				fprintf(file2, "7");
				
			}
			else if (strcmp(wd_arr[i].rt, "$t3") == 0) {
				fprintf(file2, "8");
				
			}
			else if (strcmp(wd_arr[i].rt, "$s0") == 0) {
				fprintf(file2, "9");
				
			}
			else if (strcmp(wd_arr[i].rt, "$s1") == 0) {
				fprintf(file2, "A");
				
			}
			else if (strcmp(wd_arr[i].rt, "$s2") == 0) {
				fprintf(file2, "B");
				
			}
			else if (strcmp(wd_arr[i].rt, "$gp") == 0) {
				fprintf(file2, "C");
				
			}
			else if (strcmp(wd_arr[i].rt, "$sp") == 0) {
				fprintf(file2, "D");
				
			}
			else if (strcmp(wd_arr[i].rt, "$fp") == 0) {
				fprintf(file2, "E");
				
			}
			else if (strcmp(wd_arr[i].rt, "$ra") == 0) {
				fprintf(file2, "F");
				
			}
			//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~immediate
			fprintf(file2, "%03X",wd_arr[i].imm_num & 0xFFF);
			fprintf(file2, "\n");

			

		}
		else if (wd_arr[i].mem != NULL) {
		fprintf(file2, "%08X\n", wd_arr[i].mem & 0xFFFFFFFF);
		}
		else{
			fprintf(file2, "00000000\n");
		}
	}
}
int main(int argc, char* argv[]){
	errno_t err1;
	errno_t err2;
	//זיכרון ראשי
	struct label lb_arr[MAX_SIZE_LB_ARRAY] = { 0 }; //array of label structres
	struct word wd_arr[WORD_MAX] = { 0 };
	
	FILE* file1 = 0;
	FILE* file2 = 0;
	FILE* file3 = 0;
	int lb_num = 0;
	
	if (err1 = fopen_s(&file1, argv[1], "r") != 0) {
		printf("couldn't open .asm file");
		exit(1);
	}
	
	
	
	lb_num = get_labels(lb_arr, file1);//first read- return number of labels and update labels array(name-row)
	fclose(file1);

	if (err1 = fopen_s(&file1, argv[1], "r") != 0) {
		printf("couldn't open .asm file");
		exit(1);
	}

	if (err2 = fopen_s(&file2, argv[2], "w+") != 0) {
		printf("couldn't open memin.txt");
		exit(1);
	}

	change_labels(lb_arr, file1, wd_arr, file2);// change labels and update words instruction to prepeare for simulator also update memin.txt

	fromWordsTotxt(wd_arr, file2, file1);
	fclose(file1); fclose(file2);
	return 0;
};
