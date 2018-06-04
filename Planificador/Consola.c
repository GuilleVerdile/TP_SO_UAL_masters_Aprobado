#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <commons/string.h>
#include <commons/log.h>
#include "Planificador.h"

int cantidadDeCentinelas(char** centinelas){
	int i = 0;
	while(centinelas[i]){
		i++;
	}
	return i;
}

int main() {
	pthread_t planificador;
	pthread_create(&planificador,NULL,planificador,NULL);
	t_log *logger=log_create("/home/utnso/git/tp-2018-1c-UAL-masters/Logs/Planificador.log","Consola",0, LOG_LEVEL_INFO);
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
    			log_info(logger, "Se ingreso comando continuar");
    		}
    		else if(!strcmp(centinelas[0],"pausar")){
        		printf("Usted ingreso pausar\n");
        		log_info(logger, "Se ingreso comando pausar");
    		}
    	    else if(!strcmp(centinelas[0],"deadlock")){
    	    		printf("Usted ingreso deadlock\n");
    	    		log_info(logger, "Se ingreso comando deadlock");
    	    }
    	    else{
    	    	printf("No se reconocio el comando %s\n", centinelas[0]);
    	    	log_error(logger, "No se reconocio el comando");
    	    }
    		break;
    	case 2:
	   		if(!strcmp(centinelas[0],"desbloquear")){
	    		printf("Usted ingreso desbloquear\n");
	    		printf("con la clave: %s\n", centinelas[1]);
	    		log_info(logger, "Se ingreso comando desbloquear");
	    		liberaClave(centinelas[1]);
	    	}
	   		else if(!strcmp(centinelas[0],"listar")){
	        	printf("Usted ingreso listar\n");
	        	printf("con el recurso: %s\n",centinelas[1]);
	        	log_info(logger, "Se ingreso comando listar");
	        	listar(centinelas[1]);
	        }
	   		else if(!strcmp(centinelas[0],"kill")){
	        	printf("Usted ingreso kill\n");
	        	printf("con el id: %s\n",centinelas[0]);
	        	log_info(logger, "Se ingreso comando Kill");
	        	}
	   	    else if(!strcmp(centinelas[0],"status")){
	   	    	printf("Usted ingreso status\n");
	   	    	printf("con la clave %s\n",centinelas[1]);
	   	    	log_info(logger, "Se ingreso comando status");
	   	    }
    	    else{
    	    	printf("No se reconocio el comando %s\n", centinelas[0]);
    	    	log_error(logger, "No se reconocio el comando");
    	    }
    		break;
    	case 3:
			if(!strcmp(centinelas[0],"bloquear")){
		    	printf("Usted ingreso bloquear\n");
		    	printf("con la clave: %s\n", centinelas[1]);
		    	printf("con el id: %s\n",centinelas[2]);
		    	log_info(logger, "Se ingreso comando bloquear");
		    	bloquearPorID(centinelas[1],transformarNumero(centinelas[2],0));
			}
			else{
    	    	printf("No se reconocio el comando %s\n", centinelas[0]);
    	    	log_error(logger, "No se reconocio el comando");
			}
    		break;
    	default:
    		printf("Usted ingreso una cantidad de argumentos invalida\n");
    		log_error(logger, "Cantidad de argumentos invalida");
    }
    log_destroy(logger);
    free(centinelas);
    free(linea);
  }
  return 0;
}
