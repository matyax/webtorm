#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>

#include "webserver_parser.h"

void webserver_taillogfile()
{
	int redirected[2];
	int nread;
	char tail[5120]; /* 10 lines of 512 max lenght by definition */
	char log_html[11147]; /* 6026 from file + 5120 of max lenght */
	char *ptr = log_html;
	FILE *fptr;

	int i;
	
	if (pipe(redirected) < 0) {
		perror("pipe()");
		return;
	}
	
	switch(fork()) {
		case -1:
			perror("fork()");
			return;
		case 0:
			close(redirected[0]);
			if (dup2(redirected[1], STDOUT_FILENO)<0) {
				perror("dup2()");
				return;
			}
			execlp("tail","tail", "public_html/webtorm.log", NULL);
			return;
		default:
			close(redirected[1]);
			if ((nread = read(redirected[0],tail,5119)) > 0)
			{
				tail[nread] = '\0';
				if ((fptr = fopen("public_html/log_template.html","r+")) == NULL)
				{
					perror("fopen()");
					return;
				}
				i = 1;
				while (fgets(ptr,(11147 - strlen(log_html)),fptr))
				{
					if (i == 108)
						sprintf(ptr,"%s%s",ptr,tail);
					ptr += strlen(ptr);
					i++;
				}
				
				fclose(fptr);
				
				if ((fptr = fopen("public_html/log.html","w")) == NULL)
				{
					perror("fopen()");
					return;
				}
				
				fwrite(log_html,1,strlen(log_html),fptr);
				
				fclose(fptr);
				
			}
			wait(NULL);
	}
}

char webserver_parse_instructions(char *path, int pipe, char started) {
	

	while ((*path != '\0') && (*path != '?'))
		path++;
	
	if (*path == '?')
	{
		*path = '\0';
		if (started == 0)
		{
			path++;
			while ((*path != '\0') && (*path != '='))
				path++;
			if (*path == '=')
			{
				path++;
				write(pipe,path,strlen(path)+1);
				return 1;
			}
		}
	}
	return 0;
	
}


char *webserver_parse(char *request)
{
	char *path; 
	char tmp[1020];
	
	if (sscanf(request, "GET %s\r\n", tmp) <= 0)
		return NULL;
		
	path = malloc(strlen(tmp) + 1);
	strncpy(path,tmp,strlen(tmp) + 1);
	return path;
}

int webserver_open(char **path)
{
	char fullpath[strlen(RAIZ) + strlen(*path)];
	int fd = 0;
	
	sprintf(fullpath,"%s%s",RAIZ,*path);
	
	if ((fd = open(fullpath,O_RDONLY,0)) <= 0)
	{
		fd = open("404.html",O_RDONLY,0);
		*path = "404.html";
	}
	
	return fd;
}

mime_t *webserver_load_mime_types()
{
	FILE *fptr;
	char buffer[150];
	char mimetype[75];
	char *ptr;
	int i;
	mime_t *inicio;
	mime_t *list = malloc(sizeof(mime_t));
	
	list->next = NULL;
	list->type = NULL;
	
	inicio = list;
	puts("cargando mime types");
	if ((fptr = fopen("/etc/mime.types","r")) != NULL)
	{
		while (fgets(buffer,150,fptr) != NULL)
		{
			if ((buffer[0] == '#') || (buffer[0] == '\n') || (buffer[0] == '\0'))
				continue;
			
			if (sscanf(buffer,"%s\t",mimetype) > 0)
			{
				ptr = buffer;
				ptr += strlen(mimetype);
				
				while ((*ptr == '\t') && (*ptr != '\0'))
					ptr++;
				
				if (strlen(ptr) > 1)
				{
					if (*(ptr+strlen(ptr) -1) == '\n')
						*(ptr+strlen(ptr) -1) = '\0';
						
					list->type = malloc(strlen(mimetype) + 1);
					strncpy(list->type, mimetype, strlen(mimetype) + 1);
					
					/* first call */
					i = 0;
					ptr = strtok( ptr, " " );
					list->ext[i] = malloc(strlen(ptr) + 1);
					strncpy(list->ext[i],ptr,strlen(ptr) + 1);
					i++;
										
					while (((ptr = strtok( NULL, " " )) != NULL ) && (i <= 6))
					{
						list->ext[i] = malloc(strlen(ptr) + 1);
						strncpy(list->ext[i],ptr,strlen(ptr) + 1);
						i++;
					}
					if (i <= 6)
						list->ext[i] = NULL;
						
					list->next = malloc(sizeof(mime_t));
					list = list->next;
					list->next = NULL;
				}
			}
		}
	}
	
	return inicio;
}

char *webserver_mimetype(char *path, mime_t *mimes)
{
	char *ptr = path + strlen(path);
	int i;
	
	while ((path != ptr) && (*ptr != '.'))
		ptr--;
	if (*ptr == '.')
	{
		ptr++;
		while (mimes->next != NULL)
		{
			for (i = 0; i <= 6 && mimes->ext[i] != NULL; i++)
			{
				if (strcmp(mimes->ext[i],ptr) == 0)
					return mimes->type;
				
			}
			mimes = mimes->next;
		}
		
	}
	
	return "unknown";
}
