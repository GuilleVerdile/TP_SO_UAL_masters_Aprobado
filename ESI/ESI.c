/*
 * Cliente.c
 *
 *  Created on: 15 abr. 2018
 *      Author: utnso
 */


#include "ESI.h"


int esi(char* path){
		FILE* f;
		size_t length = 0;
		ssize_t read;
		char* linea;
		f = fopen(path,"r");
		if(f == NULL){
			log_error(logger, "No se pudo abrir el archivo");
			exit(-1);
		}

		int sockplanificador=crearConexionCliente(pathPlanificador);
		int sockcoordinador=crearConexionCliente(pathCoordinador);
	   if(sockplanificador<0 || sockcoordinador<0){
		   log_error(logger,"Error en la conexion con los clientes");
		   return 1;
	   }
	   log_info(logger,"Se realizo correctamente la conexion con el planificador y coordinador");
	   enviarTipoDeCliente(sockcoordinador,ESI);

		while((read = getline(&linea,&length,f)) != -1 ){
			t_esi_operacion operacion = parse(linea);
			if(operacion.valido){
				enviar(sockcoordinador,operacion);
			}
			else{
				log_error(logger, "La linea <%s> no es valida",linea);
				exit(-1);
			}
		}
	   log_destroy(logger);
	   close(sockplanificador);
	   close(sockcoordinador);
	   return 0;
}

int main(int argc, char**argv){
	logger =log_create(logESI,"ESI",1, LOG_LEVEL_INFO);
	esi(argv[1]);
	return 0;
}

