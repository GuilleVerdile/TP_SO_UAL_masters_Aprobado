/*
 * config.c
 *
 *  Created on: 15 abr. 2018
 *      Author: utnso
 */
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <commons/config.h>

t_config * configuracion(char *path){
	t_config *config=config_create(path);
	char *ip=malloc(sizeof(*ip)*16);
	char *puerto=malloc(sizeof(*puerto)*5);
	printf("\nIngrese el Puerto : ");
	scanf("%s",puerto);
	config_set_value(config,"Puerto",puerto);
	printf("\nIngrese el IP : ");
	scanf("%s",ip);
	config_set_value(config,"Ip",ip);
	config_save_in_file(config,path);
	free(ip);
	free(puerto);
	return config;
}
uint32_t ip(char *ip){
	if(strcmp(ip,"0")){
		return 0;
	}
	else{
		return inet_addr(ip);
	}
}
