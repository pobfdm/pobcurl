#include <curl/curl.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>

#include <sys/stat.h> //upload
#include <fcntl.h>

typedef struct {
	int id;
	char*  url;
	char*  dest;
	char*  upsrc;
	char* userPass;
	int port;	 
	double dltotal;
	double dlnow;
	double percent;
	int percentInt;
	char errbuf[CURL_ERROR_SIZE];
	bool end ;
	long http_code ;
	int abort;
	double  total_time;
} pobInfo;


//Questa callback scrive il file scaricato
size_t writecallback(void *buffer, size_t size, size_t nmemb, void *stream){
 
	int written = fwrite(buffer, size, nmemb, (FILE *)stream);
   	return written;
}

//Questa callback scrive il progresso sulla variabile "progress" 
int progressDownload(pobInfo *inf,
                     double t, /* dltotal */ 
                     double d, /* dlnow */ 
                     double ultotal,
                     double ulnow) 
{
 
	inf->dltotal=t;
	inf->dlnow=d;
	inf->percent= d*100/t;
	inf->percentInt=(int)inf->percent;
	
	return inf->abort;
 
}

void *pobCurlDownload(pobInfo* inf)
{
	CURL *curl;
	CURLcode res;
	
	
	inf->end=false;
	inf->http_code=0;
	inf->abort=0;

	curl = curl_easy_init();
	if(curl) {

		curl_easy_setopt(curl, CURLOPT_URL, inf->url);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writecallback);
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
		curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, progressDownload);
		curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, inf);
		curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, inf->errbuf);
		
		
		
		FILE *f = fopen(inf->dest,"w");
		if (f == NULL) {
			curl_easy_cleanup(curl);
			inf->end=true;
		}

		curl_easy_setopt(curl,CURLOPT_WRITEDATA, f);
		res = curl_easy_perform(curl);
		if(res != CURLE_OK)
		{
				strcpy(inf->errbuf,curl_easy_strerror(res));
				
		}
		
		//Http code return
		curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &inf->http_code);
		if (inf->http_code == 200 && res != CURLE_ABORTED_BY_CALLBACK)
		{
				 //Succeeded
		}else{
				 //Failed
		}
		
		curl_easy_cleanup(curl);
		fclose(f);
		inf->end=true;
	}
  	
	
}

void pobCurlCleanErr(pobInfo* inf )
{
	strcpy(inf->errbuf,"");
}

void pobCurlClean(pobInfo* inf)
{
	strcpy(inf->errbuf,"");
	inf->http_code=0;
	inf->abort=0;
}


int pobCurlStartDownload(pobInfo* inf)
{
	pobCurlDownload(inf);
	
}

int pobCurlStartDownloadThread(pobInfo* inf)
{
	pthread_t pobThread;
	int  res;

	res = pthread_create( &pobThread, NULL, (void *)pobCurlDownload,  inf);
	if(res)
	{
		 fprintf(stderr,"Error - pthread_create() return code: %d\n",res);
		 return -1;
	}
	
	return 0;
}


int upload_callback (pobInfo *inf, double dltotal, double dlnow,
double ultotal, double ulnow)
{
	inf->dltotal=ultotal;
	inf->dlnow=ulnow;
	inf->percent= ulnow*100/ultotal;
	inf->percentInt=(int)inf->percent;
	
	return inf->abort;
}


void *pobCurlUpload(pobInfo* inf)
{
	CURL *curl;
	CURLcode res;
	inf->end=false;
	inf->http_code=0;
	inf->abort=0;
	inf->total_time=0;
	
	
	
	struct stat file_info;
	
	FILE *fd;

	fd = fopen(inf->upsrc, "rb"); /* open file to upload */ 
	if(!fd)
	{
		
		return NULL; /* can't continue */ 
	}
	
	/* to get the file size */ 
	if(fstat(fileno(fd), &file_info) != 0)
	{
		return NULL; /* can't continue */ 
	}
	
	curl = curl_easy_init();
	if(curl) 
	{
		curl_easy_setopt(curl, CURLOPT_URL,	 inf->url);

		curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
		
		curl_easy_setopt(curl, CURLOPT_PORT, inf->port);
		
		curl_easy_setopt(curl,CURLOPT_USERPWD,inf->userPass);
		
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
		curl_easy_setopt(curl,CURLOPT_PROGRESSFUNCTION,upload_callback);
		curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, inf);
		curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, inf->errbuf);

		curl_easy_setopt(curl, CURLOPT_READDATA, fd);

		curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)file_info.st_size);

		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

		res = curl_easy_perform(curl);
		/* Check for errors */ 
		if(res != CURLE_OK) 
		{
		  //fprintf(stderr, "curl_easy_perform() failed: %s\n",  curl_easy_strerror(res));
		  strcpy(inf->errbuf,curl_easy_strerror(res));

		}else {
		  curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &inf->total_time);


		}
		/* always cleanup */ 
		curl_easy_cleanup(curl);
	}
	fclose(fd);
	inf->end=true;
}


int pobCurlStartUpload(pobInfo* inf)
{
	pobCurlUpload(inf);
	
}

int pobCurlStartUploadThread(pobInfo* inf)
{
	pthread_t pobThread;
	int  res;

	res = pthread_create( &pobThread, NULL, (void *)pobCurlUpload,  inf);
	if(res)
	{
		 fprintf(stderr,"Error - pthread_create() return code: %d\n",res);
		 return -1;
	}
	
	return 0;
}
