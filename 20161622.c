/*
   +--------------------------------------------------------------+
   |                 System Programming Project 2                 |
   |                       ( SIC Shell )                          |
   |                                                              |
   |  File    : 20161622.c                                        |
   |  Created : Mar 18 ~ Mar 24, 2018                             |
   |  Author  : Ye-eun Lee                                        |
   |  Purpose :                                                   |
   |     1. Implementation of shell to execute assemblers, links  |
   |        ,and loaders                                          |
   |     2. Implementation of memory space in which object code   |
   |        created by compilation is loaded and executed         |
   |     3. Implemettationof OPCODE table which converts mnemonic |
   |        of SIC / XE machine to opcode value                   |
   |                                                              |
   +--------------------------------------------------------------+
   */
#include "20161622.h"

int main(){
	int mode;		//execute mode

	MemInit();			//Initialize memory array
	MakeHashTable();	//Make hash table

	while(1){
		ParInit();		//Initialize parameter(par)

		//get input
		printf("sicsim> ");
		mode = Input();
		// execute the command according to mode
		if(mode!=-1)	AddHistory();	// If input is valid, add command to history 

		switch(mode){
			case -1:	// If input is invalid, get input again
				continue;
				break;
			case H:		// execute Help()
				Help();
				break;
			case D:		// execute Dir()
				Dir();
				break;
			case Q:		// memory free and exit the program
				FreeHistory();
				FreeHash();
				return 0;
				break;
			case HI:	// execute History()
				History();
				break;
			case DU:	// execute Dump()
				Dump();
				break;
			case E:		//execute Edit()
				Edit();
				break;
			case F:		//execute Fill()
				Fill();
				break;
			case RESET:	//execute Reset()
				Reset();
				break;
			case OPCODELIST:	//execute Opcodelist()
				Opcodelist();
				break;
			case OPCODEMNEMONIC:	//execute OpcodeMnemonic
				OpcodeMnemonic();
				break;
			case ASSEMBLE:		//excute Assemble
				Assemble(par[0]);
				break;
			case TYPE:		//excute Type
				Type(par[0]);
				break;
			case SYMBOL:	//excute Symbol
				Symbol();
				break;
		}
	}
}

int Symbol(){
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
		if(par[0][i]=='.'){
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



int Assemble(char *file_name){
	FILE *fp;

	//check file type and name
	if(!IsAssemFile(file_name)){
		printf("%s is not .asm file\n", file_name);
		return -1;
	}
	
	fp = fopen(par[0], "r");
	
	if(fp==NULL){
		printf("%s not founded\n",file_name);
		return -1;
	}

	fclose(fp);

	//InitAssemNode();
	InitAssemNode();
	InitSymbolTable();

	if(AssemPass1(file_name)==-1){
		InitAssemNode();
		InitSymbolTable();
		return -1;
	}

	if(AssemPass2(file_name)==-1){
		InitAssemNode();
		InitSymbolTable();
		return -1;
	}

	return 0;

}


int AssemPass2(char* file_name){
	//Make .lst file and .obj
	//
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
			if(GetObj(cur)==ERROR) return ERROR;
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
		if(!strcmp(cur->inst,"END")){
			break;
		}
		if(cur->size==0){
			continue;
		}
		if(cur->t_flag){
			for(tcnt=cur->next;tcnt!=NULL; tcnt=tcnt->next){
				if(tcnt->t_flag==1){
					t_next_loc = tcnt->loc;
					break;
				}
			}
			fprintf(object, "\nT%06X%02X",cur->loc,t_next_loc - (cur->loc) );
		}

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
	for(cur = assem_head->next; cur!=NULL ;cur = cur->next){
		if(!strcmp(cur->inst,"END")){		
			fprintf(object, "\nE%06X\n",assem_head->loc);
			break;
		}	
		if(cur->form == 4 
				&& ('0'>cur->operand[0][0]||cur->operand[0][0]>'9')){
			fprintf(object, "\nM%06X%02X",(cur->loc)-(assem_head->loc)+1,5);
		}
		
		
	}	
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
	else if(cur_node->type == INST){
		//Set object code accrding to the format
		switch(cur_node->form){
			case 1:
				cur_node->size = 1;
				cur_node->obj = cur_node->opcode;
				break;
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
					cur_node->obj += FindReg(cur_node->operand[1]);//Question
				}
				break;
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
				cur_node->size = 4;
				cur_node->obj = ((cur_node->opcode)+(cur_node->addr_mode))*16*16*16*16*16*16;
				cur_node->obj += 1*16*16*16*16*16;
				if(cur_node->addr_mode == SIMPLE){
					if(SearchSymbol(cur_node->operand[0])==-1){
						PRINT_ERROR(cur_node->line,"Referenced an undeclared symbol!");
						return -1;
					}
					cur_node->obj += SearchSymbol(cur_node->operand[0]);
				}
				else if(cur_node->addr_mode == IMMED){
					cur_node->obj += StrToDec(cur_node->operand[0]);
				}
				break;
		}
	}
	return 0;
}

int AssemPass1(char* file_name){
	FILE *fp = fopen(file_name, "r");
	char asm_line[MAX_LINESIZE];
	char tk_str[MAX_ASM_TOKEN][MAX_LINESIZE] = {'\0'};

	InitSymbolTable();
	while(fgets(asm_line,MAX_LINESIZE,fp)!=NULL){
		
		AssemToken(asm_line, tk_str);

		if (MakeAssemNode(tk_str) ) return ERROR;
		
		if(FindPseudoInstr(assem_rear->inst) == END) break;

	}
//	PrintList(asm_line);//DD
	fclose(fp);
	return 0;
}
void InitSymbolTable(){
	int i=0;

	for(i=0 ; i<26; i++){
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

		//process','
		i++;

	}while( (tk = strtok(NULL, " \t\n")) );
	return 0;
}



int MakeAssemNode(char tk_str[][MAX_LINESIZE]){
	
	int type;


	assem_node *new_node;
	new_node = (assem_node*)malloc( sizeof(assem_node) );

	//node init;
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
	int bef_loc;
	if(assem_rear == NULL){
		new_node->loc = pc_addr;
	}
	else{
		 bef_loc = assem_rear -> loc;;
		switch(assem_rear->type){
			case COMMENT:
				new_node->loc = bef_loc;
				break;
			case PSEUDO_INST:
				switch(FindPseudoInstr(assem_rear->inst)){
					case START:
					case BASE:
						new_node->loc = bef_loc;
						break;
					case BYTE:
						if(assem_rear->operand[0][0] == 'X')
							new_node->loc = bef_loc + (HexToDec(assem_rear->operand[1])/ 256 )+ 1;
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

			case INST:
				new_node->loc = bef_loc + assem_rear -> form;
				break;
				
		}
	}
}

int FindReg(char *str){
	int i;
	char reg[9][4] = {"A","X","L","B","S","T","F","PC","SW"};
	for(i=0; i<9; i++){
		if(!strcmp(str,reg[i])) return i;
	}
	return -1;
}

int MakeSymbolTable(assem_node *new_node){
	symbol_node *cur,*bef;
	symbol_node *new_sym;
	int start_flag=1;
	int i=1;

	if( !strcmp(new_node->sym, "\0") )	return 0;	
	else if( !strcmp(new_node->inst,"START") ) return 0;

	new_sym = (symbol_node*)malloc(sizeof(symbol_node));
	strcpy( new_sym -> sym , new_node->sym);
	new_sym -> loc =  new_node -> loc;
		
	if(symbol_table[ (new_node->sym[0]) - 'A' ].next == NULL ){
		symbol_table[ ((new_node->sym)[0]) - 'A' ].next = new_sym;
		return 0;
	}


	//check 
	if(SearchSymbol( new_node->sym)!=-1){
		PRINT_ERROR(new_node->line , "Overlap symbols");
		return ERROR;
	}

	bef = symbol_table[ (new_node->sym)[0] - 'A' ].next; 
	for(cur = symbol_table[ (new_node->sym)[0] - 'A' ].next; cur!=NULL; ){
			
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
			
		else if( new_sym->sym[i]== cur->sym[i] ){				
			i++;
		}
		else{
			bef = cur;
			cur = cur->next;
		}
		start_flag = 1;
	}
	if(cur == NULL){
		new_sym->next = NULL;
		bef->next = new_sym; 
	}
	return 0;	
}

int SearchSymbol(char *key){
	//Find : return addr
	//unfind : -1
	symbol_node *cur;

	if(symbol_table[ key[0]-'A'].next==NULL)
		return -1;

	for(cur = symbol_table[ key[0]-'A' ].next; cur!=NULL; ){
		if(!strcmp(cur->sym, key))	return cur->loc;
		cur = cur->next;
	}
	return -1;
}

int GetPseudoOperand(assem_node *new_node, char tk_str[][MAX_LINESIZE]){
	int i;
	int is_sym = (!strcmp(new_node->sym , "\0")) ? 0 : 1;
	
	switch(FindPseudoInstr(new_node->inst)){
		case START:
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
	int is_sym = (!strcmp(new_node->sym , "\0")) ? 0 : 1;
	char oper2_except[4][3][10] = {
		{"CLEAR","TIXR"},//r1
		{"CVC","CVC"},	//n
		{"SHIFTL", "SHIFTR"}//r1, n
	};

	switch(new_node->form){
		case 1:
			if(strcmp(tk_str[1 + is_sym],"\0")){
				PRINT_ERROR(new_node->line, "Incorrect format!");
				return ERROR;
			}
			return 0;
			break;
		case 2:
			for(i=0;i<2;i++){
				if( ( !strcmp(oper2_except[i][0],new_node->inst) )
						|| (!strcmp(oper2_except[i][1],new_node->inst)) ){
					
					switch(i){
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
			
			if( (!strcmp(new_node->inst, "RSUB")) && (!strcmp(tk_str[1+is_sym],"\0")))
				return 0;
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
	//comment	
	int opcode[3];
	int pseudo[3];
	int i;
	if(tk_str[0][0]=='.'){
		new_node->type = COMMENT;
		strcpy(new_node->comment, tk_str[1]);
		return COMMENT;
	}
	
	//printf("%s | %s | %s | %s \n",tk_str[0],tk_str[1],tk_str[2],tk_str[3]);
	//save symbol, instruction, opcode in format 4;
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

int FindOpcode(char* key){	
	opcode_node* tmp;	// opcode_list pointer for searching
	int hash_val=0,i;		// idx: hash table index, i:index
	//get hash value
	for( i=0; i<(int)strlen(key) ; i++)	
		hash_val += key[i];
	hash_val %= HASH_MOD;

	for(tmp = hash[hash_val].head; tmp!=NULL; tmp = tmp->next){
		if(!strcmp( key, tmp->mnemonic)) {
			return tmp->opcode;
		}
	}	

	//If you can't find the opcode, return -1
	return -1;
}
char* FindForm(char* key){
	opcode_node* tmp;	// opcode_list pointer for searching
	int hash_val=0,i;		// idx: hash table index, i:index

	//get hash value
	for( i=0; i<(int)strlen(key) ; i++)	
		hash_val += key[i];
	hash_val %= HASH_MOD;

	for(tmp = hash[hash_val].head; tmp!=NULL; tmp = tmp->next){
		if(!strcmp( key, tmp->mnemonic)) {
			return tmp->form;
		}
	}	

	//If you can't find the opcode, return -1
	return NULL;
}
int FindPseudoInstr(char* key){
	int i;	
	for(i=0; i < 7; i++){
		if(!strcmp(key, pseudo_instr[i]))
			return i;
	}

	return -1;
}



void InitAssemNode(){
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
	FILE *fp = fopen(par[0],"r");
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




/*********************** Initialization ********************/
void ParInit(){
	//Initialize parameter to '\0'	

	int i;

	for( i=0;i<MAX_PARAMETER ; i++){
		par[i][0] = '\0';	// make string(par[i]) '\0'
	}

}

void MemInit(){
	//Initialize memory to "00"

	int i=0;
	char tmp_str[]="00"; //string to copy

	for(i=0; i<MAX_MEMORY ; i++)
		strcpy(mem[i],tmp_str); //make mem[i] "00"
}

/************ Process and check Input *****************/

int Input(){
	//Get input to process
	//And if the input match the format, return command value to exeicute
	//If not, return -1

	char ch_in; 	//input char
	int i=0,j=0;	//i:index for cmd, j:index for command	
	char cmd[COMMANDSIZE];	//cmd is string to process command efficiently 
	//command is global variable to be saved in history

	//get input
	while(1){
		scanf("%c",&ch_in);	//get input
		if( ch_in == '\n')	break;

		//To process ',', add space on both side and save in cmd
		if( ch_in == ','){	
			cmd[i++] = ' '; 
			cmd[i++] = ','; 
			cmd[i++] = ' ';
			command[j++] = ch_in;	//command doesn't not space
		}

		//save charater
		else{	
			cmd[i++] = ch_in;
			command[j++] = ch_in;
		}

		//if input is too long, then print error massage and return -1
		if(i>=COMMANDSIZE-1){
			PrintCmdERROR();	
			return -1;
		}
	}

	//add '\0' to end of input
	cmd[i] = '\0';
	command[j] = '\0';

	//return processed Command value
	return ProcessCommand(cmd);

}


int ProcessCommand(char* cmd){
	//Tokenize cmd and separate the command and parameter 
	//If command is valied, return command value
	//If not, return -1

	int i, j;
	char *tk; //string token
	char tmp[COMMANDSIZE];

	//Input Command Token
	//and classify command and parameter
	i=-1;
	strcpy(tmp, cmd);

	tk = strtok(tmp, " \t");
	if(!tk) return -1;
	do{
		//classify command and parameter
		if(i==-1) strcpy(cmd, tk);
		else strcpy(par[i],tk);
		i++;

		//If paramater is too much, print error message return -1
		if(i>=MAX_PARAMETER){
			PrintCmdERROR();	
			return -1;
		}
	}while( (tk = strtok(NULL, " \t") ) );	

	//If there is no input, return -1
	if(cmd[0] == '\0') return -1;		

	//Check command {H = 0, D = 1, q = 2, ...  ,TYPE = 12 }
	//If it matches format, return the enum value of the cmd. If not, return -1;
	for(i=0; i<13 ;i++)
		for(j=0 ; j<2 ; j++)
			if(!strcmp(cmd, command_list[i][j]))
				return CheckParameter(i);

	//If commad is invalid, return -1
	PrintCmdERROR();	
	return -1;
}

int CheckParameter(int cmd_num){
	//Check that parameter according to com_num is valid
	//If parameter is valid, return cmd_num
	//else, return -1

	switch(cmd_num){

		//command without parameter
		case H:
		case D:
		case Q:
		case HI:
		case RESET:
		case OPCODELIST:
		case SYMBOL:
			if(par[0][0]=='\0')
				return cmd_num;

			PrintCmdERROR();	
			return -1;
			break;


			//Check parameter of DUMP
		case DU:
			//If there is no parameter, return cmd_num
			if(par[0][0] == '\0')
				return cmd_num;

			//If there is parameter, check them
			else if( IsHex(par[0]) ){	//check first parameter is hexadecimal
				//Check the range exceed, return -1
				if( IsAddrLimitERROR( HexToDec(par[0]) ) )					
					return -1;	

				//If there is valid one parameter, return comd_num 
				if(par[1][0]=='\0') 
					return cmd_num;

				//Check the parameter valid if there is more than 1
				else if( par[1][0] == ',' && IsHex(par[2]) && par[3][0]=='\0'){
					if( IsAddrLimitERROR( HexToDec(par[2]) ) ) return -1;	//check limit error
					if( IsAddrERROR( HexToDec(par[0]) ,HexToDec(par[2]) ) )	return -1;	//check address error
					return cmd_num;	
				}
			}

			//Print error message and return -1 if parameter is invalid.
			PrintCmdERROR();	
			return -1;
			break;


			//Check parameter of EDIT
		case E:

			//Check parameter format
			if(IsHex(par[0]) && par[1][0] ==',' && IsHex(par[2]) && par[3][0] == '\0'){
				if(IsAddrLimitERROR( HexToDec(par[0]) ))return -1;	//check limit error
				if( IsDataLimitERROR( HexToDec(par[2]) ) )return -1;	//check limit error
				return E;
			}

			//Print error message and return -1 if parameter is invalid
			PrintCmdERROR();	
			return -1;
			break;


			//Check parameter of FILL
		case F:
			if(IsHex(par[0]) && par[1][0] ==',' && IsHex(par[2]) 
					&& par[3][0] ==',' && IsHex(par[4]) && par[5][0]=='\0'){

				if(IsAddrLimitERROR( HexToDec(par[0]) ))return -1;
				if( IsAddrLimitERROR( HexToDec(par[2]) ) )return -1;
				if( IsAddrERROR( HexToDec(par[0]), HexToDec(par[2]) ) )return -1;
				if( IsDataLimitERROR( HexToDec(par[4]) ) )	return -1;
				return F;
			}
			PrintCmdERROR();	
			return -1;
			break;

			//Check parameter of OPCODEMNEMONIC
		case OPCODEMNEMONIC:

			//return -1 if no input
			if(par[0][0]=='\0'){
				printf("No input mnemonic\n");
				return -1;
			}

			//Check the parameter format
			else if(par[1][0]=='\0')
				return OPCODEMNEMONIC; 

			//Print error message and return -1 if parameter is invlaid
			PrintCmdERROR();	
			return -1;
			break;

		case ASSEMBLE:
			if(par[0][0]=='\0'){
				printf("No input file name\n");
				return -1;
			}
			else if(par[1][0]=='\0')
				return ASSEMBLE;
			PrintCmdERROR();
			return -1;
			break;

		case TYPE:
			if(par[0][0]=='\0'){
				printf("No input file name\n");
				return -1;
			}
			else if(par[1][0]=='\0')
				return TYPE;
			PrintCmdERROR();
			return -1;
			break;


			//Print error message and return -1 if parameter is invlaid
		default:
			PrintCmdERROR();	
			return -1;
			break;
	}
}


/*************** Make Hash Table to save OPCODE ************/
int MakeHashTable(){
	//Function to make Hash table
	//It read "opcode.txt" and save information to opcode list

	FILE *fp=fopen("opcode.txt","r");	//file pointer
	char in2[10],in3[5];			//input from "opcode.txt"
	int in1;
	// If file pointer is NULL, print error message and return -1
	if(!fp)	{		
		printf("No \"opcode.txt\"\n");
		return -1;
	}

	// initialize hash table
	for(int i=0; i<HASH_SIZE ; i++){
		hash[i].size = 0;
		hash[i].head = NULL;
		hash[i].rear = NULL;
	}

	// Opcode nodes store input from "opcode.txt"
	while( fscanf(fp,"%X %s %s",&in1,in2,in3)!=EOF ){
		MakeOpcodeList(in1,in2,in3);
	}
	return 0;
}

void MakeOpcodeList(int opcode, char*mnemonic, char* mode){
	// Make opcode node and Save opcode information using hashing
	// Hash value : add up the ASCII code of mnemonic characters and modular 20

	int i;
	int hash_val=0;
	opcode_node* new = (opcode_node*)malloc(sizeof(opcode_node));	//Allocate memory for opcode node

	//get hash value;
	for(i=0 ; i<(int)strlen(mnemonic) ; i++)
		hash_val += mnemonic[i];		// add up the ASCII code of mnemonic		
	hash_val%=HASH_MOD;					// modular 20

	//save the information of the opcode
	new->opcode = opcode;
	strcpy( (new->mnemonic), mnemonic);
	strcpy( new->form , mode);
	new->next = NULL;

	//link the node to hash table according to hash_val
	hash[hash_val].size++;
	if(hash[hash_val].head==NULL){
		hash[hash_val].head = new;
		hash[hash_val].rear = new;
	}
	else{
		hash[hash_val].rear->next = new;
		hash[hash_val].rear = new;
	}
}

/*************** Save COMMAND History ***********/
void AddHistory(){
	//Save the command hisotry to history node

	his_node *new = (his_node*)malloc(sizeof(his_node));	//allocate memory for his_node

	//save command to history node
	strcpy(new->data, command);		
	new->next = NULL;

	//link the node
	if(his_head == NULL){
		his_head = new;
		his_rear = new;
	}
	else{
		his_rear->next = new;
		his_rear = new;
	}

}

/*************** Memeory Free **************/
void FreeHistory(){
	//Memory free related to history node

	his_node *tmp_p,*del_p;

	if(his_head==NULL) return;
	else{
		tmp_p = his_head;	//find head node
		while(1){	//free his_node
			if(tmp_p==NULL) return;
			del_p = tmp_p;
			tmp_p = tmp_p->next;
			free(del_p);
		}
	}
}
void FreeHash(){
	//Memroy free related to opcode node 

	opcode_node *tmp_p, *del_p;

	for(int i=0 ;i<HASH_SIZE ; i++){	
		tmp_p = hash[i].head;	//find head opcode_node linked to hash table
		while(1){	//free opcode_node 
			if(tmp_p == NULL) return;
			del_p = tmp_p;
			tmp_p = tmp_p->next;
			free(del_p);
		}
	}
}



/****************** Help *****************/
void Help(){
	//Print all valid command

	printf("h[elp]\n"
			"d[ir]\n"
			"q[uit]\n"
			"hi[story]\n"
			"du[mp] [start, end]\n"
			"e[dit] address, value\n"
			"f[ill] start, end, value\n"
			"reset\n"
			"opcode mnemonic\n"
			"opcodelist\n"
			"assemble filename\n"
			"type filename\n"
			"symbol\n"
		  );
}

/***************** Dir *****************/
int Dir(){
	//Function to print file information in current directory

	struct dirent *dir_ent;		//directory entry pointer
	struct stat dir_stat;		//directory 
	DIR* dir_p = opendir("."); 	//directory pointer

	if(dir_p== NULL) return -1;

	//read file information
	while( ( dir_ent = readdir(dir_p) ) ){
		stat(dir_ent->d_name, &dir_stat);

		//classify and print file
		if(S_ISDIR(dir_stat.st_mode))
			printf("%s/\n",dir_ent->d_name);
		else if(dir_stat.st_mode & S_IXUSR)
			printf("%s*\n",dir_ent->d_name);
		else
			printf("%s\n",dir_ent->d_name);
	}
	return 0;
}

void History(){
	//Function to print history 

	his_node* hp;	//his_node pointer
	int cnt=0;		//count the line number to print

	//no history
	if(his_head==NULL) return;	

	//print history
	for(hp = his_head ; hp!= NULL ;hp = hp->next){
		printf("%-3d %s\n",++cnt,hp->data);
	}		
}


/******************** Dump *************************/
void Dump(){
	//Function to process e and s
	//and print data in memory form s to e

	int s=0, e=0;		//decimal value of start and end
	char start[10], end[10];	//hex value of start and end

	//move the parameter to start and end
	strcpy(start, par[0]);
	strcpy(end, par[2]);

	//If no input about start, set s and e toprint 160 from last address
	if(start[0] == '\0'){	

		//Is last address is MEX_MEMORY-1, set s is 
		if(last_addr == MAX_MEMORY-1) s = 0;

		else s = last_addr+1;
		e = s+159;
	}

	//If exist input about start
	else{
		s = HexToDec(start); //get decimal value s from start

		//If only parameter about start, e is s+159
		if(end[0]=='\0') 
			e = s+159; 

		else	e = HexToDec(end); //get decimal value e from end
	}

	//If e is excced the MAX_MEMROY, reset e
	if(e>= MAX_MEMORY){
		e = MAX_MEMORY-1;
	}

	//Print data in memory form s to e
	PrintData(s,e);

	//save last address
	last_addr = e;
}

void PrintData(int s,int e){
	//Function to print according to format the data in memory(mem) at address(addr)

	int i,j;

	//print from s to e
	for(i=s ;i<=e; i++){

		//if i is start index print data 
		if(i==s){
			printf("%05X ",i);
			if(i%16!=0){
				for(j=0 ;j<i%16 ; j++)
					printf("   ");
			}

			printf("%02X ",HexToDec(mem[i]));

			if(i%16==15) PrintASCII(s,e,i-i%16);
			continue;
		}

		//print data
		if(i%16==0)	printf("%05X ",i);
		printf("%02X ",HexToDec(mem[i]));

		if(i%16==15) PrintASCII(s,e,i-i%16);
	}

	//print rest data to according to format
	if(e%16!=15){ 
		for(; i<e-e%16+16; i++)
			printf("   ");
		PrintASCII(s,e,e-e%16);
	}	
}
void PrintASCII(int s, int e, int addr ){
	// Function outputs  ASCII data of the line within addr.
	// consider s and e to the fit format.

	int i;		
	int dec;	//decimal value of the data


	printf("; ");

	for( i=addr; i < addr+16 ; i++){
		dec = HexToDec(mem[i]); //get decimal value

		//if 'dec: 20~7E' and 'i : s~e' print ASCII
		if( (32<=dec && dec<=126) && (s<=i&&i<=e) )	
			printf("%c",dec);
		//if not print "."
		else	 printf(".");
	}
	puts("");

}


/************************** Edit ************************/

void Edit(){
	// Function to modify data in the memory
	char val[8], addr[8];

	//get addr information and val information from par
	strcpy(addr, par[0]);
	strcpy(val, par[2]);

	//modify data in mem at addr to val
	strcpy(mem[HexToDec(addr)],val);
}


/************************ Fill *************************/

void Fill(){
	//Function to fill from s(start) ~ e(end)

	int i;
	int s,e;	//decimal number of start and end	
	char start[8],end[8],value[8];	

	//get start, end, value from parameter
	strcpy(start, par[0]);
	strcpy(end, par[2]);
	strcpy(value, par[4]);

	//convert to decimal number
	s = HexToDec(start);
	e = HexToDec(end);

	//Change data to value from s to e
	for(i=s; i<=e ; i++){
		strcpy(mem[i],value);
	}
}


/*************************** Reset ****************************/
void Reset(){
	//Function to fill all memory "00"

	int i=0;
	char tmpstr[] = "00";	//string to copy

	//Make all data in memory "00"
	for(i=0 ; i<MAX_MEMORY ; i++){
		strcpy(mem[i],tmpstr);
	}
}


/************************** OPCODE ********************************/
void Opcodelist(){
	//Function to print opcode list

	int i=0,flag=0; //i:index , flag: if flag is 1, print '->'
	opcode_node* tmp; //opcode_list pointer for searching


	for(i=0 ; i<HASH_MOD ; i++){
		printf("%d : ",i);	//print hash number in hash table
		flag = 0;	//initialize flag to be 0

		//Check the hash is empty
		if(hash[i].head == NULL) {
			printf("empty\n");
			continue;
		}

		//Print data in opcode_node connected to the hash 
		for(tmp = hash[i].head; tmp!=NULL ; tmp=tmp->next){
			if(flag) printf(" -> ");
			else flag = 1;
			printf("[%s,%X]",tmp->mnemonic,tmp->opcode);	
		}
		puts("");
	}
}



int OpcodeMnemonic(){	
	//Function to find opcode.
	//if you find the opcode print and return 1.
	//if the opcode is not exist, print error message and return 0.

	opcode_node* tmp;	// opcode_list pointer for searching
	char key[10];		// mnemonic of the opcode you want to obtain
	int hash_val=0,i;		// idx: hash table index, i:index

	strcpy(key, par[0]);	//key get mnemonic to find

	//get hash value
	for( i=0; i<(int)strlen(key) ; i++)	
		hash_val += key[i];
	hash_val %= HASH_MOD;

	//find opcode using hash value
	for(tmp = hash[hash_val].head; tmp!=NULL; tmp = tmp->next){
		if(!strcmp( key, tmp->mnemonic)) {
			printf("opcode is %X\n",tmp->opcode);
			return 1;
		}
	}	

	//If you can't find the opcode, return -1
	printf("OPCODE not found\n");
	return -1;
}


/********************** Processing Hexadecimal *******************/

int HexToDec(char* hex){
	//Function to convert Hexadecimal to Decimal

	int i;
	int tmp, dec=0;

	for(i= 0 ; i< (int)strlen(hex); i++){
		if(hex[i]=='\0') break; //if the char is '\0', then break

		tmp=hex[i]; //get char in hex[i]

		//save number to tmp using ASCII value
		if('a'<=tmp && tmp<='f') tmp = tmp - 'a' + 10;
		else if('A'<=tmp && tmp<='F') tmp = tmp - 'A' +10;
		else if('0'<=tmp && tmp<= '9') tmp -= '0';

		//calculate hexadecimal vale to decimal value
		dec *= 16;
		dec += tmp;
	}

	//return decimal value
	return dec;
}

int IsHex( char *ckstr ){
	//Check that the string(ckstr) is Hexadecimal
	//if ckstr is Hexadecimal, return 1
	//if not, return 0

	int i;

	//Check that ckstr is empty
	if(ckstr[0] == '\0') return 0;

	for(i=0; i<(int)strlen(ckstr); i++){

		//check the character in ckstr[i]
		if(ckstr[i]<'0' || (ckstr[i] > '9' && ckstr[i] <'A') || (ckstr[i]>'F'&&ckstr[i]<'a') || ckstr[i]>'f')
			return 0;
	}

	return 1;
}

/******************** Check ERROR / Print ERROR MESSAGE **********************/

void PrintCmdERROR(){
	//Function to print error message about invalid command

	printf("Unknown Command!\n");
}


int IsDataLimitERROR(int data){
	//function to check data limit
	//if data exceed limit(0x00 ~ 0xff), then return 1
	//if not, return 0

	if(data<MAX_DATA)
		return 0;
	else{
		printf("Exceede limit!\n");
		return 1;
	}
}

int IsAddrERROR(int s, int e){
	//Function to check start address and end address
	//If s is bigger than e, then return 1
	//If not, return 0

	if( e < s){
		printf("End address shouldn't be less than Start address!\n");
		return 1;
	}
	return 0;
}

int IsAddrLimitERROR(int addr){
	//function to check adrress limit
	//if adddr exceed limit(0x00000 ~ 0xfffff), then return 1
	//if not, return 0

	if(addr < MAX_MEMORY)
		return 0;
	else{
		printf("Exceeded limit!\n");
		return 1; 
	}
}

