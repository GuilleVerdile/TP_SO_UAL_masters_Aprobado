#include "CoordinadorHilos.h"

int main(){
	logger =log_create(logCoordinador,"crearHilos",1, LOG_LEVEL_INFO);
	int listener;
	struct sockaddr_in their_addr; // datos cliente
	if((listener=crearConexionServidor(pathCoordinador))==-1){
		log_error(logger, "No se pudo crear el socket servidor");
		log_destroy(logger);
		return -1;
	}
	log_info(logger, "Se creo el socket de Servidor");
	int nuevoCliente;        // descriptor de socket de nueva conexión aceptada
	int addrlen;
	int casoDiscriminador;
	if (listen(listener, 10) == -1){ //ESCUCHA!
	    log_error(logger, "No se pudo escuchar");
		log_destroy(logger);
		return -1;
	}
	log_info(logger, "Se esta escuchando");
	addrlen = sizeof(their_addr);
	if((casoDiscriminador = accept(listener, (struct sockaddr *)&their_addr,&addrlen))==-1){ //NOS CONECTAMOS CON EL PLANIFICADOR
	    log_error(logger, "No se aceptar la conexion");
	    log_destroy(logger);
	    return -1;
	}
	log_info(logger, "Se acepto la conexion");
	printf("Nuevo cliente, se conecto el Planificador\n");
	fflush(stdout);
	if(send(casoDiscriminador,"Hola capo soy el Coordinador\n",1024,0)==-1){
	    log_error(logger, "No se pudo enviar el mensaje");
	}
	else
	    log_info(logger, "Mensaje enviado correctamente");
	pthread_t idHilo;
	//ACA COMIENZA LO DIVERTIDO =)
	while( (nuevoCliente = accept(listener, (struct sockaddr *)&their_addr,&addrlen))) //Esperamos a que llegue la primera conexion
	{
		puts("Connection accepted");
	    if(pthread_create(&idHilo , NULL , conexionESI, (void*) &nuevoCliente) < 0)
	    {
	       log_error(logger,"No se pudo crear un hilo");
	       return -1;
	    }
	        log_info(logger,"Se asigno una conexion con hilos");
	}
	if(nuevoCliente <0){
		log_error(logger,"No se pudo aceptar la conexion al cliente");
		return -1;
	}
	return 0;
}

void *conexionESI(void* cliente)
{
	log_info(logger,"ESTAMOS DENTRO DEL HILO");
    int socket = *(int*)cliente; //Lo casteamos
    int recibirValor;
    Paquete pack;
    while((recibirValor = recibir(socket,&pack)) > 0)
    {
    	printf("Paquete recibido %d %s %s\n", pack.a,pack.key,pack.value);
    	free(pack.value);
    }

    if(recibirValor == 0)
    {
        log_info(logger,"Se desconecto un ESI");
    }
    else if(recibirValor == -1)
    {
        log_error(logger,"Error al recibir el paquete");
    }
    close(socket);
    return 0;
}
