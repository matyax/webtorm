#include </usr/include/mysql/mysql.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "webtorm_db.h"

char *webtorm_db_strip(char *html)
{
	char *tmp = malloc(strlen(html) * 2); /* double of html size if we find an '''' or """" full fille :P */
	char *tmp_html = html;
	char *ptr = tmp;
	
	while (*html != '\0')
	{
		if (*html == '\'')
		{
			*tmp = '\\';
			*(tmp+1) = '\'';
			tmp += 2;
			html++;
			continue;
		}
		if (*html == '\"')
		{
			*tmp = '\\';
			*(tmp+1) = '\"';
			tmp += 2;
			html++;
			continue;
		}
		*tmp = *html;
		tmp++;
		html++;
	}
	*tmp = '\0';
	
	free(tmp_html);
	
	tmp_html = malloc(strlen(ptr) + 1);
	strncpy(tmp_html,ptr,strlen(ptr));
	
	free(ptr);
	
	return tmp_html;
}

int webtorm_db_visited(char *domain, char *path)
{
	MYSQL *conn;

	char *query;

	if ((domain == NULL) || (path == NULL))
		return 0;

	conn = mysql_init(NULL);
	
	query = malloc(42 + strlen(domain) + strlen(path));
	
	/* Connect to database */
	if (!mysql_real_connect(conn, SERVER,USER, PASSWORD, DATABASE, 0, NULL, 0))
	{
		fprintf(stderr, "%s\n", mysql_error(conn));
		return 0;
	}
	/* send SQL query */
	
	sprintf(query,"INSERT INTO visited VALUES ( '%s' )",domain);

	if (mysql_query(conn, query))
	{
		fprintf(stderr, "%s\n", mysql_error(conn));
	}
	
	sprintf(query,"INSERT INTO lastvisited VALUES ( '%s', '%s' )",domain,path);

	if (mysql_query(conn, query))
	{
		fprintf(stderr, "%s\n", mysql_error(conn));
	}

	mysql_close(conn);
	free(query);
	return 1;
}



int webtorm_db_isvisited(char *domain, char *path)
{
	MYSQL *conn;
	MYSQL_RES *res;
	MYSQL_ROW row;
	
	char *query;
   
	conn = mysql_init(NULL);
		
	/* Connect to database */
	if (!mysql_real_connect(conn, SERVER,USER, PASSWORD, DATABASE, 0, NULL, 0))
	{
		fprintf(stderr, "%s\n", mysql_error(conn));
		return 0;
	}
	
	/* send SQL query */
	if ((domain == NULL) && (path == NULL))
	{
		query = "TRUNCATE TABLE lastvisited";
		if (mysql_query(conn, query))
			fprintf(stderr, "%s\n", mysql_error(conn));
		mysql_close(conn);
		return 0;
	}

	query = malloc(78 + strlen(domain) + strlen(path));

	sprintf(query,"SELECT domain, path FROM lastvisited WHERE domain = '%s' AND path = '%s'",domain,path);

	if (mysql_query(conn, query))
	{
		fprintf(stderr, "%s\n", mysql_error(conn));
		free(query);
		mysql_close(conn);
		return 0;
	}

	res = mysql_use_result(conn);
   
	if ((row = mysql_fetch_row(res)) != NULL)
		return 1;

	/* release memory used to store results and close connection */
	free(query);
	mysql_free_result(res);
	mysql_close(conn);
	
	return 0;
}


int webtorm_db_store(char *domain, char *path, char *html)
{
	MYSQL *conn;

	char *query;

	if ((domain == NULL) || (path == NULL) || (html == NULL))
		return 0;
		
	html = webtorm_db_strip(html);

	query = malloc(39 + strlen(domain) + strlen(path) + strlen(html));

	conn = mysql_init(NULL);
		
	/* Connect to database */
	if (!mysql_real_connect(conn, SERVER,USER, PASSWORD, DATABASE, 0, NULL, 0))
	{
		fprintf(stderr, "%s\n", mysql_error(conn));
		return 0;
	}
	/* send SQL query */
	
	sprintf(query,"INSERT INTO webs VALUES ( '%s', '%s', '%s' )",domain,path,html);

	if (mysql_query(conn, query))
	{
		fprintf(stderr, "%s\n", mysql_error(conn));
	}

	mysql_close(conn);
	free(query);
	free(html);
	return 1;
}
