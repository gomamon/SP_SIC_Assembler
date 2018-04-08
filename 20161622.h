/*
   +--------------------------------------------------------------+
   |                 System Programming Project 2                 |
   |                    ( SIC/XE Assembler )                      |
   |                                                              |
   |  File    : 20161622.c                                        |
   |  Created : Mar 18 ~ Apr 9, 2018                              |
   |  Author  : Ye-eun Lee                                        |
   +--------------------------------------------------------------+
 */


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
