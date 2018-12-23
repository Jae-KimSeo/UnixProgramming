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
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <signal.h>

#define BUFSIZE 512

void catchusr(int);
void catchusr2(int);
void catchalarm(int);

void cat_m(char **s){
    char buffer[BUFSIZE];
    int nread, fd = open(s[1], O_RDONLY);

    while((nread = read(fd, buffer, BUFSIZE)) > 0){
        write(1, buffer, nread);
    }

    close(fd);
}

void cd_m(char **s){
    chdir(s[1]);
}

void cp_m(char **s){
    char buffer[BUFSIZE];
    int fd1, fd2, nread;

    fd1 = open(s[1], O_RDONLY);

    if (fd1 == -1) {
        return;
    } else {
        fd2 = open(s[2], O_WRONLY | O_CREAT | O_EXCL, 0777);

        if(fd2 == -1){
            fd2 = open(s[2], O_WRONLY | O_CREAT | O_TRUNC, 0777);
        }

        while((nread = read(fd1, buffer, BUFSIZE)) > 0){
            write(fd2, buffer, nread);
        }
    }

    close(fd1);
    close(fd2);
}

void mkdir_m(char **s){
    mkdir(s[1], 0777);
}

void ls_m(char **s){
    DIR *dp;
    struct dirent *d;
    dp = opendir(".");
    while((d = readdir(dp)) != NULL){
        if(d->d_name[0] != '.'){
            printf("%s\n", d->d_name);
        }
    }
}

void vi_m(char **s){
    char buffer[BUFSIZE];
    int nread, fd = open(s[1], O_RDWR|O_CREAT, 0777);

    while((nread = read(fd, buffer, BUFSIZE)) > 0){
        write(1, buffer, nread);
    }
    while((nread = read(0, buffer, BUFSIZE)) > 0){
        if((strncmp(buffer, "quit", 4) == 0) && (buffer[4] == '\n'))
            break;
        write(fd, buffer, nread);
    }
}

int backup(const char *name, const struct stat *status, int type){
    char buffer[BUFSIZE], file1[BUFSIZE], file2[BUFSIZE] = "./TEMP/";
    int fd1, fd2, i, nread;

    if(type == FTW_F){
        fd1 = open(name, O_RDONLY);
        strcpy(file1, name + 2);

        for(i=0;i<strlen(file1);i++){
            if(file1[i] == '/')
                file1[i] = '_';
        }

        strcat(file2, file1);
        fd2 = open(file2, O_WRONLY | O_CREAT, status->st_mode&0777);

        while((nread = read(fd1, buffer, BUFSIZE)) > 0){
            write(fd2, buffer, nread);
        }
    }
    return 0;
}

void backup_m(char **s){
    sigset_t mask;

    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR2);

    while(1){
        sigprocmask(SIG_SETMASK, &mask, NULL);
        printf("*****BACK-UP STARTS*****\n");
        mkdir("TEMP", 0333);
        sleep(5);
        ftw(".", backup, 1);
        printf("*****BACK-UP ENDS*****\n");
        sigprocmask(SIG_UNBLOCK, &mask, NULL);

        alarm(10);
        pause();
    }
}

void exec_m(char **s){
    execv(s[0], s);
}

void catchusr(int signo){
    return ;
}

void catchusr2(int signo){
    exit(1);
}

void catchalarm(int signo){
    kill(getpid(), SIGUSR1);
}

int main(int argc, char **argv){
    char in[50], *res[20] = {0};
    char *inst[7] = {"cat", "cd", "cp", "mkdir", "ls", "vi", "backup"};
    void (*f[8])(char **) = {cat_m, cd_m, cp_m, mkdir_m, ls_m, vi_m, backup_m, exec_m};
    int i, status;
    pid_t pid, b_pid;

    static struct sigaction act;

    act.sa_handler = SIG_IGN;
    sigaction(SIGINT, &act, NULL);

    act.sa_handler = catchalarm;
    sigaction(SIGALRM, &act, NULL);

    act.sa_handler = catchusr;
    sigaction(SIGUSR1, &act, NULL);

    act.sa_handler = catchusr2;
    sigaction(SIGUSR2, &act, NULL);

    while(1){
        printf("> ");
        gets(in);
        i = 0;
        res[i] = strtok(in, " ");
        if(strcmp(res[0], "exit")==0){
            kill(b_pid, SIGUSR2);
            while(waitpid(b_pid, 0, WNOHANG) == 0){
                printf(".....wait for backup .....\n");
                sleep(1);
            }
            exit(0);
        }

        while(res[i]){
            i++;
            res[i] = strtok(NULL, " ");
        }

        for(i=0;i<7;i++){
            if(!strcmp(res[0], inst[i]))
                break;
        }

        if(i == 1){
            cd_m(res);
        }
        else{
            if (i == 6){
                b_pid = fork();
                if (b_pid == 0){
                    f[i](res);
                    exit(0);
                }
                else{
                    waitpid(b_pid, 0, WNOHANG);
                }
            }
            else{
                pid = fork();
                if (pid == 0){
                    act.sa_handler = SIG_DFL;
                    sigaction(SIGINT, &act, NULL);

                    f[i](res);
                    exit(0);
                }
                else{
                    waitpid(pid, &status, 0);
                }
            }
        }

        if(WIFSIGNALED(status))
            printf("\n");

    }
}

