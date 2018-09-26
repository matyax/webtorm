#define BUFF_SIZE 1048576

int webtorm_connect(char *domain);
char *webtorm_get(int sockfd, char *host, char *path);
