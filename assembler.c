/*
   +--------------------------------------------------------------+
   |                 System Programming Project 2                 |
   |                    ( SIC/XE Assembler )                      |
   |                                                              |
   |  File    : assembler.c                                       |
   |  Created : Mar 18 ~ Apr 9, 2018                              |
   |  Author  : Ye-eun Lee                                        |
   +--------------------------------------------------------------+
 */

#include "main.h"
#include "assembler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//enum to distinguish instruction type*/
enum ASSEM_TYPE{
	ERROR = -1,
	COMMENT = 0,
	PSEUDO_INST = 1,
	INST= 2

};
//enum to distinguish pseudo instruction type
enum PSEUDO_INSTR_TYPE{
	START = 0,
	END,
	BASE,
	BYTE,
	WORD,
	RESB,
	RESW
};

//enum about address_mode and n,i bit
enum ADDRESS_MODE{
	SIC = 0,
	IMMED = 1,
	INDIR,
	SIMPLE
};

//string about pseudo instruction
char pseudo_instr[7][10]={
	"START","END","BASE","BYTE","WORD","RESB","RESW"
};	

//pc and base address
int pc_addr=0,base_addr=0;

//assemble node
assem_node *assem_head = NULL;
assem_node *assem_rear = NULL;

//symbol table
symbol_table_node symbol_table[26];



int Assemble(char *file_name){
	//Excution Assemble command(assemble file and make '.lst''.obj')
	//if .asm file have error return -1
	
	FILE *fp=NULL;

	//check file type and name
	if(!IsAssemFile(file_name)){
		printf("%s is not .asm file\n", file_name);
		return -1;
	}
	
	fp = fopen(file_name, "r");
	
	if(fp==NULL){
		printf("%s not founded\n",file_name);
		return -1;
	}

	fclose(fp);

	//initialization
	pc_addr=0,base_addr=0;
	assem_head = NULL;
	assem_rear = NULL;
	InitAssemNode();
	InitSymbolTable();

	//process string and symbol table
	if(AssemPass1(file_name)==-1){
		InitAssemNode();
		InitSymbolTable();
		return -1;
	}

	//make '.lst' '.obj'
	if(AssemPass2(file_name)==-1){
		InitAssemNode();
		InitSymbolTable();
		return -1;
	}

	
	return 0;

}



int Symbol(){
	//Excution symbol command
	//print symbol table
	int i;
	symbol_node* cur;

	if(assem_head==NULL){
		printf("A file recently assembled doesn't exist!\n");
		return -1;
	}
	for( i='Z'-'A' ; i>=0; i--){
		for(cur = symbol_table[i].next ; cur!=NULL; cur = cur->next){
			printf("\t%s\t%04X\n",cur->sym,cur->loc);
		}
	}
	return 0;
}

int IsAssemFile(char *file_name){
	// Check that file type is '.asm' 
	// If filename in par[0] is '.asm' file, return 1
	// If not, return 0
	int i;
	int check_asm=0;

	for(i=0 ; i< (int)strlen(file_name) ; i++){
		if(file_name[i]=='.'){
			if(strlen(file_name)-i == 4){
				if(file_name[i+1]=='a' && file_name[i+2]=='s' && file_name[i+3]=='m') return 1;
				else return 0;
			}
			else return 0;
		}
	}
	if(check_asm == 4) return 1;

	return 0;

}




int AssemPass2(char* file_name){
	//Make .lst file and .obj
	
	FILE *fp = fopen(file_name, "r");	//read .asm file 
	FILE *list;		//FILE POINTER for .lst file
	FILE *object;	//FILE POINTER for .obj file
	char list_name[MAX_FILENAME];	//string for list file name
	char object_name[MAX_FILENAME];	//string for object file name
	char asm_line[MAX_LINESIZE];	//string to get 1 line in .asm file
	int i;	
	int t_total=0,t_end_flag,t_next_loc; //t_total : count byte size to print to object file
							//t_end_flag : line feed sign to print to object file
	assem_node* tcnt;
	assem_node* cur = assem_head;	//Pointer to point current assem_node.
	
	//process file name to get list_name, object_name
	for(i=0 ; i<(int)strlen(file_name); i++){
		if(file_name[i] == '.')
			break;
		list_name[i] = file_name[i];
		object_name[i] = file_name[i];
	}
	strcat(list_name, ".lst");
	strcat(object_name, ".obj");

	//If '.asm' file is not emtpy or has "START" at first line,
	//make list file and object file 
	//and write first line data to list and obj. 
	if(!strcmp(cur->inst,"START")||strcmp(cur->inst, "\0") ){ 
		list = fopen(list_name,"w");
		object = fopen(object_name,"w");
		if(fgets(asm_line,MAX_LINESIZE,fp) == NULL) return 0;
		fprintf(list,"%d\t%04X\t%s",cur->line,cur->loc,asm_line);
		fprintf(object,"H%-6s%06X%06X",cur->sym,assem_head->loc, (assem_rear->loc - assem_head->loc));
		cur=cur->next;
	}
	
	// Read data one line in '.asm' file at a time 
	// and write to '.lst'
	while(fgets(asm_line,MAX_LINESIZE,fp) != NULL || cur==NULL ){
		if(asm_line[(int)strlen(asm_line)-1]=='\n')
			asm_line[(int)strlen(asm_line)-1]='\0';

		//process and write comment to list file
		if(cur->type == COMMENT)
			fprintf(list,"%d\t%s\n",cur->line,asm_line);
		
		//If instruction is base, set base address and print to list file
		else if(!strcmp(cur->inst,"BASE")){
			if(cur->operand[0][0]<='0' && cur->operand[0][0]>='9'){
				base_addr = HexToDec(cur->operand[0]);
			}
			else{
				base_addr = SearchSymbol(cur->operand[0]);
				if(base_addr == -1){
					remove(list_name);
					remove(object_name);
					PRINT_ERROR(cur->line,"Referenced an undeclared symbol!");
					return -1;
				}
			}
			fprintf(list,"%d\t\t%s\n",cur->line,asm_line);
		}

	
		else{
			//If instruction is "END", print to list file and break this loop
			if( !strcmp(cur->inst,"END")){
				cur->t_flag=1;

				fprintf(list,"%d\t\t%-33s\n",cur->line,asm_line);
				break;
			}
			fprintf(list,"%d\t%04X\t%-33s",cur->line,cur->loc,asm_line);
			
			//Get object code
			//If it has error, return -1(ERROR)
			if(GetObj(cur)==ERROR){
				remove(list_name);
				remove(object_name);
				return ERROR;
			}
			if(t_total==0) {
				cur->t_flag = 1;
			}
			//If current line has print object code
			if(cur->size!=0){

				//Add byte size to t_total to make object file
				//Print object code to list file according to size of object code
				switch(cur->size){
					case 1:
						t_total+=1;
						fprintf(list,"%02X",cur->obj);
						break;
					case 2:
						t_total+=2;
						fprintf(list,"%04X",cur->obj);
						break;
					case 3:
						t_total+=3;
						fprintf(list,"%06X",cur->obj);
						break;
					case 4:
						t_total+=4;
						fprintf(list,"%08X",cur->obj);
						break;
				}
			}

			// To match the object file format(consider linesize, constant ),
			// check t_flag  at the beginning of the line
			if(t_end_flag==1 || t_total>OBJ_LINE_SIZE){
				//consider line size
				cur->t_flag = 1;
				t_end_flag = 0;
				t_total = cur->size;
			}
			else if(!strcmp(cur->inst,"BYTE")||!strcmp(cur->inst,"WORD")){
				//consider constant
				t_end_flag=1;
				t_total = 0;
			}
			fprintf(list,"\n");
		}

		cur = cur->next;
	}

	//Make .obj file using data in assem_node
	for(cur = assem_head->next; cur!=NULL ;cur = cur->next){
		//if end break this loop
		if(!strcmp(cur->inst,"END")){
			break;
		}
		if(cur->size==0){
			continue;
		}

		if(cur->t_flag){
			//get line size
			for(tcnt=cur->next;tcnt!=NULL; tcnt=tcnt->next){
				if(tcnt->t_flag==1){
					t_next_loc = tcnt->loc;
					break;
				}
			}
			//print 'T' and address
			fprintf(object, "\nT%06X%02X",cur->loc,t_next_loc - (cur->loc) );
		}

		//print object code to '.obj' according to size t
		switch(cur->size){
			case 1:
				fprintf(object,"%02X",cur->obj);
				break;
			case 2:
				fprintf(object,"%04X",cur->obj);
				break;
			case 3:
				fprintf(object,"%06X",cur->obj);
				break;
			case 4:
				fprintf(object,"%08X",cur->obj);
				break;
		}
	}

	//print modification to '.obj'
	for(cur = assem_head->next; cur!=NULL ;cur = cur->next){
		//if end, print 'X' and break this loop
		if(!strcmp(cur->inst,"END")){		
			fprintf(object, "\nE%06X\n",assem_head->loc);
			break;
		}	
		//print 'M' and information about address
		if(cur->form == 4 
				&& ('0'>cur->operand[0][0]||cur->operand[0][0]>'9')){
			fprintf(object, "\nM%06X%02X",(cur->loc)-(assem_head->loc)+1,5);
		}
		
		
	}	

	printf("output file : [%s], [%s]\n",list_name,object_name);

	//file close
	fclose(fp);
	fclose(list);
	fclose(object);
	return 0;	
}

int GetObj(assem_node *cur_node){
	//Get obejct code in current node(cur_node)
	//If this node(cur_node) has error, return -1
	//Else, return 0;
	int sym_addr;
	int i=0;

	//In pseudo instruction, set object code according to the instruction
	if(cur_node->type == PSEUDO_INST){
		switch(FindPseudoInstr(cur_node->inst)){
			case BYTE:
			case WORD:
				if(!strcmp(cur_node->operand[0],"X")){
					cur_node->size  = (int)(strlen(cur_node->operand[0])/2+strlen(cur_node->operand[0])%2);
					cur_node->obj = HexToDec(cur_node->operand[1]);
				}
				else if(!strcmp(cur_node->operand[0],"C")){
					cur_node->size  = (int)strlen( cur_node->operand[0] );
					for(i=0 ; i < strlen(cur_node->operand[1]) ; i++){
						cur_node->obj *= (16*16);
						cur_node->obj += cur_node->operand[1][i];
					}
				}
				else{
					cur_node->obj = StrToDec(cur_node->operand[0]);
				}
				break;
			default:
				break;
		}
	}
	//In instruction, Set object code accrding to the format
	else if(cur_node->type == INST){
		switch(cur_node->form){
			//in format 1
			case 1:
				cur_node->size = 1;
				cur_node->obj = cur_node->opcode;
				break;
			//in format 2
			case 2:
				cur_node->size = 2;
				cur_node->obj = (cur_node->opcode)*16*16;
				if(!strcmp(cur_node->inst,"CLEAR")|| !strcmp(cur_node->inst,"TIXR")){
					cur_node->obj += FindReg(cur_node->operand[0])*16;
				}
				else if(!strcmp(cur_node->inst,"CVC")){
					cur_node->obj += HexToDec(cur_node->operand[0])*16;
				}
				else if(!strcmp(cur_node->inst,"SHIFTL")|| !strcmp(cur_node->inst,"SHIFTR")){
					cur_node->obj += FindReg(cur_node->operand[0])*16;
					cur_node->obj += HexToDec(cur_node->operand[1]);
				}
				else{
					cur_node->obj += FindReg(cur_node->operand[0])*16;
					cur_node->obj += FindReg(cur_node->operand[1]);
				}
				break;

			//in format 3
			case 3:
				pc_addr = cur_node->loc + 3;
				cur_node->size = 3;
				cur_node->obj = (cur_node->opcode)*16*16*16*16;
				if( !strcmp(cur_node->operand[1], "X") ){
					cur_node->obj += (2*2*2)*(16*16*16);
				}

				//Set object code according to the address mode
				switch(cur_node->addr_mode){
					//in immediate mode
					case IMMED:
						cur_node->obj += (IMMED)*16*16*16*16;
						if('0' <= cur_node->operand[0][0]
								&& cur_node->operand[0][0] <= '9')
							cur_node->obj += StrToDec(cur_node->operand[0]);
						else{
							sym_addr = SearchSymbol(cur_node->operand[0]);
							if(sym_addr==-1){
								PRINT_ERROR(cur_node->line,"Referenced an undeclared symbol!");
								return -1;
							}
							//pc relative
							if( sym_addr-pc_addr >= -2048 && sym_addr-pc_addr <= 2047){
								if((sym_addr-pc_addr)<0)
									cur_node->obj += 16*16*16; 
								cur_node->obj += sym_addr-pc_addr;
								cur_node->obj += 2*16*16*16;
							}
							//base relative
							else if( 0<=sym_addr-base_addr && sym_addr-base_addr <= 4095){
								cur_node->obj += sym_addr-base_addr;
								cur_node->obj += 4*16*16*16;
							}
						}
						break;
					//in indircet mode
					case INDIR:
						cur_node->obj += (INDIR)*16*16*16*16;
						sym_addr = SearchSymbol(cur_node->operand[0]);
						if(sym_addr==-1){
							PRINT_ERROR(cur_node->line,"Referenced an undeclared symbol!");
							return -1;
						}
						//pc relative
						if( sym_addr-pc_addr >= -2048 && sym_addr-pc_addr <= 2047){
							if((sym_addr-pc_addr)<0)
								cur_node->obj += 16*16*16; 
							cur_node->obj += sym_addr-pc_addr;
							cur_node->obj += 2*16*16*16;
						}
						//base relative
						else if( 0<=sym_addr-base_addr && sym_addr-base_addr <= 4095){
							cur_node->obj += sym_addr-base_addr;
							cur_node->obj += 4*16*16*16;
						}	
						break;
					//in simple addressing mode or SIC format
					default:
						if(!strcmp(cur_node->operand[0],"\0")){
							cur_node->obj += (SIMPLE)*16*16*16*16;
							cur_node->addr_mode = SIMPLE;
						}
						else{
							sym_addr = SearchSymbol(cur_node->operand[0]);
							if(sym_addr==-1){
								PRINT_ERROR(cur_node->line,"Referenced an undeclared symbol!");
								return -1;
							}
							//pc relative
							if( sym_addr-pc_addr >= -2048 && sym_addr-pc_addr <= 2047){
								cur_node->obj += (SIMPLE)*16*16*16*16;
								if((sym_addr-pc_addr)<0)
									cur_node->obj += 16*16*16; 
								cur_node->obj += sym_addr-pc_addr;
								cur_node->obj += 2*16*16*16;
							}
							//base relative
							else if( 0<=sym_addr-base_addr && sym_addr-base_addr <= 4095){
								cur_node->obj += (SIMPLE)*16*16*16*16;
								cur_node->obj += sym_addr-base_addr;
								cur_node->obj += 4*16*16*16;
							}
							else{
								cur_node->addr_mode = SIC;
								cur_node->obj += sym_addr;
							}

						}
						break;
				}
			
				
				break;
			case 4:
				//format 4
				cur_node->size = 4;
				cur_node->obj = ((cur_node->opcode)+(cur_node->addr_mode))*16*16*16*16*16*16;
				cur_node->obj += 1*16*16*16*16*16;
				//simple address mode
				if(cur_node->addr_mode == SIMPLE){
					if(SearchSymbol(cur_node->operand[0])==-1){
						PRINT_ERROR(cur_node->line,"Referenced an undeclared symbol!");
						return -1;
					}
					cur_node->obj += SearchSymbol(cur_node->operand[0]);
				}
				//immediate address mode
				else if(cur_node->addr_mode == IMMED){
					cur_node->obj += StrToDec(cur_node->operand[0]);
				}
				break;
		}
	}
	return 0;
}

int AssemPass1(char* file_name){
	//Function to read '.asm' file 
	//and make assem_ node that have data about assemble code.

	FILE *fp = fopen(file_name, "r");
	char asm_line[MAX_LINESIZE];
	char tk_str[MAX_ASM_TOKEN][MAX_LINESIZE] = {'\0'};

	//read and save data
	while(fgets(asm_line,MAX_LINESIZE,fp)!=NULL){
		//token one line string in .asm file
		AssemToken(asm_line, tk_str);
		
		//make assemble node. If this line have error return -1
		if (MakeAssemNode(tk_str) ) return ERROR;
		
		//if end, finish this loop
		if(FindPseudoInstr(assem_rear->inst) == END) 
			break;

	}
	fclose(fp);
	return 0;
}
void InitSymbolTable(){
	//Function to free and intialize symbol table.
	int i=0;
	symbol_node *tmp_p, *del_p;

	for(i=0 ; i<26; i++){
		//free
		tmp_p = symbol_table[i].next;	
		while(1){	
			if(tmp_p == NULL) break;
			del_p = tmp_p;
			tmp_p = tmp_p->next;
			free(del_p);
		}
		//initialize
		symbol_table[i].alpha = 'a'+i;
		symbol_table[i].next = NULL;
	}
}
int AssemToken(char asm_line[], char tk_str[][MAX_LINESIZE]){
	int i=0, j=0;
	char *tk;
	char asm_str[MAX_LINESIZE] = {'\0'};


	for(i=0; i < (int)strlen(asm_line) ;){
		if(asm_line[i]==','){
			i++;
			asm_str[j++] = ' ';
			asm_str[j++] = ',';
			asm_str[j++] = ' ';
			continue;
		}
		asm_str[j++]=asm_line[i++];
	}
	asm_str[j] = '\0';
	for(i=0; i<MAX_ASM_TOKEN; i++){
		tk_str[i][0] = '\0';
	}

	i=0;

	tk = strtok(asm_str, " \t\n");
	if(tk==NULL) return 0;
	do{
		strcpy(tk_str[i],tk);

		//process comment
		if(tk_str[i][0]=='.'){
			strcpy(tk_str[i],".");
			strcpy(tk_str[i+1],asm_line);
			break;
		}

		i++;

	}while( (tk = strtok(NULL, " \t\n")) );
	return 0;
}



int MakeAssemNode(char tk_str[][MAX_LINESIZE]){
	//make node to save data in assemble file using tk_str(token string)
	int type;	//instruction type
	assem_node *new_node;	//node to create
	new_node = (assem_node*)malloc( sizeof(assem_node) );

	//node initialization;
	strcpy(new_node->comment, "\0");
	strcpy(new_node->sym, "\0");
	strcpy(new_node->inst, "\0");
	strcpy(new_node->operand[0], "\0");
	strcpy(new_node->operand[1], "\0");
	
	new_node->obj = 0;
	new_node->next = NULL;
	new_node->type = -1;
	new_node->line = (assem_head==NULL) ? 5  : ( (assem_rear->line) + 5 );

	type=GetType_and_SaveInst(new_node,tk_str);
	
	switch(type){
		case INST:
			if(GetOperand(new_node,tk_str) == ERROR) return ERROR;
			break;
		case PSEUDO_INST:
			if(GetPseudoOperand(new_node,tk_str)==ERROR) return ERROR;
			break;
		case COMMENT:
			break;
		default:
			return ERROR;
			break;
	}	//link node

	GetLoc(new_node);
	if(MakeSymbolTable(new_node)==ERROR) return ERROR;

	if(assem_head==NULL){
		assem_rear = new_node;
		assem_head = new_node;
	}
	else{
		assem_rear->next = new_node;
		assem_rear = new_node;
	}
	return 0;
}
int StrToDec(char* str){
	//make str to decimal number and return
	//
	int i=0;
	int dec=0;
	if(str[0] == '\0'){
		return -1;
	}
	for(i=0 ; i<(int)strlen(str); i++){
		dec *= 10;
		dec += str[i]-'0';
	}
	return dec;
}
void GetLoc(assem_node *new_node){
	//function to get location using data in new_node 
	int bef_loc;

	//if new_node is first
	if(assem_rear == NULL){
		new_node->loc = pc_addr;
	}

	else{
		//get before location
		 bef_loc = assem_rear -> loc;
		 
		 //get loction according to instruction type
		switch(assem_rear->type){
			case COMMENT:
				new_node->loc = bef_loc;
				break;
			case PSEUDO_INST:
				//get loctaion accrding to pseudo instruction.
				switch(FindPseudoInstr(assem_rear->inst)){
					case START:
					case BASE:
						new_node->loc = bef_loc;
						break;
					case BYTE:
						//if hexcadecimal
						if(assem_rear->operand[0][0] == 'X')
							new_node->loc = bef_loc + (HexToDec(assem_rear->operand[1])/ 256 )+ 1;
						//if charictor
						else if(assem_rear->operand[0][0] == 'C')
							new_node->loc = bef_loc + strlen(assem_rear->operand[1]);
						break;
					case WORD:
						new_node->loc = bef_loc + 3;
						break;
					case RESB:
						new_node->loc = bef_loc + StrToDec(assem_rear->operand[0]);
						break;
					case RESW:
						new_node->loc  = bef_loc + 3*StrToDec(assem_rear->operand[0]);
						break;
				}
				break;

			//get loction if instruction type is instruction
			case INST:
				new_node->loc = bef_loc + assem_rear -> form;
				break;
				
		}
	}
}

int FindReg(char *str){
	//Find regisister and return resister number
	//If str is not register, return -1
	int i;
	char reg[9][4] = {"A","X","L","B","S","T","F","PC","SW"};//register terminate
	for(i=0; i<9; i++){
		if(!strcmp(str,reg[i])) return i;
	}
	return -1;
}

int MakeSymbolTable(assem_node *new_node){
	//Make symbol table using assem_node
	//symbol save in symbol_node['A'=0 ~ 'F'15 ]
	//If successfully end , return 0
	//It this code have error return -1
	
	symbol_node *cur,*bef;	
	symbol_node *new_sym;	//node to save symbol
	int start_flag=1;	
	int i=1;

	//if symbol is empty or start return 0
	if( !strcmp(new_node->sym, "\0") )	return 0;	
	else if( !strcmp(new_node->inst,"START") ) return 0;

	//allocate symbol_node  and initial symbol_node
	new_sym = (symbol_node*)malloc(sizeof(symbol_node));
	strcpy( new_sym -> sym , new_node->sym);
	new_sym -> loc =  new_node -> loc;
		
	//If symbol table[] is emtpy, link new symbol node and return 0
	if(symbol_table[ (new_node->sym[0]) - 'A' ].next == NULL ){
		symbol_table[ ((new_node->sym)[0]) - 'A' ].next = new_sym;
		return 0;
	}


	//check overlap error!
	if(SearchSymbol( new_node->sym)!=-1){
		PRINT_ERROR(new_node->line , "Overlap symbols");
		return ERROR;
	}

	//find place to link
	bef = symbol_table[ (new_node->sym)[0] - 'A' ].next; 
	for(cur = symbol_table[ (new_node->sym)[0] - 'A' ].next; cur!=NULL; ){
		
		//if new symbol is bigger than next symbol,
		//link new symbol node before next symbol node
		if( new_sym->sym[i] > cur->sym[i]){
			if(start_flag) {
				new_sym->next = bef;
				symbol_table[(new_node->sym)[0]-'A'].next = new_sym;
			}
			else {					
				bef->next = new_sym;
				new_sym->next = cur;
			}
			break;
		}
		
		//if same alphabet check to next alphabet
		else if( new_sym->sym[i]== cur->sym[i] ){				
			i++;
		}
		else{
			bef = cur;
			cur = cur->next;
		}
		start_flag = 1;
	}

	//linke to last
	if(cur == NULL){
		new_sym->next = NULL;
		bef->next = new_sym; 
	}
	return 0;	
}

int SearchSymbol(char *key){
	//Function to find symbol
	//Find : return addr
	//unfind : -1
	symbol_node *cur;

	// If not format or table is empty return -1
	if(key[0]<'A' || key[0]>'Z')
		return -1;
	if(symbol_table[ key[0]-'A'].next==NULL)
		return -1;


	//Check linked symbol node with symbol table
	for(cur = symbol_table[ key[0]-'A' ].next; cur!=NULL; ){
		if(cur->sym==NULL) break;
		if(!strcmp(cur->sym, key))	return cur->loc;
		cur = cur->next;
	}
	return -1;
}

int GetPseudoOperand(assem_node *new_node, char tk_str[][MAX_LINESIZE]){
	//Precess operand in pseudo instruction
	//if this line has error, return -1
	//else return 0
	
	int i;
	int is_sym = (!strcmp(new_node->sym , "\0")) ? 0 : 1;
	
	//process operand according to instruction
	switch(FindPseudoInstr(new_node->inst)){
		case START:
			// if opernad is hex, then save data
			if(IsHex(tk_str[1+is_sym])){
				pc_addr = HexToDec(tk_str[1+is_sym]);
				strcpy( new_node -> operand[0] , tk_str[1+is_sym]);
				return 0;
			}
			else{
				PRINT_ERROR(new_node->line, "Incorrect format!");
				return ERROR;
			}
			break;
		case END:
			if(!strcmp(tk_str[2+is_sym],"\0") ){
				strcpy(new_node -> operand[0], tk_str[1+is_sym]);	
				return 0;
			}
			else {
				PRINT_ERROR(new_node->line, "1Incorrect format!");
				return ERROR;
			}
			break;
		case BASE:			
			if(!strcmp(tk_str[2+is_sym],"\0") ){
				strcpy(new_node -> operand[0], tk_str[1+is_sym]);	
				return 0;
			}
			break;
		case BYTE:
		case WORD:
			//Check the format and find type of operator
			//and process operand accorcing to operand type
			if((tk_str[1+is_sym][0]=='X' ||tk_str[1+is_sym][0]=='C')
					&& tk_str[1+is_sym][strlen(tk_str[1+is_sym]) - 1] == '\'' 
						&& (tk_str[1+is_sym][1]=='\'')){

				new_node -> operand[0][0] = tk_str[1+is_sym][0];
				tk_str[1+is_sym][strlen(tk_str[1+is_sym]) - 1] = '\0'; 
				strcpy((new_node -> operand[1]), &tk_str[1+is_sym][2]);

				if(tk_str[1+is_sym][0]=='X' && !IsHex(new_node->operand[1])){ 
					PRINT_ERROR(new_node->line, "data type error!");
					return ERROR;
				}
				return 0;
			}
			else if((tk_str[1+is_sym])){
				for( i=0; i<(int)strlen(tk_str[1+is_sym]); i++){
					if(tk_str[1+is_sym][i]<'0' || tk_str[1+is_sym][i]>'9'){
						PRINT_ERROR(new_node->line, "Incorrect format!");
						return ERROR;
					}
				}
				strcpy(new_node ->operand[0] , tk_str[1+is_sym]);
				return 0;
			}
			else{
				PRINT_ERROR(new_node->line, "Incorrect format!");
				return ERROR;
			}
		case RESB:
		case RESW:
			for( i=0; i<(int)strlen(tk_str[1+is_sym]); i++){
				if(tk_str[1+is_sym][i]<'0' || tk_str[1+is_sym][i]>'9'){
					PRINT_ERROR(new_node->line, "Incorrect format!");
					return ERROR;
				}
			}
			strcpy( new_node->operand[0] ,tk_str[1+is_sym]);
			break;
	}
	return 0;
}
int IsReg(char *c){
	//If c is register, return 1
	//if c is not register, return 0
	int i;
	char reg[10][3] = {"A","X","L","B","S","T","F","PC","SW"};
	for(i=0; i<9; i++){
		if(!strcmp(c, reg[i]) ){
			return 1;
		}
	}
	return 0;
}

int GetOperand(assem_node *new_node, char tk_str[][MAX_LINESIZE]){

	int i;
	int is_sym = (!strcmp(new_node->sym , "\0")) ? 0 : 1; //If this line has symbol add 1
	char oper2_except[4][3][10] = {
		{"CLEAR","TIXR"},//r1
		{"CVC","CVC"},	//n
		{"SHIFTL", "SHIFTR"}//r1, n
	}; //instruction to process exceptionlly in format 2

	switch(new_node->form){
		//in format 1
		case 1:
			//check error (has more instruction)
			if(strcmp(tk_str[1 + is_sym],"\0")){
				PRINT_ERROR(new_node->line, "Incorrect format!");
				return ERROR;
			}
			return 0;
			break;

		//in format 2
		case 2:
			//process command exceptionally in format 2
			for(i=0;i<=2;i++){
				if( ( !strcmp(oper2_except[i][0],new_node->inst) )
						|| (!strcmp(oper2_except[i][1],new_node->inst)) ){
					switch(i){
						//"CLEAR","TIXR" - r1
						case 0:
							if(IsReg(tk_str[1+is_sym])){
								strcpy(new_node->operand[0] , tk_str[1+is_sym]);
								return 0;
							}
							else{	
								PRINT_ERROR(new_node->line, "Incorrect format!");
								return ERROR;
							}
							break;
						//"CVC","CVC" - n
						case 1:
							if(IsHex(tk_str[1+is_sym])){
								strcpy(new_node->operand[0], tk_str[1+is_sym]);
								return 0;
							}
							else{
								PRINT_ERROR(new_node->line, "Incorrect format!");
								return ERROR;
							}
							break;
						//"SHIFTL", "SHIFTR" - r1, n
						case 2:
							if(IsReg(tk_str[1+is_sym]) && IsHex(tk_str[3+is_sym])){
								strcpy(new_node->operand[0], tk_str[1+is_sym]);
								strcpy(new_node->operand[1], tk_str[3+is_sym]);
								return 0;
							}
							else{
								PRINT_ERROR(new_node->line, "Incorrect format!");			
								return ERROR;
							}
							break;
					}
				}
			}
			// r1, r2
			if(IsReg(tk_str[1+is_sym])&&IsReg(tk_str[3+is_sym])){
				strcpy(new_node->operand[0], tk_str[1+is_sym]);
				strcpy(new_node->operand[1], tk_str[3+is_sym]);
				return 0;
			}
			else{
				PRINT_ERROR(new_node->line, "Incorrect format!");			
				return ERROR;
			}
			break;
		case 3:
		case 4:
			//distinguish address mode
			switch(tk_str[1+is_sym][0]){
				case '#':
					strcpy(new_node->operand[0], &tk_str[1+is_sym][1]);
					new_node -> addr_mode = IMMED;
					break;
				case '@':
					strcpy(new_node->operand[0], &tk_str[1+is_sym][1]);
					new_node -> addr_mode = INDIR;
					break;
				default:
					strcpy(new_node->operand[0], tk_str[1+is_sym]);
					new_node ->addr_mode = SIMPLE;
					break;
			}
			

			//no operand instruction
			if( (!strcmp(new_node->inst, "RSUB")) && (!strcmp(tk_str[1+is_sym],"\0")))
				return 0;
			//if operand has x resgister
			else if(!strcmp(tk_str[3+is_sym],"X")){
				strcpy(new_node->operand[1],"X");
				return 0;
			}
			else if(!strcmp(tk_str[2+is_sym],"\0")){
				return 0;
			}
			else{
				PRINT_ERROR(new_node->line, "Incorrect format!");			
				return ERROR;
			}
			break;
	}
	return 0;
}
int GetType_and_SaveInst(assem_node *new_node, char tk_str[][MAX_LINESIZE]){
	//distinguish instruction type and save instruction and symbol
	//and processing error about insruction and symbol
	
	int opcode[3];
	int pseudo[3];
	int i;

	//comment
	if(tk_str[0][0]=='.'){
		new_node->type = COMMENT;
		strcpy(new_node->comment, tk_str[1]);
		return COMMENT;
	}
	
	//format 4
	for(i=0; i<2; i++){
		if(tk_str[i][0]=='+') {
			opcode[3] = FindOpcode(&tk_str[i][1]);
			if(opcode[3] != -1){
				if(!strcmp(FindForm(&tk_str[i][1]),"3/4")){
					new_node->form = 4;
					new_node->opcode = opcode[3];
					strcpy(new_node->inst , &tk_str[i][1]);
					if(i==1){
						strcpy(new_node->sym, tk_str[0]);
					}
					new_node->type = INST;
					return INST;
				}
				else{
					PRINT_ERROR(new_node->line, "invalid intstruction");
					return ERROR;
				}
			}
		}
	}

	//Get symbol, instruction, opcode o.w

	//If this instruction has opcode or pseudo code, find and save to opcode[i] and pseudo[i]
	//If not, opcode[i] or pseudo[i] is -1
	for(int i=0; i<2; i++){
		opcode[i] = FindOpcode(tk_str[i]);
		pseudo[i] = FindPseudoInstr(tk_str[i]);
	}	
	
	
	//Process ERROR: 2/no instruction in a line 
	if(opcode[0]==-1 && pseudo[0]==-1 
			&& opcode[1]== -1 && pseudo[1]==-1){
		PRINT_ERROR(new_node->line, "invalid instruction");
		return ERROR;
	}	
	else if((opcode[0]!=-1 || pseudo[0]!=-1) 
			&& (opcode[1]!= -1 || pseudo[1]!=-1)){
		PRINT_ERROR(new_node->line, "Symbol name overlaps with instruction");
		return ERROR;
	}

	//Separate instruction type(instruction / pseudo instruction)
	for(int i=0; i<2; i++){
		
		//If exist symbol, save it
		if(i==1){
			strcpy(new_node->sym,tk_str[0]);
		}

		//instruction type is instruction
		if(opcode[i]!=-1){
			strcpy(new_node->inst, tk_str[i]);
			new_node->opcode = opcode[i];
			new_node->form = FindForm(tk_str[i])[0] - '0';
			new_node->type = INST;
			return INST;
		}

		//instruction type is pseudo instruction
		else if(pseudo[i]!=-1){
			strcpy(new_node->inst, tk_str[i]);
			new_node->type = PSEUDO_INST;
			return PSEUDO_INST;
		}
	}
	return 0;
}

int FindPseudoInstr(char* key){
	//If key is pseudo instruction, return pseudo number (START = 0,... )
	int i;	
	for(i=0; i < 7; i++){
		if(!strcmp(key, pseudo_instr[i]))
			return i;
	}

	return -1;
}



void InitAssemNode(){
	//Initailize assem_node
	assem_node *tmp_p, *del_p;

	tmp_p = assem_head;	
	while(1){	
		if(tmp_p == NULL) return;
		del_p = tmp_p;
		tmp_p = tmp_p->next;
		free(del_p);
	}

	assem_head = NULL;
	assem_rear = NULL;
}



int Type(char *file_name){
	//excution TYPE command 
	//file open and print contents
	FILE *fp = fopen(file_name,"r");
	char in;

	if(!fp){
		printf("File not found\n");
		return -1;
	}

	while( fscanf(fp,"%c",&in)!=EOF ){
		printf("%c",in);
	}

	fclose(fp);
	return 0;
}


