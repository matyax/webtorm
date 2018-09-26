#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <sys/wait.h>

#include "spider/webtorm_log.h"
#include "webserver_connection.h"
#include "webserver_parser.h"
#include "spider/webtorm.h"
#include "spider/webtorm_db.h"

int main(int argc, const char **argv) {

	int sockfd, webspider;
	pthread_t t_id;
	int fatherchild[2];
	int *thread_arg = malloc(3 * sizeof(int));
	/*
	thread_arg[0] = conex fd;
	thread_arg[1] = stdout to child pipe
	thread_arg[2] = webspider pid
	*/
	
	 if (argc != 2) {
		perror("uso: %s <port>\n");
		return 1;
	}
	
	/* new socket */
	if ((sockfd = webserver_listen(atoi(argv[1]))) < 0)      	
		return 1;
	
	if (pipe(fatherchild) < 0) {
		perror("pipe");
		return 1;
	}
	switch (webspider = fork())
	{
		case 0:
			close(fatherchild[1]);
			
			/* initialize logs file and lastvisited DB table */
			webtorm_log(NULL);
			webtorm_db_isvisited(NULL,NULL);
			
			webtorm(fatherchild[0],2);;
		break;
		default:
			close(fatherchild[0]);
			thread_arg[1] = fatherchild[1];
			thread_arg[2] = webspider;
		break;
	}
	
	while ((thread_arg[0] = accept(sockfd, NULL, 0)) > 0)
		pthread_create(&t_id, NULL, webserver_answer, (void*)thread_arg);		
	
	kill(webspider, SIGTERM);
	wait(NULL);
	
	return 0;
}
