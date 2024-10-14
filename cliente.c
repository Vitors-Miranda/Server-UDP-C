/*******************************************************
 * Protocolos de Transporte
 * Grado en Ingeniería Telemática
 * Dpto. Ingeníería de Telecomunicación
 * Universidad de Jaén
 *
 *******************************************************
 * Práctica 2.
 * Fichero: cliente.c
 * Versión: 1.2
 * Curso: 2024/2025
 * Descripción:
 * 	Cliente de eco sencillo sobre UDP en IPv4
 * Autor: Juan Carlos Cuevas Martínez
 *
 ******************************************************
 * Alumno 1: Vitor Samuel Miranda de Souza
 * Alumno 2: Alicia Gianny Silva Caralho
 *
 ******************************************************/
#include <stdio.h>		// Biblioteca estándar de entrada y salida
#include <ws2tcpip.h>	// Necesaria para las funciones IPv6
#include <locale.h>		// Para establecer el idioma de la codificación de texto, números, etc.


#pragma comment(lib, "Ws2_32.lib")//Enlaza la biblioteca Ws2_32.lib

#define UDP_CLIENT_PORT	60001
#define UDP_SERVER_PORT	60000
#define NO_AUTH 0
#define AUTH 1

//function to verify if is alphanumeric
static int isAlfanum(const char* cadena) {
	for (int i = 0; i < strlen(cadena); i++) {
		if (!isalnum(cadena[i])) {
			return 0; // No es alfanumérico
		}
	}
	return 1; // Es alfanumérico
}

int main(int *argc, char *argv[]){
	// Variables de incialización de los Windows Sockets
	WORD wVersionRequested;
	WSADATA wsaData;

	// Fin Variables de incialización de los Windows Sockets
	SOCKET sockfd;
	struct sockaddr_in client_in,server_in,input_in;
	int input_l;
	char buffer_in[2048], buffer_out[2048];
	int in_len;
	char user_input[1024];
	int recibidos=0;
	int recibidosLogin =0;
	int enviados=0;
	int loginEnviados=0;
	char iplocal[20]="127.0.0.1";
    char ipdest[20]="127.0.0.1";
	int n_secuencia=1;
	int err=0;
	char user[16], pass[16];
	int status = 0;
	int error;

	//Inicialización de idioma
	setlocale(LC_ALL, "es-ES");

	// Inicialización Windows Sockets versión 2.2
	wVersionRequested=MAKEWORD(2,2);
	err=WSAStartup(wVersionRequested,&wsaData);
	if(err!=0)
		return(0);

	if(LOBYTE(wsaData.wVersion)!=2||HIBYTE(wsaData.wVersion)!=2){
		WSACleanup();
		return(0);
	}// Fin Inicialización Windows Sockets

	sockfd=socket(PF_INET,SOCK_DGRAM,0);
	if(sockfd==INVALID_SOCKET){
		printf("CLIENTE UDP> Error\r\n");
	}else{
		//Dirección local del cliente
		client_in.sin_family=AF_INET;
		client_in.sin_port=htons(UDP_CLIENT_PORT);
		
		
		inet_pton(AF_INET,iplocal,&client_in.sin_addr.s_addr);


		if(bind(sockfd,(struct sockaddr *)&client_in,sizeof(client_in))==SOCKET_ERROR){
			printf("CLIENTE UDP> Error %d\r\n",GetLastError());
		
		}else{
			char cadtemp[20];
				
				// Dirección remota del servidor para cada envío
				printf("CLIENTE UDP> IP del servidor [%s] : ",ipdest);
				
				gets_s(cadtemp,sizeof(cadtemp));
				if(strcmp(cadtemp,"")!=0)
					strcpy_s(ipdest,sizeof(ipdest),cadtemp);

				server_in.sin_family=AF_INET;
				server_in.sin_port=htons(UDP_SERVER_PORT);
				
				inet_pton(AF_INET,ipdest,&server_in.sin_addr.s_addr);

			do{// Se estarán enviando mensajes de eco hasta que se pulse solo un enter
				switch (status) {

					//If the user is not logged
					case NO_AUTH:

						//User input
						printf("CLIENTE UDP> Introduza el usuario : \n");
						gets_s(user, sizeof(user));

						//Validate user
						if (strlen(user) < 4 || strlen(user) > 16 || isAlfanum(user) == 0){
							printf("Error: el usuario debe tener entre 4 y 16 caracteres y ser alfanumerico.\n");
						
						
						}else {
							//password input
							printf("CLIENTE UDP> Introduza la clave: \n");
							gets_s(pass, sizeof(pass));

							//Validate password
							if (strlen(pass) < 4 || strlen(pass) > 16 || isAlfanum(pass) == 0) {
								printf("Error: la clave debe tener entre 4 y 16 caracteres y ser alfanumerico.\n");

							} else {

								//Sending the data to validate it
								sprintf_s(buffer_out, sizeof(buffer_out), "LOGIN %s %s CRLF\r\n", user, pass);
								enviados = sendto(sockfd, buffer_out, (int)strlen(buffer_out), 0, (struct sockaddr*)&server_in, sizeof(server_in));

								if (enviados != SOCKET_ERROR) {
									printf("CLIENTE UDP> Enviados %d bytes para a autenticacion\r\n\n", enviados);
									in_len = sizeof(buffer_in);
									input_l = sizeof(input_in);

									//Data recieved
									recibidos = recvfrom(sockfd, buffer_in, in_len, 0, (struct sockaddr*)&input_in, &input_l);
									if (recibidos != SOCKET_ERROR) {
										char peer[32] = "";
										int r_secuencia = 0;
										char eco[1024] = "";
										buffer_in[recibidos] = 0;

										inet_ntop(AF_INET, &input_in.sin_addr, peer, sizeof(peer));

										printf("CLIENTE UDP> Recibidos %d bytes de %s %d\r\n", recibidos, peer, ntohs(input_in.sin_port));

										//IF the server return OK the user will be logged, although it asks for the user and password again.
										if (strncmp(buffer_in, "OK", 2) == 0) {
											printf("CLIENTE UDP> %s\r\n", buffer_in);
											status = AUTH;
										}
										else if (strncmp(buffer_in, "ER", 2) == 0) {
											/*
											Error type:
											1 = Caracteres del usuario invalidos.
											2 = Caracteres de la clave invalidos.
											3 = Clave o usuario incorrecto.
											*/
											error = buffer_in[2] - '0';
											printf("CLIENTE UDP>  %d \n", error);
										}
										else {
											printf("CLIENTE UDP> Error no reconocido. \n");
										}
									}
									n_secuencia++;
								}
							}
						}
						break;

					case AUTH:
						printf("CLIENTE UDP> Introduzca una cadena para enviar al servidor: ");
						gets_s(user_input, sizeof(user_input));
						sprintf_s(buffer_out, sizeof(buffer_out), "ECHO %d %s\r\n", n_secuencia, user_input);

						enviados = sendto(sockfd, buffer_out, (int)strlen(buffer_out), 0, (struct sockaddr*)&server_in, sizeof(server_in));
						if (enviados != SOCKET_ERROR) {
							printf("CLIENTE UDP> Enviados %d bytes\r\n", enviados);
							in_len = sizeof(buffer_in);
							input_l = sizeof(input_in);

							recibidos = recvfrom(sockfd, buffer_in, in_len, 0, (struct sockaddr*)&input_in, &input_l);
							if (recibidos != SOCKET_ERROR) {
								char peer[32] = "";
								int r_secuencia = 0;
								char eco[1024] = "";
								buffer_in[recibidos] = 0;

								inet_ntop(AF_INET, &input_in.sin_addr, peer, sizeof(peer));

								printf("CLIENTE UDP> Recibidos %d bytes de %s %d\r\n", recibidos, peer, ntohs(input_in.sin_port));
								sscanf_s(buffer_in, "OK %d %[^\r]s\r\n", &r_secuencia, eco, sizeof(eco));

								if (r_secuencia == n_secuencia && strlen(eco) > 0) {
									printf("CLIENTE UDP> Eco recibido: %s\r\n", eco);
								}
								else {
									printf("CLIENTE UDP> Error en la respuesta");
								}
							}
						}
						break;
				
				}
				
			}while(strcmp("",user_input)!=0);
		}
		closesocket(sockfd);
	}//fin sockfd==INVALID_SOCKET
	
	WSACleanup();// solo Windows

}//fin main