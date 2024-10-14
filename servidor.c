/*******************************************************
 * Protocolos de Transporte
 * Grado en Ingeniería Telemática
 * Dpto. Ingeníería de Telecomunicación
 * Universidad de Jaén
 *
 *******************************************************
 * Práctica 2.
 * Fichero: servidor.c
 * Versión: 1.2
 * Curso: 2024/2025
 * Descripción:
 * 	Servidor de eco sencillo sobre UDP en IPv4
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

#define UDP_CLIENT_PORT	60001//Puerto del que deben venir los mensajes
#define UDP_SERVER_PORT	60000//Puerto en el que el servidor recibirá peticiones

#pragma comment(lib, "Ws2_32.lib")//Enlaza la biblioteca Ws2_32.lib
#define NO_AUTH 0
#define AUTH 1

//function to verify if is alphanumeric
static int isAlfanum(const char* cadena) {
	for (int i = 0; i < strlen(cadena); i++) {
		if (!isalnum(cadena[i])) {
			return 0; // is not alfanumeric
		}
	}
	return 1; // is alfanumeric
}
int main(int *argc, char *argv[])
{
	// Variables de incialización de los Windows Sockets
	WORD wVersionRequested;
	WSADATA wsaData;
	// Fin Variables de incialización de los Windows Sockets

	SOCKET sockfd;
	struct sockaddr_in server_in,input_in;
	int input_l;
	char buffer_in[2048], buffer_out[2048];
	char command[10];
	char user[16];
	char pass[16];
	char user_input[1024];
	int recibidos=0;
	int enviados=0;
	char iplocal[20]="127.0.0.1";
	int n_secuencia=0;
	int err=0;
	int status = 0;

	//Inicialización de idioma
	setlocale(LC_ALL, "es-ES");

	// Inicialización Windows Sockets
	wVersionRequested=MAKEWORD(2,2);
	err=WSAStartup(wVersionRequested,&wsaData);
	if(err!=0)
		return(0);

	if(LOBYTE(wsaData.wVersion)!=2||HIBYTE(wsaData.wVersion)!=2){
		WSACleanup();
		return(0);
	}// Fin Inicialización Windows Sockets
	
	sockfd=socket(PF_INET,SOCK_DGRAM,0); //SOCKET create a new socket
	/*
	PARAMENTERS: 
	1) the first paramenter "PF_INET" specify the address family, in this case IPv4
	2) the second one specify the type for the new socket. In this case the "SOCK_DGRAM" that allows datagrams and it utilizes the UDP protocol
	3) the last one finger the protocol that will be used. In this case was specified the value "0" that means any protocol will be specified now and the service provider will choose the protocol that will be used
	*/
	if(sockfd==INVALID_SOCKET){
		printf("SERVIDOR UDP> Error \r\n");
	}else{
		printf("SERVIDOR UDP> Socket creado correctamente\r\n");

		//Dirección local del servidor
		server_in.sin_family=AF_INET;
		server_in.sin_port=htons(UDP_SERVER_PORT);
		//server_in.sin_addr.s_addr=inet_addr(iplocal);
		inet_pton(AF_INET,iplocal,&server_in.sin_addr.s_addr);
		
		if(bind(sockfd,(struct sockaddr *)&server_in,sizeof(server_in))==SOCKET_ERROR){ // SOCKET the function "bind" associate an local address to a socket
			/*
			PARAMENTERS:
			1) identify an independent socket
			2) a point to a sockaddr structure of the local address to be assigned to the bound socket
			3) the lenght, in bytes, of the value that is pointed to addr
			*/
			printf("SERVIDOR UDP> Error %d:\r\n",GetLastError());
		}else{
			printf("SERVIDOR UDP> Bienvenido al Servidor de Eco Sencillo UDP\r\n");

			while(1){//Bucle infinito de servicio
				input_l=sizeof(input_in);
				recibidos=recvfrom(sockfd,buffer_in,2047,0,(struct sockaddr *)&input_in,&input_l); // SOCKET receive a datagram 
				if(recibidos!=SOCKET_ERROR){
					char peer[32]="";
					buffer_in[recibidos]=0;
					inet_ntop(AF_INET, &input_in.sin_addr, peer, sizeof(peer));
					printf("SERVIDOR UDP> Recibido de %s %d: %s\r\n",peer,ntohs(input_in.sin_port),buffer_in);

					if(ntohs(input_in.sin_port)==UDP_CLIENT_PORT){// Se comprueba que el mensaje llegue desde el puerto típico para
																  // este servicio, el 6001. Si no es así no se lleva a cabo ninguna
																  // acción.
						switch (status){
						case NO_AUTH:

							//The data is in "buffer_in", so we are taking the command at the first argument and saving the parameterns in user_input
							sscanf_s(buffer_in, "%s %[^\r\n]", command, sizeof(command), user_input, sizeof(user_input));

							if (strcmp(command, "LOGIN") == 0) {

								// We are taking the arguments one by one of user_input
								if(sscanf_s(user_input, "%s %s", user, sizeof(user), pass, sizeof(pass))){

									//IF user is not formatted
									if (strlen(user) < 4 || strlen(user) > 16 || isAlfanum(user) == 0) {

										//We modificated the ABNF "ER CRLF" to "ER SP ERROR_CODE CRLF".
										sprintf_s(buffer_out, sizeof(buffer_out), "ER 1 CRLF\n");

									} else{
										//IF password is not formatted
										if (strlen(pass) < 4 || strlen(pass) > 16 || isAlfanum(pass) == 0) {

											//We modificated the ABNF "ER CRLF" to "ER SP ERROR_CODE CRLF".
											sprintf_s(buffer_out, sizeof(buffer_out), "ER 2 CRLF\n");

										}else{
											// If login and password are correct
											if (strcmp(user, "user") == 0 && strcmp(pass, "1234") == 0) {
												printf("Login valido - Usuario: %s\n", user);
												sprintf_s(buffer_out, sizeof(buffer_out), "OK %s CRLF\n", user);
												status = AUTH;
											}
											//If login is incorrect return ER
											else {
												printf("Login invalido - Usuario: %s\n", user);

												//We modificated the ABNF "ER CRLF" to "ER SP ERROR_CODE CRLF".
												sprintf_s(buffer_out, sizeof(buffer_out), "ER 3 CRLF\n");
											}
										}

									}
								}
							}
							else {
								printf("Comando inválido: %s\n", command);
								sprintf_s(buffer_out, sizeof(buffer_out), "ER Comando inválido\n");
							}


							enviados = sendto(sockfd, buffer_out, (int)strlen(buffer_out), 0, (struct sockaddr*)&input_in, sizeof(input_in));
							if (enviados == SOCKET_ERROR) {
								printf("SERVIDOR UDP> Error al enviar la respuesta.");
							}
							break;

						case AUTH:

							sscanf_s(buffer_in, "%s %d %[^\r]s\r\n", command, sizeof(command), &n_secuencia, user_input, sizeof(user_input));

							if (strcmp(command, "ECHO") == 0) {// Si el mensaje no está bien formateado tampoco se responde para evitar
								// un gasto de recursos innecesario
								sprintf_s(buffer_out, sizeof(buffer_out), "OK %d %s\r\n", n_secuencia, user_input);

							}
							else {
								printf("SERVIDOR UDP> Comando no reconocido\r\n");
							}


							//ECHO
							enviados = sendto(sockfd, buffer_out, (int)strlen(buffer_out), 0, (struct sockaddr*)&input_in, sizeof(input_in));
							if (enviados == SOCKET_ERROR) {
								printf("SERVIDOR UDP> Error al enviar la respuesta.");
							}
							break;
						}
					}
				}//Si hay un error de recepción se silencia
			}//Fin bucle del servicio
		}	
		closesocket(sockfd); //SOCKET closes a existent socket
		/*
		PARAMETERS: 
		1) the socket that will close
		*/
	}//fin sockfd==INVALID_SOCKET
}//fin main