#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/wait.h>
#include <pthread.h>

#include "webserver_connection.h"
#include "webserver_parser.h"

int webserver_listen(unsigned short port)
{
	int sockfd;
	struct sockaddr_in addr_in;
	int opt;
	
	if (port <= 0)
	{
		perror("invalid port");
		return -1;
	}

	/* new socket */
	if((sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		perror("couldn't stablish a socket");
        	return -1;
	}
	
	/* socket struct setting up */
	memset(&addr_in, 0, sizeof(struct sockaddr_in));
	addr_in.sin_family = AF_INET;
	addr_in.sin_addr.s_addr = ntohl(INADDR_ANY);
	addr_in.sin_port = htons(port);

	/* socket setting up */
	opt = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *) &opt, sizeof(opt)) < 0) {
		perror("setsockopt()");
		return -1;
	}

	/* assign address to the socket */
	if (bind (sockfd, (struct sockaddr *)&addr_in, sizeof(struct sockaddr_in)) < 0) {
		perror("error when binding");
        	return -1;
	}
	/* listen upt to 20 connections */
	if ((listen(sockfd, 20)) < 0) {
		perror("error when listening");
	        return -1;
	}

	return sockfd;
}

/* 
 * Atendemos conexiones entrantes
 */
void *webserver_answer(void *arg) {
     
	char request[1024];
	char *path, *mime_type;
	int *thread_arg = (int *)arg;
	int nread, requestfd;
    	static mime_t *mimes = NULL;
    	char *to_free;
    	
    	static char started = 0; /* flag */
    	
    	if (mimes == NULL)
    	{
    		if ((mimes = webserver_load_mime_types()) == NULL)
    		{
			perror("couldn't read mime types");
			return NULL;
		}
	}
	if ((nread = read(thread_arg[0], request, 1024)) > 0)
	{
		/* in some cases we'll assign path to const char values or
		   pass it by "reference", so we'll save its value here to
		   avoid "freeing" errors */
		path = webserver_parse(request);
		to_free = path;
		
		if ((strlen(path) == 1) && (strncmp(path,"/",1) == 0))
			path = "/index.html";
		
		if (strstr(path,"../") != NULL)
		{
			free(to_free);
			close(thread_arg[0]);
			return NULL;
		}
	
		/* special request */
		if ((strstr(path,"config.html") != NULL) && (strstr(path,"?") != NULL))
			started = webserver_parse_instructions(path,thread_arg[1],started);

		if (strstr(path,"stop.html") != NULL)
		{
			kill(thread_arg[2], SIGTERM);
			wait(NULL);
		}
		
		if (strstr(path,"log.html") != NULL)
			webserver_taillogfile();
		/* end of special request */
		
		requestfd = webserver_open(&path);
	
		mime_type = webserver_mimetype(path, mimes);

		webserver_response(thread_arg[0], mime_type, requestfd);

		free(to_free);
	}
	
	close(thread_arg[0]);

	pthread_exit(NULL);
}

void webserver_response(int conexfd, char *mime_type, int requestfd) 
{
	char buf_out[4096];
	int n;
	n = snprintf(buf_out, sizeof(buf_out), "HTTP/1.0 200 OK\r\nContent-type: %s\r\n\r\n",mime_type);
	write(conexfd, buf_out, n);
	
	while ((n = read(requestfd,buf_out,4096)) > 0) 
	 	write(conexfd,buf_out,n);
	 	
	close(requestfd);
	
	return;
}
