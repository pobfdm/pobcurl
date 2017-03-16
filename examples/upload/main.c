#include <stdio.h>
#include <string.h> //strdup()
#include <libgen.h> //basename()

#include "../../pobcurl.c"

int main(int argc, char** argv)
{
	pobInfo inf;    

	inf.upsrc=strdup("pobvnc-setup.exe"); //local file
	inf.url=strdup("sftp:///192.168.1.3/home/pi/Downloads/pobvnc-setup.exe"); //url 
	inf.userPass=strdup("user:password"); //user and password
	inf.port=22; //port
	pobCurlStartUpload(&inf);
}
