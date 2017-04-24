//  lab1b-client.c
//
//  Name:   Yuxing Chen
//  Email:  
//  UID:    
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
#define LOG     'l'
#define ENCRYPT 'e'
#define BUFSIZE 2048
#define SENT    14
#define RECEIVED    18

//  save terminal mode for restoration
struct termios save_terminal_mode;
int logfd;
int is_log = 0;
int is_encrypt = 0;
int keyfd;
MCRYPT encryptfd, decryptfd;


pid_t pid = -1;

//  two arrays that deal with <cr> <lf>
char crlf[2] = {0x0D, 0x0A};
char lf[1] = {0x0A};


//  create an array of two pollfd
//  (one describing the keyboard (stdin) and one describing the pipe that returns output from the shell.)
struct pollfd pfd[2];

//  before exit, restore the saved terminal mode
void restore_terminal_mode (void) {
    tcsetattr (STDIN_FILENO, TCSANOW, &save_terminal_mode);
    if (is_encrypt) {
        mcrypt_generic_deinit(encryptfd);
        mcrypt_module_close(encryptfd);
        mcrypt_generic_deinit(decryptfd);
        mcrypt_module_close(decryptfd);
    }
    if (is_log) {
        close(logfd);
    }
//    int status = 0;
//    //  get the shell's exit status
//    waitpid(0, &status, 0);
//    fprintf(stderr, "SHELL EXIT SIGNAL=%d STATUS=%d\n", WTERMSIG(status), WEXITSTATUS(status));
}


int main(int argc, char *argv[]) {
    
    
    //  Extend and refactor your program to support a --shell argument
    static struct option long_options[] = {
        {"port",    required_argument,  0,  PORT },
        {"log",     required_argument,  0,  LOG  },
        {"encrypt", required_argument,  0,  ENCRYPT},
        {0,             0,                  0,  0 }  // The last element of the array has to be filled with zeros.
    };
    
    int opt = 0;
    int long_index = 0;
    int n_port = 0;
    int keysize;
    char *key;
    char *IV1, *IV2;
    
    while ( (opt = getopt_long(argc, argv, "", long_options, &long_index)) != -1) {
        switch(opt){
            case PORT:  //  get the port number
                n_port = atoi(optarg);  //  converts the string argument str to an integer using atoi()
                break;
            case LOG:
                //  The system call creat is provided to create new files, or to re-write old ones.
                logfd = creat(optarg, S_IRWXU);    //  00700 user (file owner) has read, write and execute permission
                if (logfd < 0){
                    fprintf(stderr, "Error: Fail to create the log file.\n");
                    exit(1);
                }
                is_log = 1;
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
                fprintf(stderr, "Usage: ./lab1b-client --port=# [--log=logfile] [--encrypt=file]");
                exit(1);
                break;
        }
    }
    
    //  get the parameters associated with the terminal referred to by STDIN_FILENO and store them in the termios structure.
    tcgetattr(STDIN_FILENO, &save_terminal_mode);
    
    //  then make a copy with the following changes
    struct termios changed_terminal_mode;
    
    tcgetattr(STDIN_FILENO, &changed_terminal_mode);
    /*  changes specified in our spec */
    changed_terminal_mode.c_iflag = ISTRIP;
    changed_terminal_mode.c_oflag = 0;
    changed_terminal_mode.c_lflag = 0;
    
    changed_terminal_mode.c_lflag &= ~(ICANON|ECHO); /* Clear ICANON and ECHO. */
    changed_terminal_mode.c_cc[VMIN] = 1;
    changed_terminal_mode.c_cc[VTIME] = 0;
    
    int flag = 0;
    flag = tcsetattr(STDIN_FILENO, TCSANOW, &changed_terminal_mode);
    if (flag < 0) { //  indicate the failure
        fprintf(stderr, "%s: Failure: the requested changes could not be successfully carried out.\n", strerror(errno));
        restore_terminal_mode();
        exit(EXIT_FAILURE);
    }
    
    /*      Socket Part     */
    
    int sockfd;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    
    /* Create a socket point */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    
    if (sockfd < 0) {
        fprintf(stderr, "ERROR, fail to open socket\n");
        restore_terminal_mode();
        exit(1);
    }
    
    server = gethostbyname("localhost");
    //	Here we use the string: "localhost"
    
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        restore_terminal_mode();
        exit(0);
    }
    
    //  bzero((char *) &serv_addr, sizeof(serv_addr));	DON'T USE THIS!
    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    //  bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);	DON'T USE THIS!
    memcpy((char *)&serv_addr.sin_addr.s_addr, (char *)server->h_addr, server->h_length);
    serv_addr.sin_port = htons(n_port);
    //	Use memset, memcpy instead
    
    /* Now connect to the server */
    if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR connecting");
        restore_terminal_mode();
        exit(1);
    }
    
    /* Now ask for a message from the user, this message
     * will be read by server
     */
    
    //	Here should be replaced by our code
    
    /*  send input from the keyboard to the socket (while echoing to the display)
        input from the socket to the display    */
    //  read from keyboard
    int rv;
    pfd[0].fd = STDIN_FILENO;
    pfd[0].events = POLLIN;
    pfd[1].fd = sockfd;
    pfd[1].events = POLLIN;
    
    for (;;) {
        rv = poll(pfd, 2, 0);
        if (rv == -1) {
            fprintf(stderr, "error in poll().\n");
            restore_terminal_mode();
            exit(1);
        }
        if (pfd[0].revents & POLLIN) {
            char buffer[BUFSIZE + 1];
            ssize_t count1 = 0;
            count1 = read(STDIN_FILENO, buffer, BUFSIZE);
            for (int k = 0; k < count1; k++) {
                //  echoing to the display while storing for sending to socket
                char cur = buffer[k];
                
                if (cur == 0x0D || cur == 0x0A) {   /*  <cr> or <lf>    */
                    write(STDOUT_FILENO, crlf, 2);
                } else {                            /* normal characters */
                    write(STDOUT_FILENO, buffer+k,1);
                }
                if (is_encrypt){
                    mcrypt_generic (encryptfd, buffer+k, 1);
                }
                write(sockfd, buffer+k, 1);
                
                /*  write to log    */
                if (is_log) {
                    char my_log[SENT] = "SENT 1 bytes: ";
                    write(logfd, my_log, SENT);
                    write(logfd,  buffer+k, 1);
                    write(logfd, "\n",1);
                }
            }
        }
        
        if (pfd[0].revents & (POLLHUP+POLLERR)) {
            fprintf(stderr, "error in shell\n");
            break;
        }
        
        if (pfd[1].revents & POLLIN) {
            char buffer2[BUFSIZE + 1];
            /* Now read server response */
            ssize_t n;
            n = read(sockfd, buffer2, BUFSIZE);
            if (n == 0) {
                break;
            }
            //	Immediately after the read, decrypt
            if (n < 0) {
                perror("ERROR reading from socket");
                restore_terminal_mode();
                exit(1);
            }
            for (int k = 0; k < n; k++) {
                if (is_log)
                {
                    char my_log[RECEIVED] = "RECEIVED 1 bytes: ";
                    write(logfd, my_log, RECEIVED);
                    write(logfd,  buffer2+k, 1);
                    write(logfd, "\n",1);
                }
                
                if (is_encrypt) {
                    mdecrypt_generic (decryptfd, buffer2+k, 1);
                }
                char cur = buffer2[k];
                if (cur == 0x0D || cur == 0x0A) {
                    write(STDOUT_FILENO, crlf, 2);
                } else {
                    write(STDOUT_FILENO, buffer2+k,1);
                }
            }
        }
        if (pfd[1].revents & (POLLHUP+POLLERR)) {
            fprintf(stderr, "error in shell\n");
            break;
        }
    }
    close(sockfd);
    restore_terminal_mode();
    exit(0);
}
