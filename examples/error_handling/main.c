#include <stdio.h>
#include <string.h> //strcpy()
#include <libgen.h> //basename()

#include "../../pobcurl.c"

int main(int argc, char** argv)
{
	pobInfo inf;    
	
	strcpy(inf.url,"http://www.freemedialab.org/wiki/lib/tpl/dokuwiki/images/logo.png_XXXwrong");  //url from web
	strcpy(inf.dest,basename(inf.url));  //dest filename
	pobCurlStartDownload(&inf);
	
	//Connection Error
	size_t len = strlen(inf.errbuf);
	
	if (len)
	{
		 
		 printf("Download error:%s\n",inf.errbuf);
		 pobCurlCleanErr(&inf);
	}
	
	//Http error
	if(inf.http_code>=400)
	{
		printf("Http error %ld\n", inf.http_code);
		unlink(inf.dest);
	}
	
	//End download
	if (inf.end==true)
	{
		 printf("Download termined.\n");
		 pobCurlClean(&inf);
	}
	
	
}
