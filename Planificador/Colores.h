/*
 * Colores.h
 *
 *  Created on: 9 jun. 2018
 *      Author: utnso
 */

#ifndef COLORES_H_
#define COLORES_H_


//->Defino los colores que voy a usar
typedef enum {Negro,Rojo,Verde,Amarillo,Azul,Magenta,Cian,Blanco} color;

char *colorear(color col,char *palabra);

char *verde(char *palabra);

char *rojo(char *palabra);

char *blanco(char *palabra);

char *amarillo(char *palabra);

char *azul(char *palabra);

char *magenta(char *palabra);

char *cian(char *palabra);

#endif /* COLORES_H_ */
