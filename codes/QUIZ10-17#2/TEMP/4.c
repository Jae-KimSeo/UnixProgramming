#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <ftw.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

void cat_m(char ** data){ /* 16010977 */

    char * filename = data[1];
    char buff[512];
    int fd = open(filename, O_RDONLY);
    int size;

    while(size = read(fd, buff, sizeof(buff))){

        write(0, buff, size);

    }
    close(fd);
}

void cd_m(char ** data){ /* 16010977 */
    chdir(data[1]);
}
void cp_m(char ** data){
    char * file_in = data[1];
    char * file_out = data[2];
    char buff[512];
    int in, out;
    int size;

    in = open(file_in, O_RDONLY);
    out = open(file_out, O_CREAT | O_WRONLY, 0644);

    while(size = read(in, buff, sizeof(buff))){
        write(out, buff, size);
    }
    close(in);
    close(out);

}
void mkdir_m(char ** data){ /* 16010977 */
    int mode = 0755;
    mkdir(data[1], mode);
}
void ls_m(char ** data){ /* 16010977 */
    DIR * dir;
    struct dirent *ent;

    dir = opendir(".");

    while(ent = readdir(dir)){
        printf("%s\n", ent->d_name);
    }

    closedir(dir);
}

void vi_m(char ** data){ /* 16010977  */
    int fd;
    char buff[512];
    int size;
    int flag = 0;


    fd = open(data[1], O_WRONLY | O_APPEND);

    if (fd != -1){
        cat_m(data);

    }
    else{
        fd = open(data[1], O_CREAT | O_WRONLY, 0644);
    }

    while(size = read(0, buff, sizeof(buff))){
        if (strncmp(buff+size-5, "quit", 4) == 0){
            size -= 5;
            flag = 1;
        }
        write(fd, buff, size);
        if (flag == 1) break;
    }

    close(fd);


}


int backup_file(const char * name, const struct stat * st, int type){

    char buff[512];
    char output[512] ="./TEMP/";
    int in, out;
    int i, size;

    if (type != FTW_F){
        return 0;
    }

    if (strncmp(output, name, 7) == 0) return 0;

    strcpy(buff, name);
    strcat(output, name+2);

    for(i = 7; output[i]; i++){
        if (output[i] == '/')
            output[i] = '_';
    }

    in = open(buff, O_RDONLY);
    out = open(output, O_CREAT | O_WRONLY | O_TRUNC, st->st_mode);

    while(size = read(in, buff, sizeof(buff))){
        write(out, buff, size);
    }

    close(in);
    close(out);

    return 0;
}

void backup(char **s){
    mkdir("TEMP", 0777);
    ftw(".", backup_file, 1);
}

void exec_m(char **s){   /* 16011025 */
    execv(s[0],s);
}


int main(void){
    char in[50], *res[20] = {0};
    char *inst[7]={"cat","cd","cp", "mkdir","ls","vi", "backup"};
    void (*f[8])(char **)={cat_m, cd_m, cp_m, mkdir_m, ls_m, vi_m, backup, exec_m };
    int i;
    pid_t pid, b_pid;
    int background = 0;

    while(1){
        printf("> ");
        gets(in);
        i = 0;
        res[i] = strtok(in, " ");
        if (strcmp(res[0], "exit") ==0)
            exit(0);

        while(res[i]){
            i++;
            res[i] = strtok(NULL, " ");
        }

        for(i = 0; i < 7; i++){
            if (!strcmp(res[0], inst[i]))
                break;
        }
        if (i == 1){
            cd_m(res);
        }
        else{
            if (i == 6){
                background++;
                b_pid = fork();
                if (b_pid == 0){
                    f[i](res);
                    exit(0);

                }
            }
            else{
                pid = fork();
                if (pid == 0){
                    f[i](res);
                    exit(0);
                }
                else{
                    waitpid(pid, 0, 0);
                }
            }
        }
        if (background){
            if(waitpid(b_pid, 0, WNOHANG)){
                background = 0;
            }
        }
    }
}

