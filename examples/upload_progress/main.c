#include <stdio.h>
#include <string.h> //strdup()
#include <libgen.h> //basename()

#include "../../pobcurl.c"

int main(int argc, char** argv)
{
	pobInfo inf;
	char user[20];
	char password[20];
	char userPass[40];    

	inf.upsrc=strdup("pobvnc-setup.exe"); //local file
	inf.url=strdup("sftp:///192.168.1.3/home/pi/Downloads/pobvnc-setup.exe"); //url 
	
	printf("User: "); 
	fscanf(stdin,"%s",user);
	
	printf("Password: "); 
	fscanf(stdin,"%s",password);
	
	sprintf(userPass,"%s:%s",user,password);
	
	inf.userPass=strdup(userPass); //user and password
	inf.port=22; //port
	pobCurlStartUploadThread(&inf);
	
	
	while (inf.end==false)
	{
		sleep(1);

		//Print progress
		if(inf.percent>0) printf("Uploading %d\% \n", (int)inf.percent);
		
			
		//Connection Error
		size_t len = strlen(inf.errbuf);
		if (len)
		{
			 
			 printf("Upload error:%s\n",inf.errbuf);
			 pobCurlCleanErr(&inf);
		}
		
		
	}	
	//End upload
	if (inf.end==true)
	{
		 printf("Upload termined.\n");
		 pobCurlClean(&inf);
	}
	
	return 0;
	
}
