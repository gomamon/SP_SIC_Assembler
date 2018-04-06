#include "20161622.h"

/*
int IsAssemFile(){
	// Check that file type is '.asm' 
	// If filename in par[0] is '.asm' file, return 1
	// If not, return 0

	int check_asm=0;

	for(int i=0 ; i<strlen(par[0]) ; i++){
		if(par[0][i]=='.'){
			if(strlen(par[0])-i == 4){
				if(par[0][i+1]=='a' && par[0][i+2]=='s' && par[0][i+3]=='m') return 1;
				else return 0;
			}
			else return 0;
		}
	}
	if(check_asm == 4) return 1;

	return 0;

}

int Assemble(){
	FILE *fp;	
	if(!IsAssemFile()) return -1;
	fp = fopen(par[0], "r");

	if(fp==NULL){
		printf("%s not founded\n",par[0]);
	}


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
	fclose(fp);
	return 0;
}



*/
