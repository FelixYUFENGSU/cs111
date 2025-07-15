#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

int pipeArg(int argc,char *argv[]){

//not enough arguments 
if(argc < 2){
	 errno = 0;
	 perror("Not enough arguments");
	 exit(errno);
	 }
//no need for pipe if we only have one argument 
if(argc == 2){
	if(execlp(argv[1],argv[1],NULL) == -1){
		perror("Failed");
		exit(errno);
	}
	return 0;
}

int fd[2];
int input_fd = 0;
int err = 0;

for(int i = 1; i < argc -1; i++){
	errno = 0;
	err = pipe(fd);
	if(err == -1){
		perror("Pipe failed");
		return errno;
	}
	pid_t pid = fork();
	errno = 0;
	if(pid < 0){
		perror("fork failed");
		return errno;
	}
	//child process
	else if(pid == 0){
		//redirect input
		if(input_fd != 0){
			dup2(input_fd,STDIN_FILENO);
			close(input_fd);
		}

		//redirect output if not last command 
		if(i < argc - 1){
			close(fd[0]);
			dup2(fd[1], STDOUT_FILENO);
			close(fd[1]);
		}
		execlp(argv[i], argv[i], (char*)0);
		perror("fail execlp");
		exit(errno);
	}
	else{
		//parent process
		close(fd[1]);
		errno = 0;
		err = dup2(fd[0], 0);
		if(err == -1){
			perror("fail dup2");
			return errno;
		}
		close(fd[0]);

		errno = 0;
		err = wait(NULL);
		if(err == -1){
			perror("fail wait");
			return errno;
		}
	}
}

errno = 0;
err = execlp(argv[1], argv[1], (char*)0);
if(err == -1){
	perror("fail execlp");
	return errno;
}

return 0;
}

int main(int argc, char *argv[])
{
 int val;
 val = pipeArg(argc,argv);

return val;
}
