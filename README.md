#Pobcurl
A simple wrapper for libCurl

How works?

Include pobcurl:

	#include "../../pobcurl.c"

Start:
	
	pobInfo inf;	

	inf.url=strdup("http://www.example.org/myfile.txt");
	inf.dest=strdup(basename(inf.url));
	pobCurlStartDownload(&inf);

You can use thread with function "pobCurlStartDownloadThread()":

	pobCurlStartDownloadThread(&inf);

See the folder "examples" for more info.
