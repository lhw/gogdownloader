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
off_t get_remote_file_size(char *url) {
	CURL *curl;
	double length;

	curl = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_NOBODY, 1);
	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_perform(curl);

	curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &length);
	curl_easy_cleanup(curl);

	return (off_t)length;
}
int create_download_handle(struct active_t *a) {
	char *range;

	range = malloc(22);
	sprintf(range, "%ld-%ld", a->from, a->to);
	a->file = fopen(a->info->file->path, "r+");
	if(!a->file)
		return 0;

	a->curl = curl_easy_init();
	curl_easy_setopt(a->curl, CURLOPT_WRITEFUNCTION, file_write_callback);
	curl_easy_setopt(a->curl, CURLOPT_WRITEDATA, a);
	curl_easy_setopt(a->curl, CURLOPT_URL, a->info->link);
	curl_easy_setopt(a->curl, CURLOPT_RANGE, range); 

	free(range);

	return 1;
}
int create_partial_download(struct download_t *dl, int n) {
	FILE *create;
	char *directory, *seperator;
	off_t length, chunk;
	struct file_t *file;

	file = dl->file;
	directory = malloc(100);

	seperator = strchr(file->path+1, '/');
	strcpy(directory, file->path);
	directory[seperator - file->path] = 0;
	if(mkdir(directory, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) && errno != EEXIST) {
		free(directory);
		return 0;
	}
	free(directory);

	dl->active = malloc(n * sizeof(struct active_t));

	create = fopen(file->path, "w+");
	fclose(create);

	length = get_remote_file_size(dl->link);
	chunk = length / n;

	for(int i = 0; i < n; i++) {
		dl->active[i].info = dl;
		dl->active[i].file = fopen(file->path, "r+");
		dl->active[i].from = i * chunk;
		dl->active[i].to = (i * chunk) + chunk;
		if(dl->active[i].to + chunk >= length)
			dl->active[i].to = length;
		dl->active[i].chunk_size = chunk;

		if(create_download_handle(dl->active)) {
			curl_multi_add_handle(dl->multi, dl->active[i].curl);
		}
	}

	return 1;
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
		if(*error_msg && strlen(*error_msg) > 1)
			free(*error_msg);
		*error_msg = strdup(error);
	}
	if(!*buffer) {
		res = 1;
		if(error_msg != NULL) {
			if(*error_msg && strlen(*error_msg) > 1)
				free(*error_msg);
			*error_msg = "Failed for unknown reason";
		}
	}
	/* XXX: dirty check here. really have to change this */
	if(*buffer[0] == '<') {
		res = 1;
		if(*error_msg && strlen(*error_msg) > 1)
			free(*error_msg);
		*error_msg = strdup(*buffer);
		curl_free(*buffer);
		*buffer = NULL;
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
int http_get_json(struct oauth_t *oauth, const char *url, char **buffer) {
	int res;
	struct json_object *answer;
	char *error_msg;
	
	error_msg = oauth->error;
	res = http_get_oauth(oauth, url, buffer);
	if(res) {
		answer = json_tokener_parse(*buffer);
		if(is_error(answer)) {
			res = 0;
			if(error_msg && strlen(error_msg) > 1)
				free(error_msg);
			error_msg = strdup(*buffer);
			curl_free(*buffer);
			*buffer = NULL;
		}
		else
			json_object_put(answer);
	}

	return res;
}
