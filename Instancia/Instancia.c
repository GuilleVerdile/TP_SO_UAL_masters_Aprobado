#include "Instancia.h";

struct TE{
	char clave[40]; //Clave xd
	void* entrada; //Direccion de memoria del archivo de entradas
	int tamValor; //Tamanio del valor asociado a la clave
}typedef tablaEntradas;

tablaEntradas *tablas =NULL;
int cantEntradasDisponibles;
int tamEntradas;

int main(){
	logger =log_create(logInstancias,"Instancia",1, LOG_LEVEL_INFO);
	int sockcoordinador;
    if((sockcoordinador =crearConexionCliente("/home/utnso/git/tp-2018-1c-UAL-masters/Config/Instancia.cfg")) == -1){
    	log_error(logger,"Error en la conexion con el coordinador");
    }
    log_info(logger,"Se realizo correctamente la conexion con el coordinador");
    enviarTipoDeCliente(sockcoordinador,INSTANCIA);
    inicializarTablaEntradas(sockcoordinador);
    t_config *config=config_create("/home/utnso/git/tp-2018-1c-UAL-masters/Config/Instancia.cfg");
    char *buff = config_get_string_value(config, "nombreInstancia"); //obtengo el id
    enviarCantBytes(sockcoordinador,buff);
    send(sockcoordinador,buff,string_length(buff) + 1,0); //ENVIO EL NOMBRE DE LA INSTANCIA
    //NO HACE FALTA HACER FREE AL BUFF YA QUE EL CONFIG DESTROY LO HACE SOLO
    config_destroy(config);
    log_destroy(logger);
    close(sockcoordinador);
	 return 0;
}

void inicializarTablaEntradas(int sockcoordinador){
    int tam = obtenerTamDelSigBuffer(sockcoordinador);
    char* buff = malloc(tam);
    recv(sockcoordinador,buff, tam , 0);
    cantEntradasDisponibles = transformarNumero(buff,0);
    log_info(logger,"La cantidad de entradas es %d", cantEntradasDisponibles);
    free(buff);
    tam = obtenerTamDelSigBuffer(sockcoordinador);
    buff = malloc(tam);
    recv(sockcoordinador,buff, tam , 0);
    tamEntradas = transformarNumero(buff,0);
    log_info(logger,"El tamagno de entradas es %d", tamEntradas);
    free(buff);
}




