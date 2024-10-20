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

#define NO_AUTH 0 //variable for the case when the client is not logged in.
#define AUTH 1 //variable for the case when the client is logged in.

 //function to verify if it is alphanumeric
static int isAlfanum(const char* cadena) {

	//Traversing the letters of the word
	for (int i = 0; i < strlen(cadena); i++) {


		if (!isalnum(cadena[i])) {
			return 0; // it is not alfanumeric
		}

	}
	return 1; // it is alfanumeric
}

//function to verify the Log In data
static int log_in(char command[10], char user_input[1024], int status, char user[16], char pass[16]) { //APLICACIÓN: verify the command of the sesion beggining and verify the format
	/*
	ABNF format that can be receiv:
	"LOGIN" SP USER SP PASS CRLF
	*/

	if (strcmp(command, "LOGIN") == 0) { //the method format is correct

		if (strlen(user) < 4 || strlen(user) > 16 || isAlfanum(user) == 0) { // the user format is not correct
			return 1;

		}
		else {
			if (strlen(pass) < 4 || strlen(pass) > 16 || isAlfanum(pass) == 0) { //the password format is not correct
				return 2;

			}
			else {
				if (strcmp(user, "agsc0001") == 0 && strcmp(pass, "z1089913v") == 0) { //the login and password is correct to log in
					printf("Login valido - Usuario: %s\n", user);
					return 0;
				}
				else { //The login is wrong
					printf("Login invalido - Usuario: %s\n", user);
					return 3;
				}
			}
		}
	}

	else { //The client sends other method when the server is waiting for a LOGIN.
		printf("El cliente debe conectarse primero: %s\n", command);
		return 4;
	}
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
				recibidos=recvfrom(sockfd,buffer_in,2047,0,(struct sockaddr *)&input_in,&input_l); // SOCKET the function recvform receives a datagram and stores the source address
					/*
					PARAMETERS:
					1) the parameter "sockfd" identify the bound socket
					2) the second parameter "buffer_in" is a buffer to incoming data
					3) "2047" informes the length of the buffer pointed to the second parameter 
					4) this parameter is the flags parameter that can be used to influence the behavior of the function invocation beyond the options specified for the associated socket. In this case there'is not any flag.
					5) (struct sockaddr *)&input_in is a parameter that point to a buffer in a sockaddr structure that will hold the source address upon return.
					6) pointer to the size, in bytes, of the buffer pointed to by the last parameter
					*/
				if(recibidos!=SOCKET_ERROR){
					char peer[32]=""; //it saves the client address
					buffer_in[recibidos]=0; //creating a buffer to recieve the data
					int status_code; //variable to recieve the satus of request
					char user[16];
					char pass[16];

					inet_ntop(AF_INET, &input_in.sin_addr, peer, sizeof(peer)); //adding the client adress into "peer"
					printf("SERVIDOR UDP> Recibido de %s %d: %s\r\n",peer,ntohs(input_in.sin_port),buffer_in);

					if(ntohs(input_in.sin_port)==UDP_CLIENT_PORT){// Se comprueba que el mensaje llegue desde el puerto típico para
																  // este servicio, el 6001. Si no es así no se lleva a cabo ninguna
																  // acción.
						switch (status){
						case NO_AUTH: // The client is not logged

							//The data is in "buffer_in", so we are taking the command at the first argument and saving the parameterns in user_input
							sscanf_s(buffer_in, "%s %[^\r\n]", command, sizeof(command), user_input, sizeof(user_input)); 

							//saving the pass and the user
							if (sscanf_s(user_input, "%s %s", user, sizeof(user), pass, sizeof(pass))) { // APLICACION receiv the user data 
								/* ABNF FORMAT :
								"LOGIN" SP USER SP PASS CRLF 
								USER = 4*16 VCHAR 
								PASS = 4*16 VCHAR
								*/
								status_code = log_in(command, user_input, status, user, pass); //it recieves the satatus

								if (status_code == 0) { //it is Ok
									sprintf_s(buffer_out, sizeof(buffer_out), "OK %s CRLF\n", user); // APLICACION sends to the user a confirmation message
									/*
									ABNF format:
									"OK" SP USER CRLF
									*/
									status = AUTH; // changing the status for the client sends a ECHO in the nest request
								}
								else { //it is an Error
									//We modificated the ABNF "ER CRLF" to "ER SP ERROR_CODE CRLF".
									sprintf_s(buffer_out, sizeof(buffer_out), "ER %d CRLF\n", status_code); // APLICACION sends a type of error to client if occurs a error on the log in
									/*
									ABNF format:
									"ER" SP status_code CRLF
									*/
								}
							}
							else {
								sprintf_s(buffer_out, sizeof(buffer_out), "ER 5 CRLF\n");
							}
							
							
							break;

						case AUTH: //case the client is logged in

							sscanf_s(buffer_in, "%s %d %[^\r]s\r\n", command, sizeof(command), &n_secuencia, user_input, sizeof(user_input)); 

							if (strcmp(command, "ECHO") == 0) {// Si el mensaje no está bien formateado tampoco se responde para evitar
								// un gasto de recursos innecesario
								sprintf_s(buffer_out, sizeof(buffer_out), "OK %d %s\r\n", n_secuencia, user_input); // APLICACION this function sends ECHO with the number of sequence and the usr input
								/*
								ABNF format:
								"OK" SP sequence_number SP user_input
								*/

							}

							/*
							If the server is requesting an echo and the client sends other method,
							it means that the client is trying to log in. Probably, the client lost
							the conexion and other client begins a request. So, the server sends 
							the data for the Log In Method.
							*/

							else {
								sscanf_s(buffer_in, "%s %[^\r\n]", command, sizeof(command), user_input, sizeof(user_input));
								if (sscanf_s(user_input, "%s %s", user, sizeof(user), pass, sizeof(pass))) { //saving the pass and the user
									status_code = log_in(command, user_input, status, user, pass); //it recieves the satatus
									if (status_code == 0) { //it is Ok
										sprintf_s(buffer_out, sizeof(buffer_out), "OK %s CRLF\n", user); // APLICATION sends to user the confirmation mensage when the user is correctly authenticated
										/*
										ABNF format:
										"OK" SP user CRLF
										*/
										status = AUTH; // changing the status for the client sends a ECHO in the nest request
									}
									else { //it is an Error
										//We modificated the ABNF "ER CRLF" to "ER SP ERROR_CODE CRLF".
										/*
											Error type:
											1 = Invalid user characters.
											2 = Invalid password characters.
											3 = Incorrect username or password.
											4 = Invalid command.
											5 = Invalid number of parameters.
										*/
										sprintf_s(buffer_out, sizeof(buffer_out), "ER %d CRLF\n", status_code); // APLICATION sends the error type
										/*
										ABNF format:
										"ER" SP status_code CRLF
										*/
									}
								}
								else {
									sprintf_s(buffer_out, sizeof(buffer_out), "ER 5 CRLF\n");
								}
							}
							break;
						}

						//it sends the OK or ER to the client
						enviados = sendto(sockfd, buffer_out, (int)strlen(buffer_out), 0, (struct sockaddr*)&input_in, sizeof(input_in)); // SOCKET this funtion sends data to specific destination
							 /*
							 PARAMETERS
							 1) idetinify thee soket that will be used
							 2) a pointer to a buffer containing the data that will be transmitted
							 3) the length of the buffer on the last parameter
							 4) set of flags that specify the way in which the call is made. In this case, there is not flag
							 5) (struct sockaddr *)&input_in is a parameter that point to a buffer in a sockaddr structure that will hold the source address upon return.
							 6) the size, in bytes, of the address pointed to by the to parameter. 
							 */
						if (enviados == SOCKET_ERROR) {
							printf("SERVIDOR UDP> Error al enviar la respuesta del servidor.");
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