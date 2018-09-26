#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "webtorm_parser.h"

response_t *webtorm_parse_struct(response_t *response)
{
	char *tmp = response->html;
	char *to_free = response->html;
	char buff[256]; /* I love 2 powers */
	int code = 0; /* response code */
	float http_ver;
	
	if (tmp == NULL)
		return response;
	
	while (*tmp != '\0')
	{
		if ((*tmp == '\r') && (*(tmp-1) == '\n') && (*(tmp+1) == '\n'))
		{
			tmp += 2;
			break;
		}
		if (code == 0)
		{
			if (sscanf(tmp,"HTTP/1.1 %d ",&code) > 0)
				http_ver = 1.1;
			if (sscanf(tmp,"HTTP/1.0 %d ",&code) > 0)
				http_ver = 1.0;
			if (sscanf(tmp,"HTTP/0.9 %d ",&code) > 0)
				http_ver = 0.9;
			
			/* jump to a new line */
			while ((*tmp != '\0') && (*tmp != '\n') && (*tmp != 'H'))
				tmp++;
			continue;
		}
		else 
		{ 
			if (http_ver != 0.9f)
			{
				if ((response->header[0] == NULL) && (sscanf(tmp,"Content-Type: %s",buff) > 0))
				{
					response->header[0] = malloc(strlen(buff) + 1);
					strncpy(response->header[0],buff,strlen(buff) + 1);
					
					/* jump to a new line */
					while ((*tmp != '\0') && (*tmp != '\n'))
						tmp++;
				}

				if ((code == 301) || (code == 302))
				{
					if (sscanf(tmp,"Location: %511s",buff) > 0)
					{
						response->header[1] = malloc(strlen(buff) + 1);
						strncpy(response->header[1],buff,strlen(buff) + 1);
						
						/* jump to a new line */
						while ((*tmp != '\0') && (*tmp != '\n'))
							tmp++;
					}
				}
			}
		}
		tmp++;
	}
	if (code)
	{
		if (strlen(tmp) > 1)
		{
			response->html = malloc(strlen(tmp) + 1);
			strncpy(response->html,tmp,strlen(tmp) + 1);
		}
		else
			response->html = NULL;
		
		response->code = code;
		
	}

	free(to_free);

	return response;
	
}

char *webtorm_tolower(char *string)
{
	char *tmp = string;
	
	for (; *string != '\0'; string++)
		*string = tolower(string[0]);

	return tmp;
}

int webtorm_isurl(char *string)
{
	if (*string == '/')
		return 0;
	
	if (strncmp(string,"http://",7) == 0)
		return 1;
		
	if (strncmp(string,"www.",4) == 0)
		return 1;

	return 0;
}
int webtorm_validate_file(char *href)
{
	char *ptr;

	if ((strncmp(href,"https://",8) == 0) || (strncmp(href,"ftp",3) == 0) || (strncmp(href,"mailto:",7) == 0))
		return 0;
	
	if ((strlen(href) == 1) && (strncmp(href,"/",1) == 0))
		return 1;
		
	if (href[strlen(href) -1] == '/')
		return 1;
	
	if (href[strlen(href) -1] == '\\')
		return 0;
	
	ptr = href + strlen(href);
	
	while ((ptr >= href) && (*ptr != '.'))
		ptr--;
	
	if (*ptr == '.')
	{
		ptr++;
		if ((strncmp(ptr,"html",4) == 0) || (strncmp(ptr,"php",3) == 0) || (strncmp(ptr,"htm",3) == 0) || (strncmp(ptr,"asp",3) == 0))
			return 1;
	}
	return 0;
	
}

href_t *webtorm_gethrefs(char *html, char *domain)
{
	char *tmp;
	char buff[255];
	char *aux;
	int i;
	href_t *href;
	href_t *inicio = NULL;

	if (html == NULL)
		return NULL;
	if (strlen(html) <= 1)
		return NULL;
	
	//printf("%s\n%s\n\n",html,domain);
	/* to lower! */
	tmp = webtorm_tolower(html);

	while (*tmp != '\0')
	{
		if (strncmp(tmp,"href",4) == 0)
		{
			tmp += 4;
			i = 0;

			while ((*tmp != '\0') && (*tmp != '\n') && (*tmp != '"'))
				tmp++;
			
			if (*tmp == '"')
			{
				tmp++;
				while ((*tmp != '\0') && (*tmp != '\n') && (*tmp != '"') && (i <= 255))
				{
					if ((*tmp == '\'') || (*tmp == '\\'))
					{
						i = 0;
						break;
					}
					buff[i] = *tmp;
					i++;
					tmp++;
				}
			}
			if (*tmp == '\0')
				break;
			if ((*tmp == '\n') || (i > 255))
				continue;
			
			buff[i] = '\0';
			
			if (strlen(buff) >= 1)
			{
				/* reject unwanted protocols */
				if ((strncmp(buff,"https://",8) == 0) || (strncmp(buff,"ftp",3) == 0) || (strncmp(buff,"mailto:",7) == 0))
					continue;
				
				if (inicio == NULL)
				{
					href = malloc(sizeof(href_t));
					inicio = href;
				}
				else
				{
					href->next = malloc(sizeof(href_t));
					href = href->next;
				}
				
				if (webtorm_isurl(buff) == 0)
				{
					
					if ((domain[strlen(domain) - 1] == '/') && (buff[0] == '/'))
					{
						aux = buff;
						aux++;
					}
					else
						aux = buff;
					
					if ((domain[strlen(domain) - 1] != '/') && (aux[0] != '/'))
					{
						href->url = malloc(strlen(domain) + strlen(aux) + 2);
						sprintf(href->url,"%s/%s",domain,aux);
						href->url[strlen(domain) + strlen(aux) + 1] = '\0';
					}
					else
					{
						href->url = malloc(strlen(domain) + strlen(aux) + 1);
						sprintf(href->url,"%s%s",domain,aux);
						href->url[strlen(domain) + strlen(aux)] = '\0';
					}
				}
				else
				{
					href->url = malloc(strlen(buff) + 1);
					strncpy(href->url,buff,strlen(buff) + 1);
				}
				href->next = NULL;
			}
		}
		tmp++;
	}
	return inicio;
}
