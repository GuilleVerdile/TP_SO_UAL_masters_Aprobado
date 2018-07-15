#include "Consola.h"

int recorrerCentinela(char** centinelas,int liberar){
	int i = 0;
	while(centinelas[i]){
		if(liberar)
			free(centinelas[i]);
		i++;
	}
	return i;
}

void consola() {
  int pause = 0;
  char* linea;
  while(1) {
    linea = readline(">");
    if (!linea) {
      break;
    }
    int i =0;
    char** centinelas = string_split(linea," ");
    int n = recorrerCentinela(centinelas,0);
    switch(n){
    	case 0:
    		break;
    	case 1:
    		if(!strcmp(centinelas[0],"continuar")){
    			imprimir(magenta,"Usted ingreso continuar\n");
    			logTest("Se ingreso comando continuar",Blanco);
    			if(pause){
    			pthread_mutex_unlock(&mutex_pausa);
    			pause = 0;}
    		}
    		else if(!strcmp(centinelas[0],"pausar")){
    			imprimir(magenta,"Usted ingreso pausar\n");
        		logTest("Se ingreso comando pausar",Blanco);
        		if(!pause){
        			pthread_mutex_lock(&mutex_pausa);
        			pause = 1;}
    		}
    	    else if(!strcmp(centinelas[0],"deadlock")){
    	    	imprimir(magenta,"Usted ingreso Deadlock\n");
    	    	logTest("Se ingreso comando Deadlock",Blanco);
    	    	pthread_mutex_lock(&mutex_pausa);
    	    	algoritmoBanquero();
    	    	pthread_mutex_unlock(&mutex_pausa);
    	    }
    	    else{
    	    	imprimir(magenta,"No se reconocio el comando %s\n", centinelas[0]);
    	    	logTest("No se reconocion el comando",Blanco);
    	    }
    		break;
    	case 2:
	   		if(!strcmp(centinelas[0],"desbloquear")){
	    		imprimir(magenta,"Usted ingreso desbloquear\n");
	    		imprimir(magenta,"con la clave: %s\n", centinelas[1]);
	    		logTest("Se ingreso comando desbloquear",Blanco);
	    		liberaClave(centinelas[1]);
	    	}
	   		else if(!strcmp(centinelas[0],"listar")){
	   			imprimir(magenta,"Usted ingreso listar\n");
	   			imprimir(magenta,"con el recurso: %s\n",centinelas[1]);
	        	logTest("Se ingreso comando listar",Blanco);
	        	listar(centinelas[1]);
	        }
	   		else if(!strcmp(centinelas[0],"kill")){
	   			imprimir(magenta,"Usted ingreso kill\n");
	   			imprimir(magenta,"con el id: %s\n",centinelas[0]);
	   			logTest("Se ingreso comando Kill",Blanco);
	        	}
	   	    else if(!strcmp(centinelas[0],"status")){
	   	    	imprimir(magenta,"Usted ingreso status\n");
	   	    	imprimir(magenta,"con la clave %s\n",centinelas[1]);
	   	    	logTest("Se ingreso comando estatus",Blanco);
	   	    }
    	    else{
    	    	imprimir(amarillo,"No se reconocio el comando %s\n", centinelas[0]);
    	    	logTest("No se reconocio el comando",Blanco);
    	    }
    		break;
    	case 3:
			if(!strcmp(centinelas[0],"bloquear")){
				imprimir(magenta,"Usted ingreso bloquear\n");
				imprimir(magenta,"con la clave: %s\n", centinelas[1]);
				imprimir(magenta,"con el id: %s\n",centinelas[2]);
		    	logTest("Se ingreso comando bloquear",Blanco);
		    	bloquearPorConsola(centinelas[1],transformarNumero(centinelas[2],0));
			}
			else{
				imprimir(amarillo,"No se reconocio el comando %s\n", centinelas[0]);
    	    	logTest("No se reconocio el comando",Blanco);
			}
    		break;
    	default:
    		imprimir(rojo,"Usted ingreso una cantidad de argumentos invalida\n");
    		logTest("Cantidad de argumentos invalida",Blanco);
    }
    recorrerCentinela(centinelas,1);
    free(centinelas);
    free(linea);
  }
}
