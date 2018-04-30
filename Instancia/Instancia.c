/*
 * ClienteChat.c
 *
 *  Created on: 16 abr. 2018
 *      Author: utnso
 */

#include "Instancia.h";

int main(){
	logger =log_create(logESI,"ESI",1, LOG_LEVEL_INFO);
    int sockcoordinador;
    if((sockcoordinador =crearConexionCliente(pathCoordinador)) == -1){
    	log_error(logger,"Error en la conexion con el coordinador");
    }
    log_info(logger,"Se realizo correctamente la conexion con el coordinador");
    enviarTipoDeCliente(sockcoordinador,INSTANCIA);
    while(1){

    }
    log_destroy(logger);
    close(sockcoordinador);
	 return 0;
}
