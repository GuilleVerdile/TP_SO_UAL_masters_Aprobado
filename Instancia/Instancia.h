
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
void compactacion();
void inicializarTablaEntradas();
void manejarPaquete(t_esi_operacion paquete);
void meterClaveALaTabla(char* clave);
void meterValorParTalClave(char*valor,int posTabla);
int encontrarTablaConTalClave(char clave[40]);
void* hacerDump();
void almacenarTodaInformacion();
int llegaAOcuparTodasLaEntradas(int* posicion,int* hayQueCompactar,int cantEntradasAOcupar);
int posicionDeLaEntrada(char* entradaABuscar);
#endif /* SOCKET_INSTANCIA_H_ */
