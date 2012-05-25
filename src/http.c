#include "gog.h"
size_t static write_callback(void *buffer, size_t size, size_t nmemb, void *userp) {
    char **response_ptr =  (char**)userp;
    *response_ptr = strndup(buffer, (size_t)(size *nmemb));
	 return strlen(*response_ptr);
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
	free(error);

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
struct message_t *setup_handler(struct oauth_t *oauth, char *reply) {
	struct json_object *answer;

	if(oauth->msg != NULL)
		free(oauth->msg);
	oauth->msg = malloc(sizeof(struct message_t));

	answer = json_tokener_parse(reply);
	oauth->msg->result = strcmp(json_object_get_string(json_object_object_get(answer, "result")), "ok") == 0 ? 1 : 0;
	oauth->msg->timestamp = json_object_get_int(json_object_object_get(answer, "timestamp"));

	json_object_put(answer);

	return oauth->msg;
}
