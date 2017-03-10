#include <stdio.h>
#include <string.h> //strdup()
#include <libgen.h> //basename()

#include "../../pobcurl.c"

int main(int argc, char** argv)
{
	pobInfo inf;    

	inf.url=strdup("http://www.freemedialab.org/wiki/lib/tpl/dokuwiki/images/logo.png");
	inf.dest=strdup(basename(inf.url));
	pobCurlStartDownload(&inf);
}
