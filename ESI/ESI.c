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
		   log_error(logger,"Error en la conexion con los clientes");
		   return 1;
	   }
	   log_info(logger,"Se realizo correctamente la conexion con el planificador y coordinador");
	   enviarTipoDeCliente(sockcoordinador,ESI);
	   enviar(sockcoordinador,pack);
	   log_destroy(logger);
	   close(sockplanificador);
	   close(sockcoordinador);
	   return 0;
}

int main(){
	logger =log_create(logESI,"ESI",1, LOG_LEVEL_INFO);
	Paquete pack;
	pack.a=SET;
	strcpy (pack.key,"MILLAVE");
	pack.value="MIVALOR";
	esi(pack);

	return 0;
}

