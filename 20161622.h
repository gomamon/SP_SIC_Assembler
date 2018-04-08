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

#define MAX_LINESIZE 120
#define MAX_FILENAME 55
#define MAX_ASM_TOKEN 7
#define PRINT_ERROR( line , c ) printf("line %d ERROR:: %s\n",line,c) 
#define OBJ_LINE_SIZE 30


//enum about valid command
enum COMMAND_TYPE {
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

/****/
enum ASSEM_TYPE{
	ERROR = -1,
	COMMENT = 0,
	PSEUDO_INST = 1,
	INST= 2

};
enum PSEUDO_INSTR_TYPE{
	START = 0,
	END,
	BASE,
	BYTE,
	WORD,
	RESB,
	RESW
};

enum ADDRESS_MODE{
	SIC = 0,
	IMMED = 1,
	INDIR,
	SIMPLE
};

char pseudo_instr[7][10]={
	"START","END","BASE","BYTE","WORD","RESB","RESW"
};
int pc_addr=0,base_addr=0;

typedef struct AssemNode{
	int t_flag;

	int line;
	int loc;
	int size;
	int type;
	int form;

	int addr_mode;

	unsigned obj;
	int opcode;
	char comment[55];
	char sym[12];
	char inst[12];
	char operand[2][12];
	
	struct AssemNode *next;
}assem_node;

assem_node *assem_head = NULL;
assem_node *assem_rear = NULL;

/**symbol**/
typedef struct SymbolNode{
	char sym[12];
	int loc;
	struct SymbolNode *next;
} symbol_node;


typedef struct SymbolTableNode{
	char alpha;
	struct SymbolNode *next;
} symbol_table_node;

symbol_table_node symbol_table[26];

////////////////////
int Symbol();
int AssemPass2(char* file_name);
void InitAssemNode();
int MakeAssemNode(char tkstr[][MAX_LINESIZE]);

int GetType_and_SaveInst(assem_node *new_node, char tk_str[][MAX_LINESIZE]);

char* FindForm(char* key);
int FindPseudoInstr(char* key);

int FindReg(char *str);
void GetLoc(assem_node *new_node);
int StrToDec(char* str);
int FindOpcode(char* key);
int GetObj(assem_node *cur_node);

int Assemble(char *file_name);
int Type(char *file_name);
int IsAssemFile(char *file_name);
int MakeSymbolTable(assem_node *new_node);
int GetOperand(assem_node *new_node, char tk_str[][MAX_LINESIZE]);
int GetPseudoOperand(assem_node *new_node, char tk_str[][MAX_LINESIZE]);
void InitSymbolTable();
int AssemPass1(char *file_name);
int AssemToken(char *asm_line, char tk_str[][MAX_LINESIZE]);
int SearchSymbol();
/////////////////////

//int AssemToken(char *asm_line, char **tk_str);
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
	int opcode;
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
void MakeOpcodeList(int opcode, char* mnemonic, char* mode);


/*Function to print according to format data in memory*/
void PrintData(int s, int e);
void PrintASCII(int s, int e, int addr);


/*ERROR FUNCTION*/
int IsAddrLimitERROR(int addr);
int IsAddrERROR(int s, int e);
int IsDataLimitERROR(int data);
void PrintCmdERROR();
