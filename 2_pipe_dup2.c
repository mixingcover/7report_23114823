#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "사용법: %s [명령어] [인자들...]\n", argv[0]);
        return 1;
    }

    int fd[2];   /* fd[0] = 읽기 끝, fd[1] = 쓰기 끝 */
    if (pipe(fd) == -1) { perror("pipe"); return 1; }

    /* 자식 프로세스: 명령어를 실행하고 출력을 파이프로 보낸다 */
    if (fork() == 0) {   
        close(fd[0]);                 /* 읽기 끝은 쓰지 않는다 */
        dup2(fd[1], STDOUT_FILENO);   /* 표준출력을 파이프 쓰기 끝으로 교체 */
        close(fd[1]);                 /* 원본 디스크립터는 이제 필요 없다 */
        
        execvp(argv[1], &argv[1]);    /* 전달받은 명령어와 인자들 실행 */
        perror("execvp");             /* exec가 실패했을 때만 도달 */
        _exit(127);                 
    }

    /* 부모 프로세스: 자식이 파이프에 쓴 출력을 읽어서 화면에 출력한다 */
    close(fd[1]);                     /* 쓰기 끝은 쓰지 않는다 */

    char buf[256];
    ssize_t n;
    while ((n = read(fd[0], buf, sizeof(buf) - 1)) > 0) {
        buf[n] = '\0';
        printf("%s", buf);
    }
    close(fd[0]);

    wait(NULL);
    wait(NULL);   /* 자식 프로세스가 종료될 때까지 대기하여 좀비 프로세스 방지 */
    return 0;
}