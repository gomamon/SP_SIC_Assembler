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
int Assemble();
int Type();

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
			case ASSEMBLE:
				Assemble();
				break;
			case TYPE:
				Type();
				break;
			case SYMBOL:
				break;
		}
	}
}

int Assemble(){
	
}
int Type(){
	FILE *fp = fopen(par[0],"r");
	char in;
	
	if(!fp){
		printf("File not found\n");
		return -1;
	}
	while( fscanf(fp,"%c",&in)!=EOF ){
		printf("%c",in);
	}
	return 0;
}

/************************ Initialization ********************/
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

/*************** Make Hash Table to save OPCODE ************/
int MakeHashTable(){
	//Function to make Hash table
	//It read "opcode.txt" and save information to opcode list
	
	FILE *fp=fopen("opcode.txt","r");	//file pointer
	char in1[5],in2[10],in3[5];			//input from "opcode.txt"
	
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
	while( fscanf(fp,"%s %s %s",in1,in2,in3)!=EOF ){
		MakeOpcodeList(in1,in2,in3);
	}
	return 0;
}

void MakeOpcodeList(char* opcode, char*mnemonic, char* mode){
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
	strcpy( new->opcode, opcode);
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
			printf("[%s,%s]",tmp->mnemonic,tmp->opcode);	
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
			printf("opcode is %s\n",tmp->opcode);
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

