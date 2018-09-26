#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>

#include "webtorm_log.h"
#include "webtorm_connection.h"
#include "webtorm_log.h"

int webtorm_connect(char *domain)
{
	int sockfd;
	struct sockaddr_in addr_remoto;
	struct hostent *ip;
	char log[512];

	/* ip resolve */
	if ((ip = gethostbyname(domain)) == NULL)
	{
		snprintf(log,511,"Can't resolve: %s<br>",domain);
		webtorm_log(log);
		return 0;
	}
	
	memset(&addr_remoto, 0, sizeof(addr_remoto));

	/* AF_INET */
	addr_remoto.sin_family = AF_INET;

	/* get ip in network byte order format*/	
	memcpy((char *) &addr_remoto.sin_addr, *ip->h_addr_list, sizeof(addr_remoto.sin_addr));

	/* host from cpu to net */
	addr_remoto.sin_port = htons(80);
	
	/* new socket */	
	if ((sockfd = socket(PF_INET, SOCK_STREAM, 0))<0) {
		perror("couldn't stablish a socket");
		return 0;
	}

	/* connect! */
	if (connect(sockfd, (struct sockaddr *)&addr_remoto, sizeof(addr_remoto))) {
		perror("couldn't connect to remote host");
		return 0;
	}

	return sockfd;
}

char *webtorm_get(int sockfd, char *host, char *path)
{
	char *request = malloc(294 + strlen(host) + strlen(path)); /* 294 basic request + 255 max for path + 128 host max size */
	char *readbuffer = malloc(BUFF_SIZE); /* 1mb for reading... too much? */
	char *tmp = readbuffer; /* pointer to buffer beginning */
	int n;
	int total = BUFF_SIZE;
	
	char log[31];

	n = sprintf(request,"GET %s HTTP/1.1\r\nHost: %s\r\nUser-Agent: WebTorm\r\nAccept: text/xml,application/xml,application/xhtml+xml,text/html;q=0.9,text/plain;q=0.8\r\nAccept-Language: es-ar,es;q=0.8,en-us;q=0.5,en;q=0.3\r\nAccept-Charset: ISO-8859-1,utf-8;q=0.7,*;q=0.7\r\nKeep-Alive: 300\r\nConnection: close\r\n\r\n",path,host);

	/* request */
	write(sockfd,request,n);

	/* response */	
	while (((n = read(sockfd,readbuffer,4096)) > 0) && (total >= 0))
	{
		total -= n;
		readbuffer +=n;
	}
		
	snprintf(log,30,"Total downloaded: %d kb<br>",((BUFF_SIZE - total) / 1024));
	webtorm_log(log);
	
	if (BUFF_SIZE == total)
	{
		free(request);
		return NULL;
	}
	*(tmp+BUFF_SIZE-total) = '\0';

	free(request);
	
	return tmp;
}
