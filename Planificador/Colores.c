/*
 * Colores.c
 *
 *  Created on: 9 jun. 2018
 *      Author: utnso
 */
#include "Colores.h"

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

void loggear(t_log *log,Color color,char *texto){
	printf("\x1b[1;3%dm",color);
	log_info(log,texto);
	printf("\x1b[0m");
}

void logTest(char *texto){
	loggear(log_test,Blanco,texto);
}

void logImportante(char *texto){
	loggear(log_test,Verde,texto);
}

