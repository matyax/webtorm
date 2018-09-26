#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

#include "webtorm_parser.h"
#include "webtorm_browser.h"
#include "webtorm_connection.h"
#include "webtorm_log.h"
#include "webtorm_db.h"

webtorm_t *webtorm_strip(webtorm_t *request)
{
	char *tmp;
	char *old_url = request->url; /*apuntamos para liberar cuando no se necesite más*/

	if (strncmp(request->url,"http://",7) == 0)
	{
		request->url += 7;
		request->n -= 7;
	}	
	/* chequeamos malos request */
	if (request->n == 0)
	{
		free(old_url);
		return NULL;
	}

	tmp = malloc(request->n + 1);
	
	strncpy(tmp,request->url,request->n + 1);
		
	/*liberamos*/
	free(old_url);

	request->url = tmp;
	
	return request;
}

char *webtorm_getpath(webtorm_t *request, int skip)
{		
	char *path;
	char *tmp = request->url;

	if (request->n == skip)
		return "/";	
	
	tmp += skip;

	path = malloc(request->n - skip + 1);
	
	strncpy(path,tmp,request->n - skip);
	
	*(path+request->n - skip) = '\0';

	return path;
}

char *webtorm_getdomain(webtorm_t *request)
{
	char *domain;
	int i;
	
	for (i = 0; i <= request->n; i++)
	{
		if (*(request->url+i) == '/')
		{
			i++;	
			break;
		}
	}
	domain = malloc(i);

	strncpy(domain,request->url,i-1);

	*(domain+i-1) = '\0';

	return domain;
	
}

void *webtorm_browser(void *arg)
{	
	webtorm_t *request = arg;
	
	href_t *hrefs = NULL;
	
	/* to avoid NULL returns */

	int sockfd;
		
	char *domain; /* dominio a search */
	char *path; /* web to request */
	
	char log[512];
	
	response_t *response = malloc(sizeof(response_t)); /* readed web */
	
	/* integrity initializations */
	
	response->code = 0;
	response->header = malloc(2 * sizeof(char *));
	
	response->header[0] = NULL; /* mime type */
	response->header[1] = NULL; /* location */
	
	response->html = NULL;
	//puts(request->url);
	/* clean http:// */
	if ((request = webtorm_strip(request)) == NULL)
	{
		webtorm_log("Invalid request: empty \"http://\"<br>");
		return NULL;
	}

	domain = webtorm_getdomain(request);

	path = webtorm_getpath(request, strlen(domain));

	/* seguridad */	
	if (strlen(path) >= 255)
	{
		snprintf(log,511,"Request too long: %s<br>",path);
		webtorm_log(log);
		return NULL;
	}
	if (strlen(domain) >= 128)
	{
		snprintf(log,511,"Domain too long: %s<br>",domain);
		webtorm_log(log);
		return NULL;
	}
	
	/* validate html, php or asp */
	if (webtorm_validate_file(path) == 0)
	{	
		snprintf(log,511,"Rejected %s<br>",request->url);
		webtorm_log(log);
	}

	if (webtorm_db_isvisited(domain,path) == 0)
	{
		snprintf(log,511,"Accepted %s<br>Domain: %s<br>Web to request: %s",request->url,domain,path);
		webtorm_log(log);
		/* connect and request */

		if ((sockfd = webtorm_connect(domain)) > 0)
		{

			webtorm_db_visited(domain,path);

			response->html = webtorm_get(sockfd, domain, path);

			/* parse the response */
			response = webtorm_parse_struct(response);

			switch (response->code)
			{
				/* OK */
				case 200:

					hrefs = webtorm_gethrefs(response->html,domain);
					webtorm_db_store(domain,path,response->html);
				break;
				/* moved temporarily/permanently*/
				case 301:
				case 302:
					hrefs = malloc(sizeof(href_t));
					hrefs->url = malloc(strlen(response->header[1]) + 1); /* header Location */
					strncpy(hrefs->url,response->header[1],strlen(response->header[1]));
					hrefs->next = NULL;
				break;
			}
			
		}
	}
	else
	{	
		snprintf(log,511,"Already visited %s<br>",request->url);
		webtorm_log(log);
	}
	
	if ((strncmp(path,"/",1) == 0) && (strlen(path) > 1))
		free(path);
	//puts("1");
	free(domain);
	//puts("2");
	free(request->url);
	//puts("3");
	//free(response->html);
	//puts("4");
	free(response->header[0]);
	//puts("5");
	free(response->header[1]);
	//puts("6");
	free(response->header);
	//puts("6");
	free(response);
	//puts("7");
	if (hrefs == NULL)
	{
		/* simulate null return */
		hrefs = malloc(sizeof(href_t));
		hrefs->url = NULL;
		hrefs->next = NULL;
	}
	//puts("8");
	pthread_exit(hrefs);
}
