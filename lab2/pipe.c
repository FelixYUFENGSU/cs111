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

int input_fd = 0;
pid_t pids[argc -1];

for(int i = 1; i < argc; i++){
	int fd[2];
	
	if(i < argc -1 && pipe(fd) == -1){
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
			dup2(fd[1], STDOUT_FILENO);
			close(fd[1]);
			close(fd[0]);
		}
		execlp(argv[i], argv[i], (char*)0);
		perror("fail execlp");
		exit(errno);
	}
	if(input_fd != 0){
		close(input_fd);
	}
	if(i < argc -1){
		close(fd[1]);
		input_fd = fd[0];
	}

	pids[i-1] = pid;
	
}

int retval = 0;
for(int i = 0; i < argc -1; i++){
	int status;
	waitpid(pids[i], &status, 0);
	if(retval == 0 && WIFEXITED(status)) 
		retval = WEXITSTATUS(status);
	if(retval == 0 && WIFSIGNALED(status))
		retval = 128 + WTERMSIG(status);
}

return retval;

}

int main(int argc, char *argv[])
{

 return pipeArg(argc,argv);
}
