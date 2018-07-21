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
void tirarError(char* mensaje){
	log_error(logger,"%s",mensaje);
	resultado[0] = 'a';
}
int conectarESI(char* tipoServidor){
	t_config *config=config_create(pathEsi);
	char* stringPuerto = malloc(strlen(tipoServidor)+strlen("Puerto de Conexion al ") + 1);
	strcpy(stringPuerto,"Puerto de Conexion al ");
	strcat(stringPuerto,tipoServidor);
	//log_info(logger,"%s",stringPuerto);
	char* stringIP = malloc(strlen(tipoServidor)+strlen("IP de Conexion al ") + 1);
	strcpy(stringIP,"IP de Conexion al ");
	strcat(stringIP,tipoServidor);
	//log_info(logger,"%s",stringIP);
	int socket = crearConexionCliente(config_get_int_value(config, stringPuerto),config_get_string_value(config, stringIP));
	if(socket < 0){
		log_error(logger,"Error en la conexion con los clientes");
		config_destroy(config);
	}
	log_info(logger,"Se realizo correctamente la conexion con el %s", tipoServidor);
	config_destroy(config);
	free(stringPuerto);
	free(stringIP);
	return socket;
}

void hacerUnaOperacion(){
	enviarTipoDeCliente(sockcoordinador,"1");
	t_esi_operacion operacion = parse(linea);
	if(operacion.valido){
			enviar(sockcoordinador,operacion);
			destruir_operacion(operacion);
			recv(sockcoordinador,resultado,2,0);
			log_info(logger,"Se realizo la operacion");
			log_warning(logger,"El resultado de la operacion es: %s",(resultado[0]=='e')?"OK":(resultado[0]=='b')?"BLOQUEAR":"ABORTA");
			if(resultado[0] == 'e')
				send(sockplanificador,resultado,2,0);
			if(resultado[0]=='a'){
				int tam = obtenerTamDelSigBuffer(sockcoordinador);
				char* mensajeError = malloc(tam);
				recv(sockcoordinador,mensajeError,tam,0);
				tirarError(mensajeError);
				free(mensajeError);
		}
	}else{
		destruir_operacion(operacion);
		tirarError("ESI: Error Operacion Invalida");
	}
	close(sockcoordinador);
}

int main(int argc, char**argv){
	ssize_t read;
	logger =log_create(logESI,"ESI",1, LOG_LEVEL_INFO);
	f = fopen(argv[1],"r");
	int noBloqueado = 1;
	if(f == NULL){
		log_error(logger, "No se pudo abrir el archivo");
		exit(-1);
	}
	sockplanificador = conectarESI("Planificador");
	resultado = malloc(2);
	log_info(logger,"Se conecto con el planificador. Esperando permiso para ejecucion...");
	while((!feof(f) || !noBloqueado)&& recv(sockplanificador, resultado, 2, 0) > 0){ // MIRO QUE NO SEA FIN DE ARCHIVO PARA NO LEER UNA INSTRUCCION VACIA XD
		if(resultado[0]=='f')//me forzaron a finalizar
			break;
		log_info(logger,"El planificador me dejo ejecutar");
		if(noBloqueado){
			if(getline(&linea,&length,f) < 0) break; //OBTENGO LA LINEA
		}
		log_info(logger,"La operacion a ejecutar es %s",linea);
		if(linea)
		sockcoordinador = conectarESI("Coordinador");
		hacerUnaOperacion();
		if(resultado[0]=='a'){
			break;
		}
		noBloqueado = strcmp(resultado,"b");

		if(noBloqueado < 0)break;
	}
	if(linea){
		free(linea);
	}
    send(sockplanificador,"f",2,0); //FINALIZO LA EJECUCION DEL ESI
	free(resultado);
	log_destroy(logger);
	close(sockplanificador);
	fclose(f);
	return 0;
}

