/*
 * Cliente.c
 *
 *  Created on: 15 abr. 2018
 *      Author: utnso
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "FuncionesConexiones.h"
#include <commons/string.h>
#include <commons/log.h>
#include <commons/config.h>

const int SET=0;
const int GET=1;
const int STORE=2;

typedef struct{
	int a;
	char key[40];
	char *value;
}Paquete;

char* transformarTamagnoKey(char* key){
	int tam=string_length(key);
	if(tam<10){
		//return "0"+string_itoa(tam);
		return string_from_format("%s%s","0",string_itoa(tam));
	}
	else
		return string_itoa(tam);
}
char* serealizarPaquete(Paquete pack){
	char* serealizado =string_itoa(pack.a);
	if(!pack.a){
	string_append(&serealizado, transformarTamagnoKey(&pack.key));
	}
	string_append(&serealizado, &pack.key);
	if(!pack.a){
	string_append(&serealizado, pack.value);
	}
	return string_from_format("%s",serealizado);
}

void enviar(int socket,Paquete pack){
	int i =0;
	char *enviar=malloc(5);
	char *buff=serealizarPaquete(pack);
	char *cantBytes=string_itoa(string_length(buff)+1);
	string_append(&cantBytes, "z");
	while(i<string_length(cantBytes)){
		enviar =string_substring(cantBytes, i, 4);
		send(socket,enviar,5,0);
		i=i+4;
	}
	send(socket,buff,string_itoa((string_length(buff)+1)),0);
	free(enviar);
}

int esi(char* pathCoordinador,char*pathPlanificador,Paquete pack){
		char* buffer = malloc(1024);
		int sockplanificador=crearConexionCliente(pathPlanificador);
		int sockcoordinador=crearConexionCliente(pathCoordinador);
	   printf("Se crearon sockets cliente!");
	   if(sockplanificador<0 || sockcoordinador<0){
		   printf("Error de conexion a los servidores\n");
		   return 1;
	   }
	   printf("Se conecto a los 2 servidores\n");
	   enviar(sockcoordinador,pack);
	   return 0;
}

int main(){
	Paquete pack;
	pack.a=SET;
	strcpy (pack.key,"MILLAVE");
	pack.value="MIVALOR";
	char *pathCoordinador="/home/utnso/git/tp-2018-1c-UAL-masters/Config/Coordinador.cfg";
	char *pathPlanificador="/home/utnso/git/tp-2018-1c-UAL-masters/Config/Planificador.cfg";
	esi(pathCoordinador,pathPlanificador,pack);
	while(1){

	}
	return 0;
}

