/*
 * Colores.c
 *
 *  Created on: 9 jun. 2018
 *      Author: utnso
 */
#include "Colores.h"
//->Esta funcion colorea una palabra de acuerdo a un color

char *colorear(color col,char *palabra){
	return string_from_format("\x1b[1;3%dm%s%s",col,palabra,"\x1b[0m");
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
