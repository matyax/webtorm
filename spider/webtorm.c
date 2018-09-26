#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include </usr/include/mysql/mysql.h>

#include "webtorm_parser.h"
#include "webtorm_browser.h"
#include "webtorm_log.h"
#include "webtorm_db.h"


href_t *webtorm_geturls(char *instructions, int nread)
{
	char *ptr;
	href_t *urls;
	href_t *inicio;
	
	if (nread == 0)
		return NULL;
	instructions[nread -1] = '\0';
	
	if ((ptr = strtok(instructions, "+" )) != NULL)
	{
		urls = malloc(sizeof(href_t));
		
		urls->url = malloc(strlen(ptr) + 1);
		strncpy(urls->url,ptr,strlen(ptr) + 1);
		urls->next = NULL;
		
		inicio = urls;
	}
	while ((ptr = strtok(NULL, "+" )) != NULL)
	{
		urls->next = malloc(sizeof(href_t));
		urls = urls->next;
		
		urls->url = malloc(strlen(ptr) + 1);
		strncpy(urls->url,ptr,strlen(ptr) + 1);
		urls->next = NULL;
	}
	return inicio;
}

void webtorm(int pipe, int nthreads)
{
	webtorm_t *request = malloc(nthreads * sizeof(webtorm_t));
	pthread_t hilos[nthreads];
	href_t *hrefs;
	href_t *urls;
	href_t *tmp;
	char instructions[1024];
	int nread;
	int i,ii;
		
	while (1)
	{
		/* wait for instructions */
		if ((nread = read(pipe,instructions,1024)) > 0)	
			urls = webtorm_geturls(instructions,nread);			

		while (urls != NULL)
		{
			for (i = 0; i < nthreads && urls != NULL; i++)
			{
				request[i].n = strlen(urls->url);
				request[i].url = malloc(request[i].n + 1);
				strncpy(request[i].url,urls->url,request[i].n);

				if (pthread_create(&hilos[i], NULL, webtorm_browser, (void*)&request[i]))
					perror("pthread_create()");

				tmp = urls->next;
				free(urls->url);
				free(urls);
				urls = tmp;
				
			}

			for (ii=0; ii < i; ii++)
			{

				while (pthread_join(hilos[ii], (void **)&hrefs));

				if (hrefs->url != NULL)
				{
					if (urls != NULL)
					{

						tmp = urls;
						while (tmp->next != NULL)
							tmp = tmp->next;
						tmp->next = malloc(sizeof(href_t));
						memcpy(tmp->next,hrefs,sizeof(href_t));
					}
					else
					{

						urls = malloc(sizeof(href_t));
						memcpy(urls,hrefs,sizeof(href_t));
						free(hrefs);
					}
				}
			}
		}
		
	}
}
