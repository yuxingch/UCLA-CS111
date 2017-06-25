//	wk3 discussion
/*
	1. 1A --> client
		  --> server
	2. socket
	3. encryption

	client
	^
	socket
	v       pipes
	server  <---  shell
	        --->

	client: port #, IP address..., pre-processing, \r\n, stdout
			stdin
	server: forward the message to the pipe, detecting the special characters
	shell:	forward the message, to socket, to stdout

	encryption, before/after socket
 */

/*	Socket
	1. file descriptor, just one, can both read and write
	2. read/write
 */


//	https://www.tutorialspoint.com/unix_sockets/socket_client_example.htm
#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>

#include <string.h>

int main(int argc, char *argv[]) {
   int sockfd, portno, n;
   struct sockaddr_in serv_addr;
   struct hostent *server;
   
   char buffer[256];
   
   if (argc < 3) {
      fprintf(stderr,"usage %s hostname port\n", argv[0]);
      exit(0);
   }
	
   portno = atoi(argv[2]);
   
   /* Create a socket point */
   sockfd = socket(AF_INET, SOCK_STREAM, 0);
   
   if (sockfd < 0) {
      perror("ERROR opening socket");
      exit(1);
   }
	
   server = gethostbyname(argv[1]);
   //	Here we use the string: "localhost"
   
   if (server == NULL) {
      fprintf(stderr,"ERROR, no such host\n");
      exit(0);
   }
   
   bzero((char *) &serv_addr, sizeof(serv_addr));	//	DON'T USE THIS!
   serv_addr.sin_family = AF_INET;
   bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);	//	DON'T USE THIS!
   serv_addr.sin_port = htons(portno);
   //	Use memset, memcpy instead

   /* Now connect to the server */
   if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
      perror("ERROR connecting");
      exit(1);
   }
   
   /* Now ask for a message from the user, this message
      * will be read by server
   */

   //	Here should be replaced by our code
   printf("Please enter the message: ");
   bzero(buffer,256);
   fgets(buffer,255,stdin);
   
   //	Immediately before the write, encrypt
   /* Send message to the server */
   n = write(sockfd, buffer, strlen(buffer));
   
   if (n < 0) {
      perror("ERROR writing to socket");
      exit(1);
   }
   
   /* Now read server response */
   bzero(buffer,256);
   n = read(sockfd, buffer, 255);
    //	Immediately after the read, decrypt

   if (n < 0) {
      perror("ERROR reading from socket");
      exit(1);
   }
	
   printf("%s\n",buffer);
   return 0;
}



#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>

#include <string.h>

int main( int argc, char *argv[] ) {
   int sockfd, newsockfd, portno, clilen;
   char buffer[256];
   struct sockaddr_in serv_addr, cli_addr;
   int  n;
   
   /* First call to socket() function */
   sockfd = socket(AF_INET, SOCK_STREAM, 0);
   
   if (sockfd < 0) {
      perror("ERROR opening socket");
      exit(1);
   }
   
   /* Initialize socket structure */
   bzero((char *) &serv_addr, sizeof(serv_addr));
   portno = 5001;
   
   serv_addr.sin_family = AF_INET;
   serv_addr.sin_addr.s_addr = INADDR_ANY;
   serv_addr.sin_port = htons(portno);	
   
   /* Now bind the host address using bind() call.*/
   if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
      perror("ERROR on binding");
      exit(1);
   }
      
   /* Now start listening for the clients, here process will
      * go in sleep mode and will wait for the incoming connection
   */
   
   listen(sockfd,5);
   clilen = sizeof(cli_addr);
   
   /* Accept actual connection from the client */
   newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
   //	In server, there are multiple fds
   //	b/c multiple connections

   if (newsockfd < 0) {
      perror("ERROR on accept");
      exit(1);
   }
   
   /* If connection is established then start communicating */
   bzero(buffer,256);
   n = read( newsockfd,buffer,255 );
   
   if (n < 0) {
      perror("ERROR reading from socket");
      exit(1);
   }
   
   printf("Here is the message: %s\n",buffer);
   
   /* Write a response to the client */
   n = write(newsockfd,"I got your message",18);
   
   if (n < 0) {
      perror("ERROR writing to socket");
      exit(1);
   }
      
   return 0;
}





/*	--log=filename	*/



/*	Encryption 	*/
//	https://en.wikipedia.org/wiki/Block_cipher_mode_of_operation
//	We should use CFB in our assignment

//	mycrypt
//	Using the default algorithm is fine. (TWOFISH)

MCRYPT mcrypt_module_open( char *algorithm, char* algorithm_directory, char* mode, char* mode_directory);
//	each time you call open, you get a module.

int mcrypt_generic_init( MCRYPT td, void *key, int lenofkey, void *IV);
//	*key from keyfile
//	After calling this function you can use the descriptor for encryption or decryption (not both)
//		THIS MEANS YOU HAVE TO INITIALIZE TWO MODULES, one for encryption and one for decryption

//	Code for decryption

/* First example: Encrypts stdin to stdout using TWOFISH with 128 bit key
and CFB */
#include <mcrypt.h>
#include <stdio.h>
#include <stdlib.h>
/* #include <mhash.h> */
main() {
  MCRYPT td;
  int i;
  char *key;
  char password[20];
  char block_buffer;
  char *IV;
  int keysize=16; /* 128 bits */
  key=calloc(1, keysize);
  strcpy(password, "A_large_key");
/* Generate the key using the password */
/*  mhash_keygen( KEYGEN_MCRYPT, MHASH_MD5, key, keysize, NULL, 0, password,
strlen(password));
 */
  memmove( key, password, strlen(password));
  td = mcrypt_module_open("twofish", NULL, "cfb", NULL);
  if (td==MCRYPT_FAILED) {
     return 1;
  }
  IV = malloc(mcrypt_enc_get_iv_size(td));
/* Put random data in IV. Note these are not real random data,
 * consider using /dev/random or /dev/urandom.
 */
  /*  srand(time(0)); */
  for (i=0; i< mcrypt_enc_get_iv_size( td); i++) {
    IV[i]=rand();	//	here rand() is pseudo-random number generator
    				//	As long as the seed is the same, the string will be the same
    				//	In each of your processes, the seed is the same
  }
  i=mcrypt_generic_init( td, key, keysize, IV);
  if (i<0) {
     mcrypt_perror(i);
     return 1;
  }
  /* Encryption in CFB is performed in bytes */
  while ( fread (&block_buffer, 1, 1, stdin) == 1 ) {
      mcrypt_generic (td, &block_buffer, 1);
/* Comment above and uncomment this to decrypt */
/*    mdecrypt_generic (td, &block_buffer, 1);  */
      fwrite ( &block_buffer, 1, 1, stdout);
  }
/* Deinit the encryption thread, and unload the module */
  mcrypt_generic_end(td);
  return 0;
}

