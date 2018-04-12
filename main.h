#define COMMANDSIZE 256		//max command size	
#define MAX_PARAMETER 8		//max number of parameter 
#define MAX_MEMORY 1048576	//max memory size (0x0000~0xffff)
#define MAX_DATA 256		//max data size	(0x00~0xff)
#define HASH_SIZE 20		//hash size
#define HASH_MOD 20			//modular to get modual value
#define MAX_LINESIZE 120	//one line size
#define MAX_FILENAME 55		//file name size
#define MAX_ASM_TOKEN 7		//token nunber
#define PRINT_ERROR( line , c ) printf("line %d ERROR:: %s\n",line,c) //error macro
#define OBJ_LINE_SIZE 30	//objfile line max size

/**node to save History**/
typedef struct HistoryNode{
	char data[COMMANDSIZE];
	struct HistoryNode *next;
}his_node;

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

/*assemble node*/
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


