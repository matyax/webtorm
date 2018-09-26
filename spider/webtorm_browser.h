typedef struct
{
	char *url;
	int n;
} webtorm_t;

webtorm_t *webtorm_strip(webtorm_t *request);

char *webtorm_getpath(webtorm_t *request, int skip);

char *webtorm_getdomain(webtorm_t *request);

void *webtorm_browser(void *arg);
