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

int cantidadDeArgumentos(char* linea, int cantidadNecesitada){
	int cantArgumentos = 0;
	for(int i = 0;i < strlen(linea);i++){
		if(linea[i] == ' '){
			cantArgumentos++;
		}
	}
	if(cantArgumentos != cantidadNecesitada){
		printf("Ingreso una cantidad invalida de argumentos\n");
		return 0;
	}
	return 1;
}


void consolaPlanificador() {
  char* linea;
  char* comando;
  char* id;
  char clave[40];
  while(1) {
    linea = readline(">");
    if (!linea) {
      break;
    }
    int i =0;
    comando=malloc(sizeof(linea));
    encontrarCentinela(linea,comando,&i);
    if(!strcmp(comando,"continuar")){
    	if(cantidadDeArgumentos(linea,0)){
    		printf("Usted ingreso continuar\n");
    	}
    }
    else if(!strcmp(comando,"pausar")){
    	if(cantidadDeArgumentos(linea,0)){
    		printf("Usted ingreso pausar\n");
    	}
    }
    else if(!strcmp(comando,"bloquear")){
    	if(cantidadDeArgumentos(linea,2)){
    		printf("Usted ingreso bloquear\n");
    		encontrarCentinela(linea,clave,&i);
    		printf("con la clave: %s\n", clave);
    		id = malloc(sizeof(linea));
    		encontrarCentinela(linea,id,&i);
    		printf("con el id: %s\n",id);
    		free(id);
    	}
    }
    else if(!strcmp(comando,"desbloquear")){
    	if(cantidadDeArgumentos(linea,1)){
    		printf("Usted ingreso desbloquear\n");
    		encontrarCentinela(linea,clave,&i);
    		printf("con la clave: %s\n", clave);
    	}
    }
    else if(!strcmp(comando,"listar")){
    	if(cantidadDeArgumentos(linea,0)){
    		printf("Usted ingreso listar\n");
    		id = malloc(sizeof(linea)); //ID = RECURSO EN ESTE CASO PARA EVITAR USAR MAS VARIABLES!
    		encontrarCentinela(linea,id,&i);
    		printf("con el recurso: %s\n",id);
    		free(id);
    	}
    }
    else if(!strcmp(comando,"kill")){
    	if(cantidadDeArgumentos(linea,1)){
    		printf("Usted ingreso kill\n");
    		id = malloc(sizeof(linea));
    		encontrarCentinela(linea,id,&i);
    		printf("con el id: %s\n",id);
    		free(id);
    	}
    }
    else if(!strcmp(comando,"status")){
    	if(cantidadDeArgumentos(linea,1)){
    		printf("Usted ingreso status\n");
    		encontrarCentinela(linea,clave,&i);
    		printf("con la clave %s\n",clave);
    	}
    }
    else if(!strcmp(comando,"deadlock")){
    	if(cantidadDeArgumentos(linea,0)){
    		printf("Usted ingreso deadlock\n");
    	}
    }else{
    	printf("No se reconocio el comando %s\n", comando);
    }

    free(comando);
    free(linea);
    }
}
