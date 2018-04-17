#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>

void encontrarCentinela(char* linea, char* auxiliar, int *i){
    while(linea[*i] != ' ' && linea[*i]!='\0'){
    	auxiliar[*i] = linea[*i];
    	(*i)++;
    }
    auxiliar[*i] = '\0';
    printf("%s",auxiliar);
}

void consolaPlanificador() {
  char * linea;
  char* comando;
  while(1) {
    linea = readline(">");
    if (!linea) {
      break;
    }
    comando=malloc(sizeof(linea));
    int i =0;
    encontrarCentinela(linea,comando,&i);
    if(!strcmp(comando,"continuar")){
    	printf("Usted ingreso continuar\n");
    }else if(!strcmp(comando,"pausar")){
    	printf("Usted ingreso pausar\n");
    }else if(!strcmp(comando,"bloquear")){
    	printf("Usted ingreso bloquear\n");
    }else if(!strcmp(comando,"desbloquear")){
    	printf("Usted ingreso desbloquear\n");
    }else if(!strcmp(comando,"listar")){
    	printf("Usted ingreso listar\n");
    }else if(!strcmp(comando,"kill")){
    	printf("Usted ingreso kill\n");
    }else if(!strcmp(comando,"status")){
    	printf("Usted ingreso status\n");
    }else if(!strcmp(comando,"deadlock")){
    	printf("Usted ingreso deadlock\n");
    }
    free(comando);
    free(linea);
  }
}
