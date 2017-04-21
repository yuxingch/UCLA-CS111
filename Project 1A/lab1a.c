//  lab1a.c

//  Name:   Yuxing Chen
//  Email:  
//  UID:    

//  Reference: https://www.gnu.org/software/libc/manual/html_node/Noncanon-Example.html

#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <poll.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>

#define SHELL       's'
#define BUFSIZE     2048 //  arbitrary? better choice?

int is_shell = 0;
pid_t pid = -1;

//  two arrays that deal with <cr> <lf>
char crlf[2] = {0x0D, 0x0A};
char lf[1] = {0x0A};

//  create two pipes
int pipe_to_child[2];
int pipe_from_child[2];

//  save terminal mode for restoration
struct termios save_terminal_mode;

//  create an array of two pollfd
//  (one describing the keyboard (stdin) and one describing the pipe that returns output from the shell.)
struct pollfd pfd[2];

//  before exit, restore the saved terminal mode
void restore_terminal_mode (void) {
    tcsetattr (STDIN_FILENO, TCSANOW, &save_terminal_mode);
    if (is_shell) { /*  print shell's exit status to stderr if we run with --shell argument */
        int status = 0;
        //  get the shell's exit status
        waitpid(0, &status, 0);
//        if(WIFEXITED(status)) {
//            fprintf(stderr, "SHELL EXIT SIGNAL=%d STATUS=%d\n", WEXITSTATUS(status), (status >> 8) & 0x00ff);
//        }
        //  lower 8 (actually 7) bits of the status is the signal, higher 8 bits of the status is the status
        fprintf(stderr, "SHELL EXIT SIGNAL=%d STATUS=%d\n", WTERMSIG(status), WEXITSTATUS(status));
    }
}

//  read input from the keyboard into a buffer.
//  write the received characters back out to the display, one character at a time.
void read_and_write(int input, int output) {
    //  create our buffer
    char buf[BUFSIZE + 1];
    ssize_t count = 0;
//    int offset = 0;
    while (1) {
        count = read(input, buf, BUFSIZE);
        for (int i = 0; i < count; i++) {
            char cur = buf[i];  //  get the current char in the buffer
            if (cur == 0x0D || cur == 0x0A) {
                //  map received <cr> or <lf> into <cr><lf>
                write(output, crlf, 2);
            }
            else if (cur == 0x04) { /*  ^D  */
                //  upon detecting a pre-defined escape sequence (^D), restore normal terminal modes and exit.
                restore_terminal_mode();
                exit(0);
            }
            else {  /*  normal case */
                write(output, buf+i, 1);
            }
        }
    }
}

void my_handler(int signum) {
    /*  Deal with ^C in --shell mode
            once received SIGINT, the child process had already been interrupted
            we need to restore the terminal mode and exit
            according to piazza, we regard this as normal execution, with exit code 0   */
    if (is_shell && signum == SIGINT) {
        restore_terminal_mode();
        exit(0);
    }
    /*  Deal with kill xxxxx    */
    if (signum == SIGPIPE) {
        restore_terminal_mode();
        exit(0);
    }
}

int main(int argc, char * argv[]) {
    
    //  Extend and refactor your program to support a --shell argument
    static struct option long_options[] = {
        {"shell",       no_argument,  0,  SHELL },
        {0,             0,                  0,  0 }  // The last element of the array has to be filled with zeros.
    };
    
    int temp = 0;
    int long_index = 0;
    
    while ( (temp = getopt_long(argc, argv, "", long_options, &long_index)) != -1) {
        switch(temp){
            case SHELL:
                //  When your program reads a ^C (0x03) from the keyboard, it should use kill(2) to send a SIGINT to the shell process.
                signal(SIGINT, my_handler);
                signal(SIGPIPE, my_handler);
                //  indicate we're in --shell mode
                is_shell = 1;
                break;
            default:    /*  invalid argument    */
                exit(1);
                break;
        }
    }
    /*
     put the keyboard (the file open on file descriptor 0) into character-at-a-time, no-echo mode (also known as non-
     canonical input mode with no echo). It is suggested that you get the current terminal modes, save them for restoration,
     and then make a copy with the following changes:
        c_iflag = ISTRIP;
        c_oflag = 0;
        c_lflag = 0;
     and these changes should be made with the TCSANOW option.
     */
    
    /* Make sure stdin is a terminal. */
    if (!isatty (STDIN_FILENO))
    {
        fprintf (stderr, "%s: Not a terminal.\n", strerror(errno));
        exit (EXIT_FAILURE);
    }
    
    //  get the parameters associated with the terminal referred to by STDIN_FILENO and store them in the termios structure.
    tcgetattr(STDIN_FILENO, &save_terminal_mode);
    //  atexit (restore_terminal_mode);
    
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
        exit(EXIT_FAILURE);
    }
    
    //  In ./lab1a mode, do the normal read/write and restore the terminal mode before exit.
    if (!is_shell) {
        read_and_write(0, 1);
        restore_terminal_mode();
        exit(0);
    }
    
    //  ./lab1a --shell mode
    if (is_shell){
        //  pipes for communication
        if (pipe(pipe_to_child) == -1) {
            fprintf(stderr, "pipe() failed.\n");
            exit(1);
        }
        if (pipe(pipe_from_child) == -1) {
            fprintf(stderr, "pipe() failed.\n");
            exit(1);
        }
        
        pid = fork();
        
        if (pid > 0) {  /*  parent process  */
            //  close the ends of pipes that we don't use in the parent process
            close(pipe_to_child[0]);
            close(pipe_from_child[1]);
            
            int rv;
            pfd[0].fd = STDIN_FILENO;
            pfd[0].events = POLLIN; // check for normal data
            
            pfd[1].fd = pipe_from_child[0];
            pfd[1].events = POLLIN; // check for normal data
            
            for(;;) {
                rv = poll(pfd, 2, 0);
                if (rv == -1) {
                    fprintf(stderr, "error in poll().\n");
                    restore_terminal_mode();
                    exit(1);
                }
                if (pfd[0].revents & POLLIN) {
                    char buf1[BUFSIZE + 1];
                    ssize_t count1 = 0;
                    //  read from keyboard to buffer
                    count1 = read(STDIN_FILENO, buf1, BUFSIZE);
                    //  write to shell and stdout while checking special characters
                    for (int k = 0; k < count1; k++) {
                        char cur = buf1[k];
                        if (cur == 0x0D || cur == 0x0A) {   /*  <cr> or <lf>    */
                            //  <cr> or <lf> should go to shell as <lf>
                            write(pipe_to_child[1], lf, 1);
                            //  <cr> or <lf> should echo as <cr><lf>
                            write(STDOUT_FILENO, crlf, 2);
                        } else if (cur == 0x04) {           /*  ^D/EOF  */
                            //  upon receiving an EOF (^D) from the terminal, close the pipe to the shell
                            close(pipe_to_child[1]);
                        } else if (cur == 0x03) {           /* ^C   */
                            //  When your program reads a ^C from the keyboard
                            //  use kill(2) to send a SIGINT to the shell process
                            kill(0, SIGINT);
                        } else {                            /* normal characters */
                            write(pipe_to_child[1], buf1+k, 1);
                            write(STDOUT_FILENO, buf1+k,1);
                        }
                    }
                }
                if (pfd[0].revents & (POLLHUP+POLLERR)) {
                    fprintf(stderr, "error in shell");
                    break;
                }
                if (pfd[1].revents & POLLIN) {
                    char buf2[BUFSIZE + 1];
                    ssize_t count2 = 0;
                    //  read from shell to buffer
                    count2 = read(pipe_from_child[0], buf2, BUFSIZE);
                    //  write to stdout while checking special characters
                    for (int k = 0; k < count2; k++) {
                        char cur = buf2[k];
                        if (cur == 0x0D || cur == 0x0A) {
                            //  <cr> or <lf> should echo as <cr><lf>
                            write(STDOUT_FILENO, crlf, 2);
                            //                        } else if (cur == 0x04) {
                            //                            close(to_child_pipe[0]);
                            //                            close(to_child_pipe[1]);
                            //                            close(from_child_pipe[0]);
                            //                            close(from_child_pipe[1]);
                            //                            kill(pid, SIGHUP);
                            //                            restore_terminal_mode();
                            //                            exit(0);
                        } else {    /* normal characters */
                            write(STDOUT_FILENO, buf2+k, 1);
                        }
                    }
                }
                //  upon receiving EOF or polling-error from the shell, we know that we have all of the output the shell has generated
                if (pfd[1].revents & (POLLHUP+POLLERR)) {
                    close(pipe_from_child[0]);  //  close another end of the pipe
                    restore_terminal_mode();
                    exit(0);
                }

            }
        } else if (pid == 0) {  /*  child process (shell)   */
            //  close the ends of pipes that we don't use in the child process
            close(pipe_to_child[1]);
            close(pipe_from_child[0]);
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
                exit(1);
            }
        } else {
            fprintf(stderr, "fork() failed.\n");
            exit(1);
        }
    }

//    read_and_write(0, 1);
    restore_terminal_mode();
    exit(0);
}
