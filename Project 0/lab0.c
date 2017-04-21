//  main.c

//  NAME: Yuxing Chen
//  EMAIL: 
//  ID: 

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <string.h>

//  follow the format given in the discussion section

#define INPUT       'i'
#define OUTPUT      'o'
#define SEGFAULT    's'
#define CATCH       'c'
#define STOREVALUE  'a'

void my_handler (int signum) {
//    char my_message[32] = "Catches the segmentation fault.\n";
//    write(1, my_message, 31);
    fprintf(stderr, "%s: Catches the segmentation fault.\n", strerror(errno));
    exit(4);
}

int main(int argc, char * argv[]) {
    /*  ARGUMENT PARSING with getopt_long(3)
        This follows from the example provided in: https://linux.die.net/man/3/getopt_long , in which
        it illustrates the use of getopt_long() with most of its features.
     
        no_argument if the option does not take an argument;
        required_argument if the option requires an argument; 
        optional_argument if the option takes an optional argument.
        
        --input=filename ...: required_argument
        --output=filename ...: required_argument
        --segfault ...: no_argument
        --catch ...: no_argument
     */
    static struct option long_options[] = {
        {"input",       required_argument,  0,  INPUT },
        {"output",      required_argument,  0,  OUTPUT },
        {"segfault",    no_argument,        0,  SEGFAULT },
        {"catch",       no_argument,        0,  CATCH },
        {0,             0,                  0,  0 }  // The last element of the array has to be filled with zeros.
    };
    
    int temp = 0;
    int long_index = 0;
    int fd0, fd1;
    int flag = 0;
    while ( (temp = getopt_long(argc, argv, "i:o:sc", long_options, &long_index)) != -1){
        switch(temp){
            case INPUT: // use the specified file as standard input (making it the new fd0)
                fd0 = open(optarg, O_RDONLY);   //  int open (const char *filename, int flags[, mode_t mode])
                if (fd0 < 0){   //  If you are unable to open the specified input file,
                                //  report the failure (on stderr, file descriptor 2) using fprintf(3),
                                //  and exit(2) with a return code of 2.
                    fprintf(stderr, "%s: Unable to open the specified input file.\n", strerror(errno));
                    exit(2);
                } else {
                    close(0);
                    dup(fd0);
                    close(fd0);
                }
                break;
            case OUTPUT:
                fd1 = creat(optarg, S_IRWXU);
                if (fd1 < 0){
                    fprintf(stderr, "%s: Unable to create the specified output file.\n", strerror(errno));
                    exit(3);
                } else {
                    close(1);
                    dup(fd1);
                    close(fd1);
                }
                break;
            case SEGFAULT:
                flag = 1;
                break;
            case CATCH:
                signal(SIGSEGV, my_handler);
                break;
            default:
                fprintf(stderr, "%s: Usage: ./lab0 --input=filename --output=filename2\n", strerror(errno));
                exit(1);
        }
    }
    
    //  sets a char * pointer to NULL and then stores through the null pointer
    char* ptr = NULL;
    if (flag){
        *ptr = STOREVALUE;
    }
    
    ssize_t ret;
    int buf_size = 2048;
    char* buffer[buf_size + 1];
    ret = read(0, &buffer, buf_size);
    while (ret > 0) {
        write(1, &buffer, ret);
        ret = read(0, &buffer, buf_size);
    }
    
    exit(0);
}
