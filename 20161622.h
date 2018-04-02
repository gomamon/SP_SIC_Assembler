/*
+--------------------------------------------------------------+
|                 System Programming Project 1                 |
|                       ( SIC Shell )                          |
|                                                              |
|  File    : 20161622.h (header file)                          |
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
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdlib.h>


#define COMMANDSIZE 256		//max command size	
#define MAX_PARAMETER 8		//max number of parameter 
#define MAX_MEMORY 1048576	//max memory size (0x0000~0xffff)
#define MAX_DATA 256		//max data size	(0x00~0xff)
#define HASH_SIZE 20		//hash size
#define HASH_MOD 20			//modular to get modual value


//enum about valid command
enum COMMANDTYPE {
	H = 0,
	D,
	Q,
	HI,
	RESET,
	OPCODELIST,
	DU,
	E,
	F,
	OPCODEMNEMONIC,
	ASSEMBLE,
	TYPE,
	SYMBOL
};

char mem[ MAX_MEMORY ][3];				//memory to save data
char par[MAX_PARAMETER][COMMANDSIZE];	//parameter from input
char command[ COMMANDSIZE ];			//command to save in history node
int last_addr=-1;						//save the last address in dump


/*command list*/
char command_list[13][2][15]={
	{"h","help"},
	{"d","dir"},
	{"q", "quit"},
	{"hi","history"},
	{"reset","reset"},
	{"opcodelist","opcodelist"},
	{"du","dump"},
	{"e","edit"},
	{"f", "fill"},
	{"opcode","opcode"},
	{"assemble", "assemble"},
	{"type", "type"},
	{"symbol","symbol"}
};



/**node to save History**/
typedef struct HistoryNode{
	char data[COMMANDSIZE];
	struct HistoryNode *next;
}his_node;
his_node* his_head = NULL;
his_node* his_rear = NULL;



/*Node to save OpcodeList using hash table*/
typedef struct OpcodeNode{
	struct OpcodeNode* next;
	char opcode[5];
	char mnemonic[10];
	char form[5];
}opcode_node;

typedef struct{
	int size;
	opcode_node *head;
	opcode_node *rear;
}hash_table;

hash_table hash[20];


/* Function to execute Command */
void Help();
int Dir();
void History();
void Dump();
void Edit();
void Fill();
void Reset();
int Input();
int OpcodeMnemonic();
void Opcodelist();

/*Function to Initialize*/
void ParInit();
void MemInit();

/*Function to Free*/
void FreeHistory();
void FreeHash();

/*Function to save command history*/
void AddHistory();

/*Function to Process Hexadecimal */
int IsHex(char *ckstr );
int HexToDec(char *hex);


/*Function to check and process command */
int ProcessCommand(char* cmd);
int CheckParameter(int cmd_num);


/*Function to Make Opcode list using hash table*/
int MakeHashTable();
void MakeOpcodeList(char* opcode, char* mnemonic, char* mode);


/*Function to print according to format data in memory*/
void PrintData(int s, int e);
void PrintASCII(int s, int e, int addr);


/*ERROR FUNCTION*/
int IsAddrLimitERROR(int addr);
int IsAddrERROR(int s, int e);
int IsDataLimitERROR(int data);
void PrintCmdERROR();
