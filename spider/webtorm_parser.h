typedef struct
{
	short int code;
	char **header;
	/*
		to keep it simple, we'll use the next list.
		header[0] content type
		header[1] location
	*/
	char *html;
} response_t;

typedef struct href_t
{
	char *url;
	struct href_t *next;
} href_t;

response_t *webtorm_parse_struct(response_t *response);

char *webtorm_tolower(char *string);

int webtorm_isurl(char *string);

int webtorm_validate_file(char *href);

href_t *webtorm_gethrefs(char *html, char *domain);
