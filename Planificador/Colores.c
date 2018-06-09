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

