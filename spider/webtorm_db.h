#define SERVER "127.0.0.1"
#define USER "root"
#define PASSWORD ""
#define DATABASE "webtorm"

int webtorm_db_visited(char *domain, char *path);
int webtorm_db_isvisited(char *domain, char *path);
int webtorm_db_store(char *domain, char *path, char *html);
