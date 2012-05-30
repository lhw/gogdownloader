#include "gog.h"
size_t static write_callback(void *buffer, size_t size, size_t nmemb, void *userp) {
    char **response_ptr =  (char**)userp;
    *response_ptr = strndup(buffer, (size_t)(size *nmemb));
	 return strlen(*response_ptr);
}
size_t static file_write_callback(void *buffer, size_t size, size_t nmemb, void *userp) {
	struct active_t *active = (struct active_t *)userp;
	if(!active->current && active->from)
		fseek(active->file, active->from, SEEK_SET);
	return (active->current = fwrite(buffer, size, nmemb, active->file));
}
size_t get_remote_file_size(char *url) {
	CURL *curl;
	double length;

	curl = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_NOBODY, 1);
	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_perform(curl);

	curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &length);
	curl_easy_cleanup(curl);

	return (long)length;
}
CURL *create_download_handle(struct active_t *a) {
	CURL *curl;
	char *range;

	range = malloc(22);
	sprintf(range, "%d-%d", a->from, a->to);
	a->file = fopen(a->info->path, "r+");
	if(!a->file)
		return NULL;

	curl = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, file_write_callback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, a);
	curl_easy_setopt(curl, CURLOPT_URL, a->dl->link);
	curl_easy_setopt(curl, CURLOPT_RANGE, range); 

	free(range);

	return curl;
}
int http_get(const char *url, char **buffer, char **error_msg) {
	CURL *curl;
	CURLcode res;
	char *error;

	curl = curl_easy_init();
	error = malloc(1000);

	curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, &error); 
	curl_easy_setopt(curl, CURLOPT_HTTPGET, 1);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, buffer);
	curl_easy_setopt(curl, CURLOPT_URL, url);

	if((res = curl_easy_perform(curl)) != 0 && error_msg != NULL) {
		if(*error_msg)
			free(*error_msg);
		*error_msg = strdup(error);
	}
	if(!*buffer) {
		res = 1;
		if(error_msg != NULL) {
			if(*error_msg)
				free(*error_msg);
			*error_msg = "Failed for unknown reason";
		}
	}

	curl_easy_cleanup(curl);
	curl_free(error);

	return res == 0 ? 1 : 0;
}
int http_get_oauth(struct oauth_t *oauth, const char *url, char **buffer) {
	char *req_url;	
	int res;

	req_url = oauth_sign_url2(url, NULL, OA_HMAC, NULL, CONSUMER_KEY, CONSUMER_SECRET, oauth->token, oauth->secret);
	res = http_get(req_url, buffer, &(oauth->error));
	free(req_url);
	return res;
}
