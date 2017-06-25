//	lab1a
//	discussion

/*	chapter 17 of the GNU Library
	read(0, ...) reading from stdin
	canonical: as long as you press the return key, the read function can get the info in the buffer
	noncanonical: you cannot "erase", since once you type a character, you'll get it.

	ctrl+c: your terminal interpret it, send a signal to the process
 */

void read_and_write() {
	while (...) {
		count = read(0, buf, 1024);
		for (int i = 0; i < count; i++) {
			if (buf[i] == 0x0D || buf[i] == 0x0A) {
				write(1, ..., 2); // <cr><lf>.
			}
			else if ( == ^D) {

			}
			write(1, buf[i], 1);
		}
	}
}

int example () {
	pipe(my_pipe);
	print("a");
	int cid = fork();
	print ("b");
	if (cid > 0){	//	in child process: cid == 0; in parent process, cid == pid of child
		print("c");
	}
	else if (cid == 0) {
		print("d");
	}
}

/* you have only one pipe. you have two copies of my_pipe array.
parent					|	child
my_pipe					|	my_pipe
fork()
write(...my_pipe[1]...)	|	read(...my_pipe[0]...)
 */

//	child process will hang forver

close(my_pipe[0]); //	in parent process
close(my_pipe[1]); //	in child process
//	Always close FDs you DO NOT use.


you have 8 fd after you create two pipes and call fork().
you have to close 4 fds.
now you have only 2 fds open in each process



execvp(cat);
printf(); //	if it succeeds, this line of code will no longer be executed.

dup2();	//	call this three times before execvp
//	dup2(pipe_fd, 0/1/2);
//	remember to close the two remained fds.
//	you have 3 fds at the write end of p2. what should we do?
close(p1[0]);
close(p2[1]);
execvp(/bin/bash);

waitpid(pid_t pid, int *wstatus, int options);
//	wstatus, the lower 2 bytes.
//	existatus(8 bits)core dump flag(1 bit)signal(7 bits)


//	only POLLIN, don't need POLLOUT
