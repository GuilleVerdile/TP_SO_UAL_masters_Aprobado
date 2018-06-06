/*
 * Cliente.c
 *
 *  Created on: 15 abr. 2018
 *      Author: utnso
 */

char *pathEsi="../Config/ESI.cfg";
#include "ESI.h"
	size_t length = 0;
	char* linea = NULL;
	int sockcoordinador;
	int sockplanificador;
	char* resultado;
void conectarESI(int *sockcoordinador,int *sockplanificador){
	t_config *config=config_create(pathEsi);
	*sockplanificador=crearConexionCliente(config_get_int_value(config, "Puerto de Conexion al Planificador"),config_get_string_value(config, "IP de Conexion al Planificador"));
	*sockcoordinador=crearConexionCliente(config_get_int_value(config, "Puerto de Conexion al Coordinador"),config_get_string_value(config, "IP de Conexion al Coordinador"));
		   if(sockplanificador<0 || sockcoordinador<0){
			   log_error(logger,"Error en la conexion con los clientes");
			   config_destroy(config);
		   }
		   log_info(logger,"Se realizo correctamente la conexion con el planificador y coordinador");
		   config_destroy(config);
}


void* hacerUnaOperacion(){
			t_esi_operacion operacion = parse(linea);
			if(operacion.valido){
				enviar(sockcoordinador,operacion);
				recv(sockcoordinador,resultado,2,0);
				send(sockplanificador,resultado,2,0);
			}
			else{
				send(sockplanificador,"a",2,0);
				log_error(logger, "La linea <%s> no es valida",linea);
				exit(-1);
			}
			free(linea);
}

int main(int argc, char**argv){
	pthread_t hiloConexionCoordinador=-1; //LO INICIALIZAMOS EN -1
	FILE* f;
	ssize_t read;
	f = fopen(argv[1],"r");
	if(f == NULL){
		log_error(logger, "No se pudo abrir el archivo");
		exit(-1);
	}
	logger =log_create(logESI,"ESI",1, LOG_LEVEL_INFO);
	conectarESI(&sockcoordinador,&sockplanificador);
	enviarTipoDeCliente(sockcoordinador,ESI);
	resultado = malloc(2);
		send(sockplanificador,"1",2,0);
		while(recv(sockplanificador, resultado, 2, 0)){
			if(hiloConexionCoordinador==-1 || (pthread_cancel(&hiloConexionCoordinador) < 0)) //SI CUMPLE LA PRIMERA CONDICION NO ENTRA AL CANCEL
			{
				if(getline(&linea,&length,f) < 0) break; //OBTENGO LA LINEA
			}
			pthread_create(&hiloConexionCoordinador,NULL,hacerUnaOperacion(),NULL);
		}
    send(sockplanificador,"f",2,0); //FINALIZO LA EJECUCION DEL ESI
	free(resultado);
	log_destroy(logger);
	close(sockcoordinador);
	close(sockplanificador);
	fclose(f);
	return 0;
}

