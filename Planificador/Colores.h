/*
 * Colores.h
 *
 *  Created on: 9 jun. 2018
 *      Author: utnso
 */

#ifndef COLORES_H_
#define COLORES_H_

#include <commons/log.h>
#include <stdarg.h>
#include <stdio.h>

//->Defino los colores que voy a usar

typedef enum {Negro,Rojo,Verde,Amarillo,Azul,Magenta,Cian,Blanco} Color;

//->Defino que loggers voy a usar

t_log *log_test;

t_log *log_importante;

//
char *colorear(Color color,char *palabra);

char *verde(char *palabra);

char *rojo(char *palabra);

char *blanco(char *palabra);

char *amarillo(char *palabra);

char *azul(char *palabra);

char *magenta(char *palabra);

char *cian(char *palabra);

void imprimir(char*(*color)(char*),char *texto);

void loggear(t_log *log,const char *texto,...);

void logTest(char *texto);

void logImportante(char *texto);

#endif /* COLORES_H_ */
