/*
 * Cliente.c
 *
 *  Created on: 15 abr. 2018
 *      Author: utnso
 */

#include "ESI.h"
	size_t length = 0;
	char* linea = NULL;
	int sockcoordinador;
	int sockplanificador;
	char* resultado;
	FILE* f;
void conectarESI(int *sockcoordinador,int *sockplanificador){
	t_config *config=config_create(pathEsi);
	*sockplanificador=crearConexionCliente(config_get_int_value(config, "Puerto de Conexion al Planificador"),config_get_string_value(config, "IP de Conexion al Planificador"));
	*sockcoordinador=crearConexionCliente(config_get_int_value(config, "Puerto de Conexion al Coordinador"),config_get_string_value(config, "IP de Conexion al Coordinador"));
		   if(*sockplanificador<0 || *sockcoordinador<0){
			   log_error(logger,"Error en la conexion con los clientes");
			   config_destroy(config);
		   }
		   log_info(logger,"Se realizo correctamente la conexion con el planificador y coordinador");
		   config_destroy(config);
}


void* hacerUnaOperacion(){
	log_info(logger,"Estamos dentro del hilo");
	t_esi_operacion operacion = parse(linea);
	if(operacion.valido){
		enviar(sockcoordinador,operacion);
		destruir_operacion(operacion);
		recv(sockcoordinador,resultado,2,0);
		log_info(logger,"Se realizo la operacion");
		log_warning(logger,"El resultado de la operacion es: %s",(resultado[0]=='e')?"OK":"ABORTA");
		if(resultado[0] == 'a' || !feof(f))
		send(sockplanificador,resultado,2,0);
	}
	else{
		send(sockplanificador,"a",2,0);
		log_error(logger, "La linea <%s> no es valida",linea);
		exit(-1);
	}

}

int main(int argc, char**argv){
	pthread_t hiloConexionCoordinador=-1; //LO INICIALIZAMOS EN -1

	ssize_t read;
	f = fopen(argv[1],"r");
	if(f == NULL){
		log_error(logger, "No se pudo abrir el archivo");
		exit(-1);
	}
	logger =log_create(logESI,"ESI",1, LOG_LEVEL_INFO);
	conectarESI(&sockcoordinador,&sockplanificador);
	enviarTipoDeCliente(sockcoordinador,"1");
	resultado = malloc(2);
	send(sockplanificador,"1",2,0);
	int cancelValue = 0;
	while(!feof(f) && recv(sockplanificador, resultado, 2, 0) > 0){ // MIRO QUE NO SEA FIN DE ARCHIVO PARA NO LEER UNA INSTRUCCION VACIA XD
		log_info(logger,"El planificador me dejo ejecutar");
		if(hiloConexionCoordinador==-1 || (cancelValue = pthread_cancel(hiloConexionCoordinador))!=0) //SI CUMPLE LA PRIMERA CONDICION NO ENTRA AL CANCEL
		{
			log_info(logger,"SE VA REALIZAR UN GETLINE %s",(cancelValue==3)?"EL PROCESO NO EXISTE":"ES MI PRIMER PROCESO");
			if(getline(&linea,&length,f) < 0) break; //OBTENGO LA LINEA
		}
		log_info(logger,"La operacion a ejecutar es %s",linea);
		pthread_create(&hiloConexionCoordinador,NULL,hacerUnaOperacion,NULL);
	}
	pthread_join(hiloConexionCoordinador,NULL);
	if(linea){
		free(linea);
	}
    send(sockplanificador,"f",2,0); //FINALIZO LA EJECUCION DEL ESI
	free(resultado);
	log_destroy(logger);
	close(sockcoordinador);
	close(sockplanificador);
	fclose(f);
	return 0;
}

