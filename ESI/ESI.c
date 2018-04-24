/*
 * Cliente.c
 *
 *  Created on: 15 abr. 2018
 *      Author: utnso
 */


#include "ESI.h"


int esi(Paquete pack){
		int sockplanificador=crearConexionCliente(pathPlanificador);
		int sockcoordinador=crearConexionCliente(pathCoordinador);
	   if(sockplanificador<0 || sockcoordinador<0){
		   printf("Error de conexion a los servidores\n");
		   return 1;
	   }
	   printf("Se crearon sockets cliente!");
	   printf("Se conecto a los 2 servidores\n");
	   enviar(sockcoordinador,pack);
		while(1){

		}
	   close(sockplanificador);
	   close(sockcoordinador);
	   return 0;
}

int main(){
	Paquete pack;
	pack.a=SET;
	strcpy (pack.key,"MILLAVE");
	pack.value="MIVALOR";
	esi(pack);

	return 0;
}

