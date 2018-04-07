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
		printf("mode: %d\n",mode);
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
			case ASSEMBLE:
				Assemble(par[0]);
				break;
			case TYPE:
				Type(par[0]);
				break;
			case SYMBOL:
				break;
		}
	}
}


int IsAssemFile(char *file_name){
	// Check that file type is '.asm' 
	// If filename in par[0] is '.asm' file, return 1
	// If not, return 0
	int i;
	int check_asm=0;

	for(i=0 ; i< strlen(file_name) ; i++){
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
	}
	fclose(fp);

	//InitAssemNode();
	InitAssemNode();

	if(AssemPass1(file_name)==-1) return -1;


}

int AssemPass1(char* file_name){
	int i;
	FILE *fp = fopen(file_name, "r");
	char asm_line[MAX_LINESIZE];
	char tk_str[MAX_ASM_TOKEN][MAX_LINESIZE] = {'\0'};
	int location;
	int pc;

	while(fgets(asm_line,MAX_LINESIZE,fp)!=NULL){
		AssemToken(asm_line, tk_str);
		printf("%s | %s | %s | %s \n",tk_str[0],tk_str[1],tk_str[2],tk_str[3]);
		MakeAssemNode(tk_str);
		/*
		if(!strcpy(tk_str[1], "START")){
		   location  = HexToDec(tk_str[2]);

		   if (location == -1){
		   printf("line %d ERROR: start address is not valid", assem_rear->line);
		   }
		   continue;
		//write listing line
		}
		else
		location = 0;
		*/
	}

}

int AssemToken(char asm_line[], char tk_str[][MAX_LINESIZE]){
	int i=0, j=0;
	int comment_flag=0;
	char *tk;
	char asm_str[MAX_LINESIZE] = {'\0'};


	for(i=0; i < strlen(asm_line) ;){
		if(asm_line[i]==','){
			i++;
			asm_str[j++] = ' ';
			asm_str[j++] = ',';
			asm_str[j++] = ' ';
			continue;
		}
		asm_str[j++]=asm_line[i++];
	}
	
	for(i=0; i<MAX_ASM_TOKEN; i++){
		tk_str[i][0] = '\0';
	}

	i=0;
	tk = strtok(asm_str, " \t\n");
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
}



int MakeAssemNode(char tk_str[][MAX_LINESIZE]){
	
	int type;


	assem_node *new_node;
	new_node = (assem_node*)malloc( sizeof(assem_node) );

	//node init;
	strcpy(new_node->comment, "\0");
	strcpy(new_node->sym, "\0");
	strcpy(new_node->inst, "\0");
	new_node->next = NULL;
	new_node->type = -1;
	new_node->line = (assem_rear==NULL) ?  5 : ( (assem_rear->line) + 5 );



	//link node
	if(assem_head==NULL){
		assem_rear = new_node;
		assem_head = new_node;
	}
	else{
		assem_rear->next = new_node;
		assem_rear = new_node;
	}
	type=GetType_and_SaveInst(new_node,tk_str);
	switch(type){
		case INST:
			return (GetOperand(new_node,tk_str)==-1) ? -1 : INST;
			break;
		case PSEUDO_INST:
			return (GetPseudoOperand(new_node,tk_str)==-1) ? -1 : PSEUDO_INST;
			break;
		default:
			return type;
			break;
	}
}
int GetPseudoOperand(assem_node *new_node, char tk_str[][MAX_LINESIZE]){
	int is_sym = (!strcmp(new_node->sym , "\0")) ? 0 : 1;
	switch(FindPseudoInstr(new_node->inst)){
		case START:
			if(IsHex(tk_str[1+is_sym])){
				pc_addr = HexToDec(tk_str[1+is_sym]);
				strcpy( new_node -> operand[0] , tk_str[1+is_sym]);
			}
			else{
				PRINT_ERROR(new_node->line, "Incorrect format!");
				return -1;
			}
			break;
		case END:
			break;
		case BASE:
			break;
		case BYTE:
			break;
		case WORD:
			break;
		case RESB:
			break;
		case RESW:
			break;
	}
}
int IsReg(char *c){
	int i;
	char reg[10][3] = {"A","X","L","B","S","T","F","PC","SW"};
	for(int i=0; i<9; i++){
		if(!strcmp(c, reg[i]) ){
			return 1;
		}
	}
	return 0;
}

int GetOperand(assem_node *new_node, char tk_str[][MAX_LINESIZE]){
	int i,j;
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
}

int GetType_and_SaveInst(assem_node *new_node, char tk_str[][MAX_LINESIZE]){
	//comment	
	int opcode[3];
	int pseudo[3];
	int error_flag=0;
	int i;
	if(tk_str[0][0]=='.'){
		new_node->type = COMMENT;
		strcpy(new_node->comment, tk_str[1]);
		return COMMENT;
	}
	
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
						strcpy(new_node->sym, &tk_str[i][1]);
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

	//symbol, instruction, opcode o.w
	
	for(int i=0; i<2; i++){
		opcode[i] = FindOpcode(tk_str[i]);
		pseudo[i] = FindPseudoInstr(tk_str[i]);
	}	
	
	//process error
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

	for(int i=0; i<2; i++){
		if(i==1){
			strcpy(new_node->sym,tk_str[0]);
		}
		if(opcode[i]!=-1){
			strcpy(new_node->inst, tk_str[i]);
			new_node->opcode = opcode[i];
			new_node->form = FindForm(tk_str[i])[0] - '0';
			new_node->type = INST;
			return INST;
		}
		else if(pseudo[i]!=-1){
			strcpy(new_node->inst, tk_str[i]);
			new_node->type = PSEUDO_INST;
			return PSEUDO_INST;
		}
	}
}

int FindOpcode(char* key){	
	opcode_node* tmp;	// opcode_list pointer for searching
	int hash_val=0,i;		// idx: hash table index, i:index

	strcpy(key, par[0]);	//key get mnemonic to find

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
	for(i=0; i < 6; i++){
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

