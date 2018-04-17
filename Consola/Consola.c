#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>

void encontrarCentinela(char* linea, char* auxiliar, int *i){
	int j = 0;
    while(linea[*i] != ' ' && linea[*i]!='\0'){
    	auxiliar[j] = linea[*i];
    	(*i)++;
    	j++;
    }
    auxiliar[j] = '\0';
    (*i)++;
}

void main() {
  char* linea;
  char* comando;
  char* id;
  char clave[40];

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
    }
    else if(!strcmp(comando,"pausar")){
    	printf("Usted ingreso pausar\n");
    }
    else if(!strcmp(comando,"bloquear")){
    	printf("Usted ingreso bloquear\n");
    	encontrarCentinela(linea,clave,&i);
    	printf("con la clave: %s\n", clave);
    	id = malloc(sizeof(linea));
    	encontrarCentinela(linea,id,&i);
    	printf("con el id: %s\n",id);
    	free(id);
    }
    else if(!strcmp(comando,"desbloquear")){
    	printf("Usted ingreso desbloquear\n");
    	encontrarCentinela(linea,clave,&i);
    	printf("con la clave: %s\n", clave);
    }
    else if(!strcmp(comando,"listar")){
    	printf("Usted ingreso listar\n");
    	id = malloc(sizeof(linea)); //ID = RECURSO EN ESTE CASO PARA EVITAR USAR MAS VARIABLES!
    	encontrarCentinela(linea,id,&i);
    	printf("con el recurso: %s\n",id);
    	free(id);
    }
    else if(!strcmp(comando,"kill")){
    	printf("Usted ingreso kill\n");
    	id = malloc(sizeof(linea));
    	encontrarCentinela(linea,id,&i);
    	printf("con el id: %s\n",id);
    	free(id);
    }
    else if(!strcmp(comando,"status")){
    	printf("Usted ingreso status\n");
    	encontrarCentinela(linea,clave,&i);
    	printf("con la clave %s\n",clave);
    }
    else if(!strcmp(comando,"deadlock")){
    	printf("Usted ingreso deadlock\n");
    }
    free(comando);
    free(linea);
  }
