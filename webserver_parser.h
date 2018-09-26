#define RAIZ "public_html"

typedef struct mime_t
{
	char *type;
	char *ext[6]; /* keeping it simple... space for up to 6 file extension */
	struct mime_t *next;
} mime_t;

void webserver_taillogfile();
char webserver_parse_instructions(char *path, int pipe, char started);
char *webserver_parse(char *request);
int webserver_open(char **path);
mime_t *webserver_load_mime_types();
char *webserver_mimetype(char *path, mime_t *mimes);
