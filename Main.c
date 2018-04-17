/*
 * Main.c
 *
 *  Created on: 15 abr. 2018
 *      Author: utnso
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <commons/config.h>

#include "Config/config.h"
#include "Socket/Coordinador.h"
#include "Socket/Planificador.h"
#include "Socket/ESI.h"
#include "Socket/Coordinador.h"
#include "Socket/CoordinadorMultiple.h"
#include "Socket/Instancia.h"
#include "Consola/Consola.h"

int main(){
		 char *pathCoordinador="/home/utnso/git/tp-2018-1c-UAL-masters/Config/Coordinador.cfg";
		 char *pathPlanificador="/home/utnso/git/tp-2018-1c-UAL-masters/Config/Planificador.cfg";
		 char *pathCoordinadorMultiple="/home/utnso/git/tp-2018-1c-UAL-masters/Config/CoordinadorMultiple.cfg";
		 int opcion;
		 scanf("%i",&opcion);
		 switch(opcion){
		 case 1:
			 	 coordinador(pathCoordinador);
			 	 break;
		 case 2:
			 	 planificador(pathPlanificador);
			 	 break;
		 case 3:
			 	 coordinadormultiple(pathCoordinadorMultiple);
			 	 break;
		 case 4:
			 	 esi(pathCoordinador,pathPlanificador);
			 	 break;
		 case 5:
			 	 instancia(pathCoordinadorMultiple);
			 	 break;
		 }
		 while(1){}
		 return 0;
}
