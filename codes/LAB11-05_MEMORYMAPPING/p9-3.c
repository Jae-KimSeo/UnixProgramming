#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include <ftw.h>
#include <stdlib.h>
#include <sys/mman.h>

#define BUFSIZE 512

void do_child(int i, int *addr){
    	int fd, child, j;
    
    	child = *(addr + 0);
    
        while(i != child){
		sleep(2);
		printf(".........................pid : %d waiting\n", i);

	        child = *(addr + 0);
        }

	for(j=0;j<5;j++){
                sleep(1);
        	printf("%dth child pid = %d\n", i, getpid());
        }
        *(addr + 0) = child - 1;

	exit(0);
}

int main(int argc, char **argv){

        int i, status, fd;
        pid_t pid[3];
	int *addr;

        fd = open("temp", O_RDWR | O_CREAT | O_TRUNC, 0600);
        addr = mmap(NULL, BUFSIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	ftruncate(fd, sizeof(int));
	*(addr + 0) = 2;

        for(i=0;i<3;i++){
                pid[i] =fork();
                if(pid[i] == 0){
                        do_child(i, addr);
                }
        }

        for(i=0;i<3;i++){
                wait(&status);
        }

        exit(0);
}
