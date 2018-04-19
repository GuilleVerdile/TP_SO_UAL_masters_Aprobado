#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <commons/string.h>

int cantidadDeCentinelas(char** centinelas){
	int i = 0;
	while(centinelas[i]){
		i++;
	}
	return i;
}

int main() {
  char* linea;
  while(1) {
    linea = readline(">");
    if (!linea) {
      break;
    }
    int i =0;
    char** centinelas = string_split(linea," ");
    int n = cantidadDeCentinelas(centinelas);
    switch(n){
    	case 0:
    		break;
    	case 1:
    		if(!strcmp(centinelas[0],"continuar")){
    			printf("Usted ingreso continuar\n");
    		}
    		else if(!strcmp(centinelas[0],"pausar")){
        		printf("Usted ingreso pausar\n");
    		}
    	    else if(!strcmp(centinelas[0],"deadlock")){
    	    		printf("Usted ingreso deadlock\n");
    	    }
    	    else{
    	    	printf("No se reconocio el comando %s\n", centinelas[0]);
    	    }
    		break;
    	case 2:
	   		if(!strcmp(centinelas[0],"desbloquear")){
	    		printf("Usted ingreso desbloquear\n");
	    		printf("con la clave: %s\n", centinelas[1]);
	    	}
	   		else if(!strcmp(centinelas[0],"listar")){
	        	printf("Usted ingreso listar\n");
	        	printf("con el recurso: %s\n",centinelas[1]);
	        }
	   		else if(!strcmp(centinelas[0],"kill")){
	        	printf("Usted ingreso kill\n");
	        	printf("con el id: %s\n",centinelas[0]);
	        	}
	   	    else if(!strcmp(centinelas[0],"status")){
	   	    	printf("Usted ingreso status\n");
	   	    	printf("con la clave %s\n",centinelas[1]);
	   	    }
    	    else{
    	    	printf("No se reconocio el comando %s\n", centinelas[0]);
    	    }
    		break;
    	case 3:
			if(!strcmp(centinelas[0],"bloquear")){
		    	printf("Usted ingreso bloquear\n");
		    	printf("con la clave: %s\n", centinelas[1]);
		    	printf("con el id: %s\n",centinelas[2]);
			}
			else{
    	    	printf("No se reconocio el comando %s\n", centinelas[0]);
			}
    		break;
    	default:
    		printf("Usted ingreso una cantidad de argumentos invalida\n");
    }
    free(centinelas);
    free(linea);
  }
  return 0;
}
