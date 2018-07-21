
#ifndef SOCKET_INSTANCIA_H_
#define SOCKET_INSTANCIA_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <FuncionesConexiones.h>
#include <parsi/parser.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <commons/collections/list.h>
const char* INSTANCIA = "0";
struct TE{
	char clave[40]; //Clave
	char* entrada; //Direcciones de memoria de las entradas de la clave
	int tamValor; //Tamanio del valor asociado a la clave
	int seAlmacenoElValor;
	int nroOperacion;
}typedef tablaEntradas;
void liberarListas(pthread_t id);
void compactacion();
void inicializarTablaEntradas();
void manejarPaquete(t_esi_operacion paquete);
void meterClaveALaTabla(char* clave);
int meterValorParTalClave(char*valor,tablaEntradas* tablaEntrada);
int encontrarTablaConTalClave(char clave[40]);
void* hacerDump();
void almacenarTodaInformacion();
int llegaAOcuparTodasLaEntradas(int* posicion,int cantEntradasAOcupar,int* cantidadEntradasLibres);
int posicionDeLaEntrada(char* entradaABuscar);
typedef void(*algoritmo)();
algoritmo obtenerAlgoritmoDeReemplazo();
void circular();
void lru();
void enviarValor(char* clave);
void buscarYLiberarClave(char* clave);
void liberarClave(int posTabla);
int esAtomico(tablaEntradas* tabla);
#endif /* SOCKET_INSTANCIA_H_ */
