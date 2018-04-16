/*
 * Main.c
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

#include <commons/config.h>

#include "Config/config.h"
#include "Socket/Coordinador.h"

int main(){
	t_config *miconfig;
	 char *path="/home/utnso/git/tp-2018-1c-UAL-masters/Config/config.cfg";
	 miconfig = configuracion(path);
	 return 0;
}
