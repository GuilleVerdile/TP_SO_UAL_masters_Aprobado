/*
 * Colores.c
 *
 *  Created on: 9 jun. 2018
 *      Author: utnso
 */
//Includes
#include "Colores.h"
//
//Definicion
#define log_con_logger_espesifico(nombre,logger) 						\
		void nombre(const char* texto, ...) { 							\
			va_list args;												\
			va_start(args, texto);										\
			darFormatoLog(logger,texto,args);							\
			va_end(args);												\
		}																\

//
//->Esta funcion colorea una palabra de acuerdo a un color

char *colorear(Color color,char *palabra){
	return string_from_format("\x1b[1;3%dm%s%s",color,palabra,"\x1b[0m");
}

char *verde(char *palabra){
	return colorear(Verde,palabra);
}

char *rojo(char *palabra){
	return colorear(Rojo,palabra);
}

char *blanco(char *palabra){
	return colorear(Blanco,palabra);
}

char *amarillo(char *palabra){
	return colorear(Amarillo,palabra);
}

char *azul(char *palabra){
	return colorear(Azul,palabra);
}

char *magenta(char *palabra){
	return colorear(Magenta,palabra);
}

char *cian(char *palabra){
	return colorear(Cian,palabra);
}
//Funcion ejemplo de utilizacion con puntero a funciones de colores

void imprimir(char*(*color)(char*),char *texto){
	char *aux=color(texto);
	printf("%s\n",aux);
	free(aux);
}

//Funciones de loggeo

void loggear(t_log *log,const char *texto,...){
	char *texto_imprimir;
	va_list args;
	va_start(args,texto);
	Color color=va_arg(args,Color);
	texto_imprimir=string_from_vformat(texto,args);
	va_end(args);
	printf("\x1b[1;3%dm",color);
	log_info(log,texto_imprimir);
	printf("\x1b[0m");
	free(texto_imprimir);

}

void darFormatoLog(t_log *log,const char *texto,va_list args){
	char *aux;
	Color color=va_arg(args,Color);
	aux=string_from_vformat(texto,args);
	printf("\x1b[1;3%dm",color);
	log_info(log,aux);
	printf("\x1b[0m");
	free(aux);

}
//Defino las funciones log test y log importante
log_con_logger_espesifico(logTest,log_test);

log_con_logger_espesifico(logImportante,log_importante);

