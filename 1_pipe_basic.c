#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "사용법: %s \"문자열\"\n", argv[0]);
        return 1;
    }

    int pipe1[2];  
    int pipe2[2];   

    if (pipe(pipe1) == -1) { perror("pipe1"); return 1; }
    if (pipe(pipe2) == -1) { perror("pipe2"); { close(pipe1[0]); close(pipe1[1]); } return 1; }

    pid_t pid = fork();
    if (pid == -1) { perror("fork"); return 1; }

    if (pid == 0) {
        close(pipe1[1]);
        close(pipe2[0]);

        char buf[256];
        ssize_t n = read(pipe1[0], buf, sizeof(buf) - 1);
        if (n > 0) {
            buf[n] = '\0';
            printf("[자식] 받음: \"%s\"\n", buf);

            // 소문자를 대문자로 변환
            for (int i = 0; buf[i] != '\0'; i++) {
                buf[i] = toupper((unsigned char)buf[i]);
            }

            // 변환된 문자열을 부모에게 전송
            write(pipe2[1], buf, strlen(buf));
            printf("[자식] 보냄: \"%s\"\n", buf);
        }

        close(pipe1[0]);
        close(pipe2[1]);  /* 쓰기 끝을 닫아 부모의 read()가 EOF를 보게 함 */
        
        printf("[자식] 종료\n");
        fflush(stdout);
        _exit(0);
    }

    close(pipe1[0]);
    close(pipe2[1]);

    
    const char *msg = argv[1];
    write(pipe1[1], msg, strlen(msg));
    printf("[부모] 보냄: \"%s\"\n", msg);
    close(pipe1[1]);  
    char buf[256];
    ssize_t n = read(pipe2[0], buf, sizeof(buf) - 1);
    if (n > 0) {
        buf[n] = '\0';
        printf("[부모] 받음: \"%s\"\n", buf);
    }
    close(pipe2[0]);

    wait(NULL);  /* 자식 프로세스 회수 */
    printf("[부모] 자식을 거두었습니다\n");

    return 0;
}