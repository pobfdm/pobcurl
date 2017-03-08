#include <curl/curl.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>

#define POB_MAX_LEN 500

typedef struct {
   char  url[POB_MAX_LEN];
   char  dest[POB_MAX_LEN];
   double dltotal;
   double dlnow;
   double percent;
   char errbuf[CURL_ERROR_SIZE];
   bool end ;
   long http_code ;
   int abort;
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
	inf->errbuf[0]=0;
}

void pobCurlClean(pobInfo* inf)
{
	inf->url[0]=0;
	inf->dest[0]=0;
	inf->errbuf[0]=0;
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
