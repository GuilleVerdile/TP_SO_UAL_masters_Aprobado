#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>

void encontrarSentinela(char* linea, char* auxiliar, int *i){
    while(linea[*i] != ' ' && linea[*i]!='\0'){
    	auxiliar[*i] = linea[*i];
    	(*i)++;
    }
    auxiliar[*i] = '\0';
}

void main() {
  char * linea;
  while(1) {
    linea = readline(">");
    if (!linea) {
      break;
    }
    char* auxiliar;
    int i =0;
    encontrarSentinela(linea,auxiliar,&i);
    if(!strcmp(auxiliar,"continuar")){
    	printf("Usted ingreso continuar\n");
    }else if(!strcmp(auxiliar,"pausar")){
    	printf("Usted ingreso pausar\n");
    }else if(!strcmp(auxiliar,"bloquear")){
    	printf("Usted ingreso bloquear\n");
    }else if(!strcmp(auxiliar,"desbloquear")){
    	printf("Usted ingreso desbloquear\n");
    }else if(!strcmp(auxiliar,"listar")){
    	printf("Usted ingreso listar\n");
    }else if(!strcmp(auxiliar,"kill")){
    	printf("Usted ingreso kill\n");
    }else if(!strcmp(auxiliar,"status")){
    	printf("Usted ingreso status\n");
    }else if(!strcmp(auxiliar,"deadlock")){
    	printf("Usted ingreso deadlock\n");
    }
    free(linea);
  }
}

