#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>

#define FIFO_PATH "/tmp/sysprog_fifo"

int main(void)
{
    /* FIFO 를 만든다. 이미 있으면(EEXIST) 그대로 쓴다. */
    if (mkfifo(FIFO_PATH, 0666) == -1 && errno != EEXIST) {
        perror("mkfifo");
        return 1;
    }

    printf("보내는 쪽을 기다립니다... (%s)\n", FIFO_PATH);

    int fd1 = open(FIFO_PATH, O_RDONLY);   /* 상대가 열 때까지 여기서 기다린다 */
    if (fd1 == -1) { perror("open"); return 1; }

    char buf[128];
    /* 마지막 한 자리는 남겨 두어 뒤에 '\0'을 붙일 자리로 쓴다 */
    ssize_t n = read(fd1, buf, sizeof buf - 1);
    if (n > 0) {
        buf[n] = '\0';
        printf("받은 메시지: %s\n", buf);
    }

    close(fd1);

    /* --- [추가] 키보드로 보낼 메시지 입력받기 --- */
    char msg[128];
    printf("보낼 메시지를 입력하세요: ");
    if (fgets(msg, sizeof(msg), stdin) != NULL) {
        msg[strcspn(msg, "\n")] = '\0'; /* 입력받은 개행 문자('\n') 제거 */
    }

    /* 받는 쪽이 먼저 실행되어 있어야 한다. 없으면 open 이 기다린다. */
    int fd = open(FIFO_PATH, O_WRONLY);
    if (fd == -1) {
        perror("open (받는 쪽을 먼저 실행했는지 확인)");
        unlink(FIFO_PATH);
        return 1;
    }

    if (write(fd, msg, strlen(msg)) == -1) {   /* 널 종료 문자는 보내지 않는다 */
        perror("write");
        close(fd);
        unlink(FIFO_PATH);
        return 1;
    }

    printf("보냈습니다: %s\n", msg);
    close(fd);
    
    unlink(FIFO_PATH);   /* 다 썼으면 FIFO 파일도 지운다 — 안 지우면 파일시스템에 계속 남는다 */
    return 0;
}