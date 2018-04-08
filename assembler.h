/*
   +--------------------------------------------------------------+
   |                 System Programming Project 2                 |
   |                    ( SIC/XE Assembler )                      |
   |                                                              |
   |  File    : assembler.h                                       |
   |  Created : Mar 18 ~ Apr 9, 2018                              |
   |  Author  : Ye-eun Lee                                        |
   +--------------------------------------------------------------+
 */

/*initialization*/
void InitSymbolTable();
void InitAssemNode();

/*assemble*/
int Assemble(char *file_name);	//execute assembler
int IsAssemFile(char *file_name);	//check that input is '.asm' file
int AssemPass1(char *file_name);	//make assem_node and symbol table
int AssemPass2(char* file_name);	//make '.lst' and '.obj' file

/*Process '.asm' file and make assem_node*/
int MakeAssemNode(char tkstr[][MAX_LINESIZE]);		//make assem_node
int AssemToken(char *asm_line, char tk_str[][MAX_LINESIZE]);	//token string in '.asm' file
int GetType_and_SaveInst(assem_node *new_node, char tk_str[][MAX_LINESIZE]);//get instruction type and save istruction
void GetLoc(assem_node *new_node);	//get location
int GetObj(assem_node *cur_node);	//get object code


/*Function to find and get some value*/
int FindReg(char *str);		
char* FindForm(char* key);
int FindPseudoInstr(char* key);
int FindOpcode(char* key);

/*Type*/
int Type(char *file_name);
int GetOperand(assem_node *new_node, char tk_str[][MAX_LINESIZE]);
int GetPseudoOperand(assem_node *new_node, char tk_str[][MAX_LINESIZE]);

/*Symbol*/
int Symbol();
int SearchSymbol();
int MakeSymbolTable(assem_node *new_node);

/*Function to Process Hexadecimal and Decimal number*/
int HexToDec(char*hex);
int IsHex(char*ckstr);
int StrToDec(char* str);
