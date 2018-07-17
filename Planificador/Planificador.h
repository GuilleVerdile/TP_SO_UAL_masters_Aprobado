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
#include <FuncionesConexiones.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <pthread.h>
#include <semaphore.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <commons/string.h>
#include "Colores.h"

//
int idBuscar;// esto es por ahora
int idGlobal;// esto es por ahora
int idBanquero;
char *claveABuscar;
t_list *procesos;
t_list *listos;
t_list *terminados;
t_list *bloqueados;
sem_t sem_replanificar;
sem_t sem_procesoEnEjecucion;
sem_t sem_ESIejecutoUnaSentencia;
sem_t sem_finDeEjecucion;
//semaforos in jiava lenguage
sem_t sem_pausar;
pthread_mutex_t mutex_pausa;
//flags in jiava lenguage
int flag_seEnvioSignalPlanificar;
int flag_quierenDesalojar;
//
sem_t sem_finDeEsiCompleto;
sem_t semCambioEstado;

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

void cerrarPlanificador();
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
Proceso* obtenerSegunCriterio(bool (*comparar) (void*,void*));
Proceso *sjf();
Proceso *hrrn();
bool contieneAlProceso(void *a);
bool esIgualAClaveABuscar(void *a);
Bloqueo *buscarClave();
Bloqueo *buscarBloqueoPorProceso(int id);
Proceso *buscarProcesoPorId(int id);
void bloquearPorID(char *clave,int id);
void bloquear(char *clave);
void liberarRecursos(int id);
void liberaClave(char *clave);
char *verificarClave(Proceso *proceso,char *clave);
void tirarErrorYexit(char* mensajeError);
void matarESI(int id);
void crearSelect(int estimacionInicial);
void planificador();
void listar(char* clave);
void bloquearClavesIniciales(t_config *config);
char *sePuedeBloquear(char*clave);
void destruirUnProceso(void *elemento);
void destruirUnBloqueado(void *elemento);
void bloquearPorConsola(char *clave,int id);
float* estimarHRRN(Proceso *proc);
bool compararHRRN(void *a,void *b);
bool algoritmoBanquero();
//jiava
void meterEsiColaListos(Proceso *proceso);
void enviarSegnalPlanificar();
#endif /* SOCKET_PLANIFICADOR_H_ */
