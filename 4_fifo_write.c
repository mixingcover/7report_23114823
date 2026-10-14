/*
 * 4_fifo_write.c — FIFO 로 메시지를 보낸다  [나중에 실행할 쪽]
 *
 * [핵심 개념]
 *   받는 쪽(3_fifo_read)이 만들어 둔 FIFO 를 열어서 쓰기만 하면 된다.
 *   서로 부모-자식 관계가 아니어도 통신이 된다는 점이 파이프와 다르다.
 *   출처: man 7 fifo
 *
 * [컴파일·실행]
 *   gcc -Wall -Wextra -o 4_fifo_write 4_fifo_write.c
 *   ./4_fifo_write 안녕하세요
 */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <errno.h>
#define FIFO_PATH "/tmp/sysprog_fifo"

int main(int argc, char *argv[])
{
    const char *msg = (argc == 2) ? argv[1] : "기본 메시지";   /* 인자가 있으면 그걸, 없으면 기본값 */

    /* 받는 쪽이 먼저 실행되어 있어야 한다. 없으면 open 이 기다린다. */
    int fd1 = open(FIFO_PATH, O_WRONLY);
    if (fd1 == -1) {
        perror("open (받는 쪽을 먼저 실행했는지 확인)");
        return 1;
    }

    if (write(fd1, msg, strlen(msg)) == -1) {   /* 널 종료 문자는 보내지 않는다 */
        perror("write");
        close(fd1);
        return 1;
    }

    printf("보냈습니다: %s\n", msg);
    close(fd1);

    if (mkfifo(FIFO_PATH, 0666) == -1 && errno != EEXIST) {
        perror("mkfifo");
        return 1;
    }

    printf("보내는 쪽을 기다립니다... (%s)\n", FIFO_PATH);

    int fd = open(FIFO_PATH, O_RDONLY);   /* 상대가 열 때까지 여기서 기다린다 */
    if (fd == -1) { perror("open"); return 1; }

    char buf[128];
    /* 마지막 한 자리는 남겨 두어 뒤에 '\0'을 붙일 자리로 쓴다 */
    ssize_t n = read(fd, buf, sizeof buf - 1);
    if (n > 0) {
        buf[n] = '\0';
        printf("받은 메시지: %s\n", buf);
    }

    close(fd);
    unlink(FIFO_PATH);   
    return 0;
}
