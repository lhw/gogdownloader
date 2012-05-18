#include "gog.h"

size_t static write_callback(void *buffer, size_t size, size_t nmemb, void *userp) {
    char **response_ptr =  (char**)userp;
    *response_ptr = strndup(buffer, (size_t)(size *nmemb));
	 return strlen(*response_ptr);
}
char *oauth_http_get3(const char *url) {
	CURL *curl;
	char *reply, *error_msg;

	curl = curl_easy_init();
	error_msg = malloc(1000);

	curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, error_msg); 
	curl_easy_setopt(curl, CURLOPT_HTTPGET, 1);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &reply);
	curl_easy_setopt(curl, CURLOPT_URL, url);

	if(curl_easy_perform(curl) != 0) {
		printf("%s\n", error_msg);
		puts(url);
	}

	free(error_msg);
	curl_easy_cleanup(curl);

	return reply;
}
int gog_request_token(char **token, char **secret) {
	char *req_url = NULL, *reply = NULL, **rv = NULL;
	int rc;

	req_url = oauth_sign_url2(config.oauth_get_temp_token, NULL, OA_HMAC, NULL, CONSUMER_KEY, CONSUMER_SECRET, NULL, NULL);
	reply = oauth_http_get3(req_url);

	free(req_url);

	if(reply) {
		rc = oauth_split_url_parameters(reply, &rv);
		qsort(rv, rc, sizeof(char *), oauth_cmpstringp);
		if(rc == 3 && !strncmp(rv[1], "oauth_token=", 11) && !strncmp(rv[2], "oauth_token_secret=", 18)) {
			*token = malloc(KEY_LENGTH + 1);
			*secret = malloc(KEY_LENGTH + 1);

			strcpy(*token, rv[1]+12);
			strcpy(*secret, rv[2]+19);

			free(rv);
			free(reply);

			return 1;
		}
	}
	if(reply)
		free(reply);
	return 0;
}
int gog_access_token(const char *email, const char *password, const char *key, const char *secret, char **verifier) {
	char *req_url, *reply = NULL, **rv = NULL, *login_uri = NULL, *user_enc = NULL, *password_enc = NULL;
	int rc;

	user_enc = oauth_url_escape(email);
	password_enc = oauth_url_escape(password);

	login_uri = malloc(strlen(LOGIN_PARAM) - 6 + strlen(config.oauth_authorize_temp_token) + strlen(user_enc) + strlen(password_enc));
	sprintf(login_uri, LOGIN_PARAM, config.oauth_authorize_temp_token, user_enc, password_enc);

	req_url = oauth_sign_url2(login_uri, NULL, OA_HMAC, NULL, CONSUMER_KEY, CONSUMER_SECRET, key, secret);
	reply = oauth_http_get3(req_url);

	free(user_enc);
	free(password_enc);
	free(login_uri);
	free(req_url);

	if(reply) { 
		rc = oauth_split_url_parameters(reply, &rv);
		qsort(rv, rc, sizeof(char *), oauth_cmpstringp);
		if(rc == 2 && !strncmp(rv[1], "oauth_verifier=", 14)) {
			*verifier = malloc(KEY_LENGTH + 1);
			strcpy(*verifier, rv[1]+15);

			free(rv);
			free(reply);

			return 1;
		}
	}
	if(reply)
		free(reply);
	return 0;
}
int gog_token(const char *auth_token, const char *auth_secret, const char *verifier, char **token, char **secret) {
	char *req_url, *reply = NULL, **rv = NULL, *token_uri = NULL;
	int rc;

	token_uri = malloc(strlen(TOKEN_PARAM) - 4 + strlen(config.oauth_get_token) + KEY_LENGTH);
	sprintf(token_uri, TOKEN_PARAM, config.oauth_get_token, verifier);

	req_url = oauth_sign_url2(token_uri, NULL, OA_HMAC, NULL, CONSUMER_KEY, CONSUMER_SECRET, auth_token, auth_secret);
	reply = oauth_http_get3(req_url);

	free(token_uri);
	free(req_url);

	if(reply) {
		rc = oauth_split_url_parameters(reply, &rv);
		qsort(rv, rc, sizeof(char *), oauth_cmpstringp);
		if(rc == 2 && !strncmp(rv[0], "oauth_token=", 11) && !strncmp(rv[1], "oauth_token_secret=", 18)) {
			*token = malloc(KEY_LENGTH + 1);
			*secret = malloc(KEY_LENGTH + 1);

			strcpy(*token, rv[0]+12);
			strcpy(*secret, rv[1]+19);

			free(rv);
			free(reply);

			return 1;
		}
	}
	if(reply)
		free(reply);
	return 0;
}
int gog_login(const char *email, const char *password, char **user_token, char **user_secret) {
	char *key = NULL, *secret = NULL, *verifier = NULL; 

	if(!gog_request_token(&key, &secret))
		goto cleanup;
	if(!gog_access_token(email, password, key, secret, &verifier))
		goto cleanup;
	if(!gog_token(key, secret, verifier, user_token, user_secret))
		goto cleanup;

cleanup:
	if(key)
		free(key);
	if(secret)
		free(secret);
	if(verifier)
		free(verifier);

	return *user_token != NULL;
}

int main() {
	char *token = NULL, *secret = NULL;

	curl_global_init(CURL_GLOBAL_SSL);

	config.oauth_get_temp_token = "https://api.gog.com/en/oauth/initialize/";
	config.oauth_authorize_temp_token = "https://api.gog.com/en/oauth/login/";
	config.oauth_get_token = "https://api.gog.com/en/oauth/token/";

	if(gog_login("foo@bar", "foobar2000", &token, &secret))
		printf("Token: %s\nSecret: %s\n", token, secret);
}
