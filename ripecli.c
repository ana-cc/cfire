#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cJSON.h"

#include <curl/curl.h>

struct memory {
	char *response;
	size_t size;
};

static size_t
cb(void *data, size_t size, size_t nmemb, void *userp)
{
	size_t realsize = size * nmemb;
	struct memory *mem = (struct memory *)userp;

	char *ptr = realloc(mem->response, mem->size + realsize + 1);
	if(ptr == NULL)
		return 0;	/* out of memory! */

	mem->response = ptr;
	memcpy(&(mem->response[mem->size]), data, realsize);
	mem->size += realsize;
	mem->response[mem->size] = 0;

	return realsize;
}

void
ripe_lookup(char *ip, struct memory *chunk) {
	char ripeURL[300];
	
	strcpy(ripeURL, "https://stat.ripe.net/data/prefix-overview/data.json?resource=");
	strcat(ripeURL, ip);

	CURL *curl = curl_easy_init();
	if (!curl) {
		printf("Failed to initialise cURL\n");
	}

	CURLcode res;
	curl_easy_setopt(curl, CURLOPT_URL, ripeURL);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, cb);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)chunk);
	res = curl_easy_perform(curl);
	curl_easy_cleanup(curl);
}

int
main(int argc, char **argv) {
	struct memory chunk;

	if (argc < 2) {
		printf("No IP address provided\n");
		return 1;
	}

	chunk.response = malloc(256);
	chunk.size = 0;

	ripe_lookup(argv[1], &chunk);

	cJSON *result = cJSON_CreateObject();
	cJSON_AddStringToObject(result, "ip", argv[1]);

	cJSON *json = cJSON_Parse(chunk.response);
	cJSON *data = cJSON_GetObjectItemCaseSensitive(json, "data");
	cJSON *asns = cJSON_GetObjectItemCaseSensitive(data, "asns");
	const cJSON *a = NULL;

	cJSON_ArrayForEach(a, asns){
		cJSON *asn = cJSON_GetObjectItemCaseSensitive(a, "asn");
		if (cJSON_IsNumber(asn) && (asn->valueint != 0)) {
			cJSON_AddNumberToObject(result, "asn", asn->valueint );
		}
		cJSON *holder = cJSON_GetObjectItemCaseSensitive(a, "holder");

		if (cJSON_IsString(holder) && (holder->valuestring != NULL)) {
			cJSON_AddStringToObject(result, "holder", holder->valuestring );
		}

		printf("%s\n", cJSON_Print(result));
	}
	cJSON_Delete(json);
	cJSON_Delete(result);
	free(chunk.response);

	return 0;
}
