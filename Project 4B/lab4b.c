//
//  main.c
//  lab4b
//
//  Created by Yuxing Chen on 5/20/17.
//  Copyright © 2017 Yuxing Chen. All rights reserved.
//

#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <ctype.h>
#include <math.h>
#include <pthread.h>
#include <time.h>

#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <mraa/aio.h>
#include <mraa/gpio.h>

#define TEMPFH  'F'
#define TEMPCT  'C'
#define TIMEPERIOD  'p'
#define SCALE   's'
#define LOGON   'l'
#define MINCMDSIZE  3

const int B = 4275;
const int R0 = 100000;
char scale = TEMPFH;

int is_log = 0;
int period = 1;
int other_buffer_size = 32;
int time_buffer_size = 9;
FILE* my_log;

sig_atomic_t volatile run_flag = 1;
mraa_aio_context rotary;
mraa_gpio_context button;

void *thread_temperature () {
    uint16_t value;
    rotary = mraa_aio_init(0);
    
    time_t time_store = 0;
    
    while(1) {
        //  get time of the sample the local timezone
        //  https://www.tutorialspoint.com/c_standard_library/c_function_localtime.htm
        time_t raw_time;
        time(&raw_time);
        
        //        fprintf(stdout, "%ld, %ld\n", raw_time, time_store);
        double diff = difftime(raw_time, time_store);
        //        fprintf(stdout, "%f\n", diff);
        if (diff > (double) period || diff == (double) period) {
            time_store = time(NULL);
            char time_buf[time_buffer_size];
            struct tm *info;
            info = localtime( &raw_time );
            //  17:25:58
            //  http://man7.org/linux/man-pages/man3/strftime.3.html
            strftime(time_buf, sizeof(time_buf), "%2H:%2M:%2S", info);
            
            //  get the temperature
            value = mraa_aio_read(rotary);
            float R = 1023.0/((float) value) - 1.0;
            R = R0*R;
            float temperature = 1.0/(log(R/R0)/B+1/298.15)-273.15;
            //  If SCALE=F, generate report in degrees Fahrenheit
            if (scale == TEMPFH)
                //  http://www.rapidtables.com/convert/temperature/how-celsius-to-fahrenheit.htm
                temperature = temperature * 1.8 + 32.0;
            
            if (run_flag) {
                /**
                 * generate report for each sample
                 *      time of the sample (e.g. 17:25:58) the local timezone
                 *      a single blank/space
                 *      a decimal temperature in degrees and tenths (e.g. 98.6)
                 *      a newline character (\n)
                 * writes that report to the stdout (fd 1)
                 * appends that report to a logfile if --log=filename
                 */
                //  writes to stdout
                //  precision: https://www.tutorialspoint.com/c_standard_library/c_function_fprintf.htm
                if (temperature < 10 ){
                    fprintf(stdout, "%s 0%2.1f\n", time_buf, temperature);
                    //  appends to a logfile if specified
                    if (is_log)
                        fprintf(my_log, "%s 0%2.1f\n", time_buf, temperature);
                } else {
                    fprintf(stdout, "%s %4.1f\n", time_buf, temperature);
                    //  appends to a logfile if specified
                    if (is_log)
                        fprintf(my_log, "%s %4.1f\n", time_buf, temperature);
                }
            }
        }
    }
}

void press_button() {
    run_flag = 0;
    char time_buf[time_buffer_size];
    time_t raw_time;
    struct tm *info;
    time(&raw_time);
    info = localtime( &raw_time );
    //  17:25:58
    //  http://man7.org/linux/man-pages/man3/strftime.3.html
    strftime(time_buf, sizeof(time_buf), "%2H:%2M:%2S", info);
    //    fprintf(stdout, "%s %s", time_buf, "SHUTDOWN");
    if (is_log) {
        fprintf(my_log, "%s %s\n", time_buf, "SHUTDOWN");
        fclose(my_log);
    }
    mraa_gpio_close(button);
    mraa_aio_close(rotary);
    exit(0);
}

void *thread_button () {
    button = mraa_gpio_init(3);
    mraa_gpio_dir(button, MRAA_GPIO_IN);
    while (! mraa_gpio_read(button))
        ;
    press_button();
    return NULL;
}

void* thread_read () {
    sleep(1);
    while(1) {
        char buf[16];
        int pos = 0;
        int temp = 0;
        while ((temp = (read(0, buf+pos, 1)) > 0) && buf[pos] != '\n')
            pos++;
        
        if (temp == 0) {
            //  EOF
            if (is_log)
                fclose(my_log);
            mraa_gpio_close(button);
            mraa_aio_close(rotary);
            exit(1);
        }
        
        char cmd[pos];
        strncpy(cmd, buf, pos);
        cmd[pos] = '\0';
        if (!strcmp(cmd, "OFF")) {
            run_flag = 0;
            if (is_log) {
                fprintf(my_log, "%s", "OFF\n");
                char time_buf[time_buffer_size];
                time_t raw_time;
                struct tm *info;
                time(&raw_time);
                info = localtime( &raw_time );
                //  17:25:58
                //  http://man7.org/linux/man-pages/man3/strftime.3.html
                strftime(time_buf, sizeof(time_buf), "%2H:%2M:%2S", info);
                fprintf(my_log, "%s %s\n", time_buf, "SHUTDOWN");
                fclose(my_log);
            }
            mraa_gpio_close(button);
            mraa_aio_close(rotary);
            exit(0);
        } else if (!strcmp(cmd, "STOP")) {
            if (is_log)
                fprintf(my_log, "%s", "STOP\n");
            run_flag = 0;
        } else if (!strcmp(cmd, "START")) {
            if (is_log)
                fprintf(my_log, "%s", "START\n");
            run_flag = 1;
        } else if (!strcmp(cmd, "SCALE=F")) {
            if (is_log)
                fprintf(my_log, "%s", "SCALE=F\n");
            scale = TEMPFH;
        } else if (!strcmp(cmd, "SCALE=C")) {
            if (is_log)
                fprintf(my_log, "%s", "SCALE=C\n");
            scale = TEMPCT;
        } else if (!strncmp(cmd, "PERIOD=", 7)) {
            for (int i = 7; i < strlen(cmd); i++) {
                if (!isdigit(cmd[i])) {
                    fprintf(stderr, "Error: Invalid command. \n");
                    if (is_log) {
                        fprintf(my_log, "%s\n%s", cmd, "Error: Invalid command.\n");
                        fclose(my_log);
                    }
                    mraa_gpio_close(button);
                    mraa_aio_close(rotary);
                    exit(1);
                }
            }
            char* temp = cmd+7;
            period = atoi(temp);
            if (is_log)
                fprintf(my_log, "%s%d\n", "PERIOD=", period);
        } else {
            fprintf(stderr, "Error: Invalid command.\n");
            if (is_log) {
                fprintf(my_log, "%s\n%s", cmd, "Error: Invalid command.\n");
                fclose(my_log);
            }
            mraa_gpio_close(button);
            mraa_aio_close(rotary);
            exit(1);
        }
    }
}

int main(int argc, char * argv[]) {
    static struct option long_options[] = {
        {"period",  required_argument,  0,  TIMEPERIOD },
        {"scale",   required_argument,  0,  SCALE },
        {"log",     required_argument,  0,  LOGON   },
        {0,         0,                  0,  0 }  // The last element of the array has to be filled with zeros.
    };
    
    int opt;
    int long_index = 0;
    int size;
    char* filename;
    
    while ( (opt = getopt_long(argc, argv, "", long_options, &long_index)) != -1) {
        switch (opt) {
            case TIMEPERIOD:
                period = atoi(optarg);
                break;
            case SCALE:
                scale = optarg[0];
                break;
            case LOGON: //  http://stackoverflow.com/questions/2445370/how-to-create-custom-filenames-in-c
                is_log = 1;
                size = strlen(optarg);
                filename = malloc(sizeof(char) * (size+1));
                sprintf(filename, "%s", optarg);
                my_log = fopen(filename, "w");
                if (my_log == NULL) {
                    fprintf(stderr, "Error in creating log file.\n");
                    exit(1);
                }
                break;
            default:
                fprintf(stderr, "Error: Bad arguments are encountered.\n");
                exit(1);
                break;
        }
    }
    
    pthread_t tid1, tid2, tid3;
    if (pthread_create(&tid1, NULL, thread_temperature, NULL)) {
        fprintf(stderr, "Error: pthread_create failed.\n");
        if (is_log)
            fclose(my_log);
        exit(1);
    }
    if (pthread_create(&tid2, NULL, thread_button, NULL)) {
        fprintf(stderr, "Error: pthread_create failed.\n");
        if (is_log)
            fclose(my_log);
        mraa_aio_close(rotary);
        exit(1);
    }
    if (pthread_create(&tid3, NULL, thread_read, NULL)) {
        fprintf(stderr, "Error: pthread_create failed.\n");
        if (is_log)
            fclose(my_log);
        mraa_aio_close(rotary);
        mraa_gpio_close(button);
        exit(1);
    }
    
    if (pthread_join(tid1, NULL) != 0 || pthread_join(tid2, NULL) != 0 || pthread_join(tid3, NULL) != 0) {
        fprintf(stderr, "Error: Failure in pthread_join.\n");
        if (is_log)
            fclose(my_log);
        mraa_gpio_close(button);
        mraa_aio_close(rotary);
        exit(1);
    }
    
    if (is_log)
        fclose(my_log);
    mraa_gpio_close(button);
    mraa_aio_close(rotary);
    exit(0);
}



