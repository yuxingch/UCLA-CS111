//  lab1b-server.c
//
//  Created by Yuxing Chen on 4/21/17.
//  Copyright © 2017 Yuxing Chen. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <getopt.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <poll.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <netdb.h>
#include <netinet/in.h>
#include <mcrypt.h>
#include <string.h>

#define PORT    'p'
#define ENCRYPT 'e'
#define BUFSIZE 2048


int is_encrypt = 0;
int keyfd;
MCRYPT encryptfd, decryptfd;


pid_t pid = -1;

//  two arrays that deal with <cr> <lf>
char crlf[2] = {0x0D, 0x0A};
char lf[1] = {0x0A};

//  create two pipes
int pipe_to_child[2];
int pipe_from_child[2];


//  create an array of two pollfd
//  (one describing the keyboard (stdin) and one describing the pipe that returns output from the shell.)
struct pollfd pfd[2];

//  before exit, restore the saved terminal mode
void prepare_to_exit (void) {
    if (is_encrypt) {
        mcrypt_generic_deinit(encryptfd);
        mcrypt_module_close(encryptfd);
        mcrypt_generic_deinit(decryptfd);
        mcrypt_module_close(decryptfd);
    }
    
    int status = 0;
    //  get the shell's exit status
    waitpid(0, &status, 0);
    fprintf(stderr, "SHELL EXIT SIGNAL=%d STATUS=%d\n", WTERMSIG(status), WEXITSTATUS(status));
}

void my_handler(int signum) {
    /*  Deal with ^C in --shell mode
     once received SIGINT, the child process had already been interrupted
     we need to restore the terminal mode and exit
     according to piazza, we regard this as normal execution, with exit code 0   */
    if (signum == SIGINT) {
        prepare_to_exit();
        exit(0);
    }
    /*  Deal with kill xxxxx    */
    if (signum == SIGPIPE) {
        prepare_to_exit();
        exit(0);
    }
    
    if (signum == SIGTERM) {
        prepare_to_exit();
        exit(0);
    }
}

int main(int argc, char *argv[]) {
    
    
    //  Extend and refactor your program to support a --shell argument
    static struct option long_options[] = {
        {"port",    required_argument,  0,  PORT },
        {"encrypt", required_argument,  0,  ENCRYPT},
        {0,             0,                  0,  0 }  // The last element of the array has to be filled with zeros.
    };
    
    int temp = 0;
    int long_index = 0;
    int n_port = 0;
    int keysize;
    char *key;
    char *IV1, *IV2;
    
    while ( (temp = getopt_long(argc, argv, "", long_options, &long_index)) != -1) {
        switch(temp){
            case PORT:  //  get the port number
                n_port = atoi(optarg);  //  converts the string argument str to an integer using atoi()
                signal(SIGINT, my_handler);
                signal(SIGPIPE, my_handler);
                signal(SIGTERM, my_handler);
                break;
            case ENCRYPT:
                keyfd = open(optarg, O_RDONLY);
                struct stat st;
                fstat(keyfd, &st);
                keysize = (int) st.st_size;
                key = (char*) malloc(keysize * sizeof(char));
                read(keyfd, key, keysize);
                close(keyfd);
                /*  initialization  */
                /*  encryption module   */
                encryptfd = mcrypt_module_open("twofish", NULL, "cfb", NULL);
                if (encryptfd == MCRYPT_FAILED) {
                    fprintf(stderr, "Error: Fail to open a module.\n");
                    exit(1);
                }
                IV1 = malloc(mcrypt_enc_get_iv_size(encryptfd));
                for (int i=0; i< mcrypt_enc_get_iv_size(encryptfd); i++) {
                    IV1[i]=rand();
                }
                if (mcrypt_generic_init(encryptfd, key, keysize, IV1) < 0) {
                    fprintf(stderr, "Error: Fail to initialize encrypt.\n");
                    exit(1);
                }
                /*  decryption module   */
                decryptfd = mcrypt_module_open("twofish", NULL, "cfb", NULL);
                if (decryptfd == MCRYPT_FAILED) {
                    fprintf(stderr, "Error: Fail to open a module.\n");
                    exit(1);
                }
//                IV2 = malloc(mcrypt_enc_get_iv_size(decryptfd));
//                for (int i=0; i< mcrypt_enc_get_iv_size(decryptfd); i++) {
//                    IV2[i]=rand();
//                }
                if (mcrypt_generic_init(decryptfd, key, keysize, IV1) < 0) {
                    fprintf(stderr, "Error: Fail to initialize decrypt.\n");
                    exit(1);
                }
                is_encrypt = 1;
                break;
            default:    /*  invalid argument    */
                fprintf(stderr, "Usage: ./lab1b-server --port=# [--encrypt=file]");
                exit(1);
                break;
        }
    }
    
    int sockfd, newsockfd, clilen;
    struct sockaddr_in serv_addr, cli_addr;
    
    /* First call to socket() function */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    
    if (sockfd < 0) {
        perror("ERROR opening socket");
        prepare_to_exit();
        exit(1);
    }
    
    /* Initialize socket structure */
    memset((char *) &serv_addr, 0, sizeof(serv_addr));
//    portno = 5001;
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(n_port);
    
    /* Now bind the host address using bind() call.*/
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR on binding");
        prepare_to_exit();
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
        prepare_to_exit();
        exit(1);
    }
    
    /* If connection is established then start communicating */
    
    //  pipes for communication
    if (pipe(pipe_to_child) == -1) {
        fprintf(stderr, "pipe() failed.\n");
        prepare_to_exit();
        exit(1);
    }
    if (pipe(pipe_from_child) == -1) {
        fprintf(stderr, "pipe() failed.\n");
        prepare_to_exit();
        exit(1);
    }
    
    pid = fork();
    
    if (pid > 0) {
        close(pipe_to_child[0]);
        close(pipe_from_child[1]);
        
        int rv;
        pfd[0].fd = newsockfd;
        pfd[0].events = POLLIN; // check for normal data
        
        pfd[1].fd = pipe_from_child[0];
        pfd[1].events = POLLIN; // check for normal data
        
        for(;;) {
            rv = poll(pfd, 2, 0);
            if (rv == -1) {
                fprintf(stderr, "error in poll().\n");
                prepare_to_exit();
                exit(1);
            }
            if (pfd[0].revents & POLLIN) {
                char buf1[BUFSIZE + 1];
                ssize_t count1 = 0;
                //  read from socket to buffer
                count1 = read(newsockfd, buf1, BUFSIZE);
                if (count1 < 0) {
                    fprintf(stderr, "Error in read.\n");
                    kill(0, SIGTERM);
                } else if (count1 == 0) {
                    kill(0, SIGTERM);
                }
                //  write to shell and stdout while checking special characters
                for (int k = 0; k < count1; k++) {
                    if (is_encrypt) {
                        mdecrypt_generic (decryptfd, buf1+k, 1);
                    }
                    char cur = buf1[k];
                    if (cur == 0x0D || cur == 0x0A) {   /*  <cr> or <lf>    */
                        //  <cr> or <lf> should go to shell as <lf>
                        write(pipe_to_child[1], lf, 1);
                    } else if (cur == 0x04) {           /*  ^D/EOF  */
                        //  upon receiving an EOF (^D) from the terminal, close the pipe to the shell
                        close(pipe_to_child[1]);
                    } else if (cur == 0x03) {           /* ^C   */
                        //  When your program reads a ^C from the keyboard
                        //  use kill(2) to send a SIGINT to the shell process
                        kill(0, SIGINT);   //  sigint
                    } else {                            /* normal characters */
                        write(pipe_to_child[1], buf1+k, 1);
                    }
                }
            }
            if (pfd[0].revents & (POLLHUP+POLLERR)) {
                fprintf(stderr, "Error in shell.\n");
                break;
            }
            if (pfd[1].revents & POLLIN) {
                char buf2[BUFSIZE + 1];
                ssize_t count2 = 0;
                //  read from shell to buffer
                count2 = read(pipe_from_child[0], buf2, BUFSIZE);
                //  write to stdout while checking special characters
                for (int k = 0; k < count2; k++) {
                    if (is_encrypt) {
                        mcrypt_generic (encryptfd, buf2+k, 1);
                    }
                    write(newsockfd, buf2+k, 1);
//                    char cur = buf2[k];
//                    if (cur == 0x0D || cur == 0x0A) {
//                        //  <cr> or <lf> should echo as <cr><lf>
//                        write(newsockfd, crlf, 2);
//                    } else {    /* normal characters */
//                        write(newsockfd, buf2+k, 1);
//                    }
                }
            }
            //  upon receiving EOF or polling-error from the shell, we know that we have all of the output the shell has generated
            if (pfd[1].revents & (POLLHUP+POLLERR)) {
                close(pipe_from_child[0]);  //  close another end of the pipe
                //  ... print out exit status ?
                prepare_to_exit();
                exit(0);
            }
        }
    } else if (pid == 0) {
        close(pipe_to_child[1]);
        close(pipe_from_child[0]);
        
        //  dup
        dup2(pipe_to_child[0], STDIN_FILENO);
        dup2(pipe_from_child[1], STDOUT_FILENO);
        dup2(pipe_from_child[1], STDERR_FILENO);
        
        //  finish all jobs, close all ends of pipes
        close(pipe_to_child[0]);
        close(pipe_from_child[1]);
        
        //  an array of pointers
        char *execvp_argv[2];
        //  The first argument should point to the filename associated with the file being executed
        execvp_argv[0] = "/bin/bash";
        //  The array of pointers must be terminated by a NULL pointer.
        execvp_argv[1] = NULL;
        if (execvp("/bin/bash", execvp_argv) == -1) {
            fprintf(stderr, "execvp() failed.\n");
            prepare_to_exit();
            exit(1);
        }
    } else {
        fprintf(stderr, "fork() failed.\n");
        prepare_to_exit();
        exit(1);
    }
    close(newsockfd);
    prepare_to_exit();
    exit(0);
}
