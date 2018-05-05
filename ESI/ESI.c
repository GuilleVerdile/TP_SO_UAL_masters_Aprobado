/*
 * Cliente.c
 *
 *  Created on: 15 abr. 2018
 *      Author: utnso
 */


#include "ESI.h"
void conectarESI(int *sockcoordinador,int *sockplanificador){
	*sockplanificador=crearConexionCliente(pathPlanificador);
	*sockcoordinador=crearConexionCliente(pathCoordinador);
		   if(sockplanificador<0 || sockcoordinador<0){
			   log_error(logger,"Error en la conexion con los clientes");
		   }
		   log_info(logger,"Se realizo correctamente la conexion con el planificador y coordinador");
}

int esi(char* path,int sockcoordinador,int sockplanificador){
		FILE* f;
		size_t length = 0;
		ssize_t read;
		char* linea = NULL;
		f = fopen(path,"r");
		if(f == NULL){
			log_error(logger, "No se pudo abrir el archivo");
			exit(-1);
		}
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
			free(linea);
		}
		fclose(f);
	   return 0;
}

int main(int argc, char**argv){
	int sockcoordinador;
	int sockplanificador;
	logger =log_create(logESI,"ESI",1, LOG_LEVEL_INFO);
	conectarESI(&sockcoordinador,&sockplanificador);
	char *buff = malloc(2);;
	int puedoEnviar=1;
	while(puedoEnviar){
		send(sockplanificador,"1",2,0);
		recv(sockplanificador, buff, 2, 0);
		if(((buff[0]-48))){
			esi(argv[1],sockcoordinador,sockplanificador);
			send(sockplanificador,"1",2,0);
			puedoEnviar=0;
		}
	}
	free(buff);
	log_destroy(logger);
	close(sockcoordinador);
	close(sockplanificador);
	return 0;
}

