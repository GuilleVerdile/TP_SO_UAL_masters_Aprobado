#include "Consola.h"

int cantidadDeCentinelas(char** centinelas){
	int i = 0;
	while(centinelas[i]){
		i++;
	}
	return i;
}

void consola() {
	t_log *log_consola;
	log_consola =log_create(logPlanificador,"consola",0, LOG_LEVEL_INFO);
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
    			log_info(log_consola, "Se ingreso comando continuar");
    		}
    		else if(!strcmp(centinelas[0],"pausar")){
        		printf("Usted ingreso pausar\n");
        		log_info(log_consola, "Se ingreso comando pausar");
    		}
    	    else if(!strcmp(centinelas[0],"deadlock")){
    	    		printf("Usted ingreso deadlock\n");
    	    		log_info(log_consola, "Se ingreso comando deadlock");
    	    }
    	    else{
    	    	printf("No se reconocio el comando %s\n", centinelas[0]);
    	    	log_error(log_consola, "No se reconocio el comando");
    	    }
    		break;
    	case 2:
	   		if(!strcmp(centinelas[0],"desbloquear")){
	    		printf("Usted ingreso desbloquear\n");
	    		printf("con la clave: %s\n", centinelas[1]);
	    		log_info(log_consola, "Se ingreso comando desbloquear");
	    		liberaClave(centinelas[1]);
	    	}
	   		else if(!strcmp(centinelas[0],"listar")){
	        	printf("Usted ingreso listar\n");
	        	printf("con el recurso: %s\n",centinelas[1]);
	        	log_info(log_consola, "Se ingreso comando listar");
	        	listar(centinelas[1]);
	        }
	   		else if(!strcmp(centinelas[0],"kill")){
	        	printf("Usted ingreso kill\n");
	        	printf("con el id: %s\n",centinelas[0]);
	        	log_info(log_consola, "Se ingreso comando Kill");
	        	}
	   	    else if(!strcmp(centinelas[0],"status")){
	   	    	printf("Usted ingreso status\n");
	   	    	printf("con la clave %s\n",centinelas[1]);
	   	    	log_info(log_consola, "Se ingreso comando status");
	   	    }
    	    else{
    	    	printf("No se reconocio el comando %s\n", centinelas[0]);
    	    	log_error(log_consola, "No se reconocio el comando");
    	    }
    		break;
    	case 3:
			if(!strcmp(centinelas[0],"bloquear")){
		    	printf("Usted ingreso bloquear\n");
		    	printf("con la clave: %s\n", centinelas[1]);
		    	printf("con el id: %s\n",centinelas[2]);
		    	log_info(log_consola, "Se ingreso comando bloquear");
		    	bloquearPorID(centinelas[1],transformarNumero(centinelas[2],0));
			}
			else{
    	    	printf("No se reconocio el comando %s\n", centinelas[0]);
    	    	log_error(log_consola, "No se reconocio el comando");
			}
    		break;
    	default:
    		printf("Usted ingreso una cantidad de argumentos invalida\n");
    		log_error(log_consola, "Cantidad de argumentos invalida");
    }
    log_destroy(log_consola);
    free(centinelas);
    free(linea);
  }
}
