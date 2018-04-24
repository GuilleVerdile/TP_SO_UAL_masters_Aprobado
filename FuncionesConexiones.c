/*
 * FuncionesConexiones.c
 *
 *  Created on: 17 abr. 2018
 *      Author: utnso
 */
#include "FuncionesConexiones.h"

const int SET=0;
const int GET=1;
const int STORE=2;

//Path de los servidores
const char *pathCoordinador="/home/utnso/git/tp-2018-1c-UAL-masters/Config/Coordinador.cfg";
const char *pathPlanificador="/home/utnso/git/tp-2018-1c-UAL-masters/Config/Planificador.cfg";

//Los pongo en escritorio para que no tengamos problemas al commitear
const char *logCoordinador="/home/utnso/Escritorio/Coordinador.log";
const char *logPlanificador="/home/utnso/Escritorio/Planificador.log";
const char *logESI="/home/utnso/Escritorio/ESI.log";
const char *logConsola="/home/utnso/Escritorio/Consola.log";
const char *logInstancias="/home/utnso/Escritorio/Instancia.log";

struct sockaddr_in dameUnaDireccion(char *path,int ipAutomatica){
	t_config *config=config_create(path);
	struct sockaddr_in midireccion;
	midireccion.sin_family = AF_INET;
	midireccion.sin_port = htons(config_get_int_value(config, "Puerto"));
	if(ipAutomatica){
		midireccion.sin_addr.s_addr = INADDR_ANY;
	}
	else{
		midireccion.sin_addr.s_addr = inet_addr(config_get_string_value(config,"Ip"));
	}
	config_destroy(config);
	return midireccion;
}
// estas Funciones retornan -1 en caso de error hay que ver despues como manejarlas
int crearConexionCliente(char*path){//retorna el descriptor de fichero
	int sock=socket(AF_INET, SOCK_STREAM, 0);
	if(sock<0){
	return -1;// CASO DE ERROR
	}
	struct sockaddr_in midireccion=dameUnaDireccion(path,0);
	memset(&midireccion.sin_zero, '\0', 8);
	if(connect(sock, (struct sockaddr *)&midireccion, sizeof(struct sockaddr))<0){
		return -1;
	}
	return sock;
}
int crearConexionServidor(char*path){//Retorna el sock del Servidor
	int sockYoServidor=socket(AF_INET, SOCK_STREAM, 0);// no lo retorno me sierve para la creacion del servidor
	if(sockYoServidor<0){
		printf("Error, no se pudo crear el socket\n");
		return -1;// CASO DE ERROR
	}
	struct sockaddr_in miDireccion=dameUnaDireccion(path,0);
	memset(&miDireccion.sin_zero, '\0', 8);
	struct sockaddr_in their_addr;//Aca queda almacenada la informacion del cliente
	int activado = 1;
    if(setsockopt(sockYoServidor, SOL_SOCKET,SO_REUSEADDR,&activado,sizeof(activado)) == -1){
    	printf("Error en la funcion setsockopt");
    	return -1;
    };
    if((bind(sockYoServidor, (struct sockaddr *)&miDireccion, sizeof(struct sockaddr)))<0){
    	printf("Error, no se pudo hacer el bind al puerto\n");
    	return -1;
    }
    return sockYoServidor;
}


//1024
int transformarNumero(char *a,int start){
	int tam=string_length(a);
	int resultado=0;
	for(int i=0;i<tam;i++){
		resultado=(a[i+start]-48)+resultado*10;
	}
	return resultado;
}
Paquete deserializacion(char* texto){
	Paquete pack;
	pack.a = texto[0] -48;
	if(!pack.a){
		int tam = (texto[1]-48)*10 + texto[2]-48;
		strcpy(pack.key,string_substring(texto,3,tam));
		pack.value = string_substring_from(texto,tam+3);
	}
	else{
		strcpy(pack.key, string_substring_from(texto,1));
	}
	return pack;
}
Paquete recibir(int socket){
	char *total=string_new();
	char *buff=malloc(5);
	while(1){
		recv(socket, buff, 5, 0);
			if(string_contains(buff, "z")){
				int i=0;
				char *aux=malloc(5);
				strcpy(aux,buff);
				aux[string_length(buff)-1]='\0';
				string_append(&total,aux);
				free(aux);
				break;
			}
		string_append(&total, buff);
	}
	free(buff);
	int tot=transformarNumero(total,0);
	printf("%d\n",tot);
	char *buf=malloc(tot);
	recv(socket,buf,tot,0);


	Paquete pack=deserializacion(buf);

	fflush(stdout);
	free(buf);
	return pack;
}
//Funciones ESI
char* transformarTamagnoKey(char key[]){
	int tam=string_length(key);
	if(tam<10){
		char* tamKey = malloc(3);
		strcpy(tamKey,"0");
		char* tam1 = string_itoa(tam);
		string_append(&tamKey,tam1);
		free(tam1);
		return tamKey;
	}
	else
		return string_itoa(tam);
}
void serealizarPaquete(Paquete pack,char** buff){
	*buff=string_itoa(pack.a);
	if(!pack.a){
		char* tamkey = transformarTamagnoKey(pack.key);
		string_append(buff,tamkey);
		free(tamkey);
	}
	string_append(buff, pack.key);
	if(!pack.a){
	string_append(buff, pack.value);
	}
}

void enviar(int socket,Paquete pack){
	int i =0;
	char *enviar;
	char *buff;
	serealizarPaquete(pack,&buff);
	char *cantBytes=string_itoa(string_length(buff)+1);
	string_append(&cantBytes, "z");
	while(i<string_length(cantBytes)){
		enviar =string_substring(cantBytes, i, 4);
		send(socket,enviar,5,0);
		i=i+4;
		free(enviar);
	}
	send(socket,buff,string_length(buff)+1,0);
	free(cantBytes);
	free(buff);
}
