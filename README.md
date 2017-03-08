#Pobcurl
A simple wrapper for libCurl

How works?

Include pobcurl:

	#include "../../pobcurl.c"

Start:
	
	pobInfo inf;	

	strcpy(inf.url,"http://example.com/myfile.png");  //url from web
	strcpy(inf.dest,basename(inf.url));  //dest filename
	pobCurlStartDownload(&inf);

You can use thread with function "pobCurlStartDownloadThread()":

	pobCurlStartDownloadThread(&inf);

See the folder "examples" for more info.
