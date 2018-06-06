/*
 * Planificador.h
 *
 *  Created on: 17 abr. 2018
 *      Author: utnso
 */

#ifndef SOCKET_PLANIFICADOR_H_
#define SOCKET_PLANIFICADOR_H_
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "FuncionesConexiones.h"
#include <commons/log.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <pthread.h>
#include <semaphore.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <commons/string.h>


//
int idBuscar;// esto es por ahora
int idGlobal;// esto es por ahora
char *claveABuscar;
t_list *procesos;
t_list *listos;
t_list *terminados;
t_list *bloqueados;
sem_t sem_replanificar;
sem_t sem_procesoEnEjecucion;
sem_t sem_ESIejecutoUnaSentencia;
sem_t sem_finDeEjecucion;
int flag_desalojo;
int flag_nuevoProcesoEnListo;
int tiempo_de_ejecucion;
float alfaPlanificador;
typedef enum {bloqueado,listo,ejecucion,finalizado}Estado;
typedef struct{
	int idProceso;
	int socketProceso;
	Estado estado;
	int tiempo_que_entro;
	float estimacionAnterior;
	float rafagaRealActual;
	float rafagaRealAnterior;
}Proceso;
typedef struct{
	char *clave;
	t_list *bloqueados;
	int idProceso;
} Bloqueo;
pthread_mutex_t planiCorto;
Proceso *procesoEnEjecucion;
//

bool procesoEsIdABuscar(void * proceso);
bool procesoEsIdABuscarSocket(void * proceso);
void actualizarEstado(int id,Estado estado,int porSocket);
void terminarProceso();
void *planificadorCortoPlazo(void *miAlgoritmo);
void *ejecutarEsi(void *esi);
void planificadorLargoPlazo(int id,int estimacionInicial);
Proceso* fifo();
float *estimarSJF(Proceso *proc);
bool compararSJF(void *a,void *b);
float* compararHRRN(Proceso *proc);
Proceso* obtenerSegunCriterio(bool (*comparar) (void*,void*));
Proceso *sjf();
Proceso *hrrn();
bool contieneAlProceso(void *a);
bool esIgualAClaveABuscar(void *a);
Bloqueo *buscarClave();
Bloqueo *buscarBloqueoPorProceso(int id);
Proceso *buscarProcesoPorId(int id);
void eliminarDeLista(int id);
void bloquearPorID(char *clave,int id);
void bloquear(char *clave);
void liberarRecursos(int id);
void liberaClave(char *clave);
char *verificarClave(Proceso *proceso,char *clave);
void desbloquear(int id);
void tirarErrorYexit(char* mensajeError);
void matarESI(int id);
void crearSelect(Proceso*(*algoritmo)(),int estimacionInicial);
void planificador();
void listar(char* clave);

#endif /* SOCKET_PLANIFICADOR_H_ */
