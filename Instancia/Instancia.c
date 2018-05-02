

#include "Instancia.h";

struct TE{
	char clave[40]; //Clave xd
	void* entrada; //Direccion de memoria del archivo de entradas
	int tamValor; //Tamanio del valor asociado a la clave
}typedef tablaEntradas;



int main(int argc, char* argv[]){
	logger =log_create(logESI,"ESI",1, LOG_LEVEL_INFO);
	FILE* TE=fopen("/home/utnso/Escritorio/archivito","r+b");
	tablaEntradas tablaAux;
	tablaEntradas *tablas = NULL;
	int cantTablas = 0;
	if(TE  == NULL){
		log_error(logger,"No se puede leer tal archivo");
		return 1;
	}
	while(fread(&tablaAux,sizeof(tablaEntradas),1,TE)){
		cantTablas++;
		tablas = realloc(tablas,cantTablas*sizeof(tablaEntradas));
		tablas[cantTablas-1] = tablaAux;
	}

	int sockcoordinador;
    if((sockcoordinador =crearConexionCliente(pathCoordinador)) == -1){
    	log_error(logger,"Error en la conexion con el coordinador");
    }
    log_info(logger,"Se realizo correctamente la conexion con el coordinador");
    enviarTipoDeCliente(sockcoordinador,INSTANCIA);
    t_esi_operacion paquete;
    recibir(sockcoordinador,&paquete);
    printf("%s",paquete.argumentos.SET.valor);
    recibir(sockcoordinador,&paquete);
    printf("%s",paquete.argumentos.SET.valor);
    log_destroy(logger);
    close(sockcoordinador);
    free(tablas);
    fclose(TE);
	 return 0;
}






