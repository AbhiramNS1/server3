#include<stdio.h>
#include<string.h>
#include<stdlib.h>

#define LABEL 1
#define OPCODE 2
#define OPERAND 3

int isZero(char * str){
    char * tmp=str;
    while(*str){
        if(*str++!='0') return 0;
    }
    return 1;
}
void fill(char * buff,char * value,int num){
    int len=0,j=0,k=0;
    while(value[len]!='\0') len++;
    len=num-len;
    while(j++<len) buff[k++]='0';
    len=0;
    while(j++<num+1) buff[k++]=value[len++];
    buff[k]='\0';
}

struct Opcode{
    char code[5];
    char value[5];
};
struct Opcode opcodes[10];
int totalOpcodes=0;

char* search_opcode(char * opcode){
    for(int i=0;i<totalOpcodes;i++)
        if(!strcmp(opcodes[i].code,opcode))
             return opcodes[i].value;
    return NULL;
}
char* search_symbol(FILE *symtab,char * symbol){
    char c;
    int sy=1,i=0;
    char symb[100], addr[10];
    rewind(symtab);
    while((c=getc(symtab))>0){
        switch(c){
            case ' ':symb[i]='\0'; sy=i=0;break;
            case '\n':
                addr[i]='\0';sy=1;
                if(!strcmp(symbol,symb)){
                    char * address=malloc(5);
                    int j=0,k=4-i;
                    while(j<k) address[j++]='0';
                    k=0;
                    while(j<5) address[j++]=addr[k++];
                    return address;
                }
                i=0;
                break;
            default:
                if(sy) symb[i++]=c;
                else addr[i++]=c;
        }
    }
    return NULL;
}
int add_to_symtab(FILE *symtab,char * symbol,int address){
    char * s=search_symbol(symtab,symbol);
    if(s){
        free(s);
        return 0;
    }
    fprintf(symtab,"%s %d\n",symbol,address);
    return 1;
}

void load_opcodes(FILE * optab){
    char c;
    int i=0,j=0,op=1;
    while((c=getc(optab))>0){
        if(c=='\n'){
            opcodes[i].value[j-1]='\0';
            op=1;j=0;i++;
            totalOpcodes++;
            continue;
        }
        else if(c==' '){
            opcodes[i].code[j]='\0';
            j=op=0;
            continue;
        }
        if(op)
            opcodes[i].code[j++]=c;
        else 
            opcodes[i].value[j++]=c;
    }
    totalOpcodes++;
}

void main(){
    char c;
    int LOCCTR=0,index=0,lineno=1,START_ADDRESS=0,
    part=LABEL
    ;
    char label[10],opcode[10],operand[10];
   
    printf("SIC v1\n");
    FILE *prgm = fopen("main.asm","r"),
         *optab=fopen("OPTAB","r"),
         *intm=fopen("imm.txt","w+"),
         *debug=fopen("debug.txt","w"),
        *symtab=fopen("symtab","w+");

    fwrite("Line No.   Address    Label     Opcode      Operand\n\n",1,53,debug);
    load_opcodes(optab);

    while((c=getc(prgm))>0){
        if(c==' ' || c=='\n') continue;
        index=0;
        do{
            if(c=='\n' || c<0) {
                if(part==OPERAND){
                    operand[index]='\0';  //printf("operand(%s)\n",operand);
                    int loc=LOCCTR,write=1;
                    if(*label!='*') 
                       if(!add_to_symtab(symtab,label,LOCCTR))
                            printf("Duplicate label '%s' on line '%d' ",label,lineno);

                    if(search_opcode(opcode)){
                        
                                LOCCTR+=3;
                               
                    }
                    else{
                        int val=atoi(operand);
                        if(!isZero(operand) && !val) {
                            if(!strcmp(opcode,"BYTE")) LOCCTR+=(index+1);
                            else printf("--- Error 'Invalid operand \"%s\" with Opode \"%s\"' on line %d ",operand,opcode,lineno);
                        }
                        else if(!strcmp(opcode,"RESW")){ LOCCTR+=(val*3);write=0;}
                        else if(!strcmp(opcode,"RESB")) {LOCCTR+=val;write=0;}
                        else if(!strcmp(opcode,"WORD")) LOCCTR+=3;
                        else if(!strcmp(opcode,"BYTE")) LOCCTR+=(index+1);
                        else if(!strcmp(opcode,"START")) {START_ADDRESS=LOCCTR=val;write=0;}
                        else if(!strcmp(opcode,"END")) { sprintf(operand,"%d",START_ADDRESS);write=0; }
                        else  printf("\n --- Error 'Invalid opcode \"%s\"' on line %d ",opcode,lineno);
                    }
                    if(write) fprintf(intm,"%s   %s   %s\n",label,opcode,operand);
                    fprintf(debug,"   %d       %d       %s        %s        %s\n",lineno-1,loc,label,opcode,operand);
                }
                else{
                    printf("\n----Error in line no : %d  ' operand/opcode/label missing'-----\n",lineno);
                }
                part=LABEL;
                lineno++;
                
                break;
            }
            else if(c!=' '){
                switch(part){
                    case LABEL:label[index++]=c;break;
                    case OPCODE:opcode[index++]=c;break;
                    case OPERAND:operand[index++]=c;break;
                }
            }
            else{
                switch(part){
                    case LABEL:label[index]='\0';part=OPCODE;//printf("label(%s)",label);
                                break;
                    case OPCODE:opcode[index]='\0';part=OPERAND;//printf("opcode(%s)",opcode) ;
                                break;
                }
                break;
            }
        }while(c=getc(prgm));
    }

    fclose(prgm);
    rewind(intm);

    FILE *output=fopen("out.o","w");
    while((c=getc(intm))>0){
         if(c==' ' || c=='\n') continue;
        index=0;
        do{
            if(c=='\n' || c<0) {
                if(part==OPERAND){
                    operand[index]='\0';  //printf("operand(%s)\n",operand);
                    char * val=search_opcode(opcode);
                    if(val){
                        char * addr=search_symbol(symtab,operand);
                        if(addr){
                            fprintf(output,"%s%s\n",val,addr);
                            free(addr);
                        }
                        else{
                            printf("---Error invalid operand %s ",operand);
                        }
                        // else fprintf(output,"%s%s\n",val,operand);
                    }
                    else if(!strcmp(opcode,"WORD")){
                        char buff[10];
                        fill(buff,operand,6);
                        fprintf(output,"%s\n",buff);
                    }
                    else if(!strcmp(opcode,"BYTE")){
                       fprintf(output,"%s\n",operand);
                    }
                }
                part=LABEL;
                break;
            }
            else if(c!=' '){
                switch(part){
                    case LABEL:label[index++]=c;break;
                    case OPCODE:opcode[index++]=c;break;
                    case OPERAND:operand[index++]=c;break;
                }
            }
            else{
                switch(part){
                    case LABEL:label[index]='\0';part=OPCODE;// printf("label(%s)",label);
                                break;
                    case OPCODE:opcode[index]='\0';part=OPERAND; //printf("opcode(%s)",opcode) ;
                                break;
                }
                break;
            }
        }while(c=getc(intm));
    }
   
    fclose(output);
    
}
