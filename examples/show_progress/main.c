#include <stdio.h>
#include <string.h> //strcpy()
#include <libgen.h> //basename()

#include "../../pobcurl.c"

int main(int argc, char** argv)
{
	pobInfo inf;    
	
	inf.url=strdup("http://www.freemedialab.org/listing/pobvnc/Windows/pobvnc-setup.exe");
	inf.dest=strdup(basename(inf.url));
	pobCurlStartDownloadThread(&inf);
	
	while (inf.end==false)
	{
		sleep(1);

		//Print progress
		printf("Downloading %d\% \n", (int)inf.percent);
		
			
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
	}	
		//End download
		if (inf.end==true)
		{
			 printf("Download termined.\n");
			 pobCurlClean(&inf);
		}
	
	
}
