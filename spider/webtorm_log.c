#include <stdio.h>

void webtorm_log(char *string)
{
	FILE *fptr;
	char *mode;
	
	if (string == NULL)
	{
		string = "Webtorm iniciado<br>";
		mode = "w";
	}
	else
		mode = "a";
		
	if ((fptr = fopen("./public_html/webtorm.log",mode)) != NULL)
	{
		fprintf(fptr,"%s<br>\n",string);
		fclose(fptr);
	}
	else
		perror("fopen()");
}
