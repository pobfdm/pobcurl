#include <stdio.h>
#include <string.h> //strcpy()
#include <libgen.h> //basename()

#include "../../pobcurl.c"

int main(int argc, char** argv)
{
	pobInfo inf;    

	strcpy(inf.url,"http://www.freemedialab.org/wiki/lib/tpl/dokuwiki/images/logo.png");  //url from web
	strcpy(inf.dest,basename(inf.url));  //dest filename
	pobCurlStartDownload(&inf);
}
