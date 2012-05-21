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

	if((res = curl_easy_perform(curl)) != 0)
		*error_msg = strdup(error);
	if(!*buffer) {
		res = 1;
		*error_msg = "Failed for unknown reason";
	}

	curl_easy_reset(curl);
	free(error);

	return res == 0 ? 1 : 0;
}
int gog_request_token(char **token, char **secret, char **error) {
	char *req_url = NULL, *reply = NULL, **rv = NULL;
	int res, rc;

	req_url = oauth_sign_url2(config.oauth_get_temp_token, NULL, OA_HMAC, NULL, CONSUMER_KEY, CONSUMER_SECRET, NULL, NULL);

	if((res = http_get(req_url, &reply, error))) {
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
	free(req_url);
	if(reply)
		free(reply);
	return res;
}
int gog_access_token(const char *email, const char *password, const char *key, const char *secret, char **verifier, char **error) {
	char *req_url = NULL, *reply = NULL, **rv = NULL, *login_uri = NULL, *user_enc = NULL, *password_enc = NULL;
	int res, rc;

	user_enc = oauth_url_escape(email);
	password_enc = oauth_url_escape(password);

	login_uri = malloc(strlen(LOGIN_PARAM) - 6 + strlen(config.oauth_authorize_temp_token) + strlen(user_enc) + strlen(password_enc));
	sprintf(login_uri, LOGIN_PARAM, config.oauth_authorize_temp_token, user_enc, password_enc);

	req_url = oauth_sign_url2(login_uri, NULL, OA_HMAC, NULL, CONSUMER_KEY, CONSUMER_SECRET, key, secret);

	free(user_enc);
	free(password_enc);
	free(login_uri);

	if((res = http_get(req_url, &reply, error))) {
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
	free(req_url);
	if(reply)
		free(reply);
	return res;
}
int gog_token(const char *auth_token, const char *auth_secret, const char *verifier, char **token, char **secret, char **error) {
	char *req_url = NULL, *reply = NULL, **rv = NULL, *token_uri = NULL;
	int res, rc;

	token_uri = malloc(strlen(TOKEN_PARAM) - 4 + strlen(config.oauth_get_token) + KEY_LENGTH);
	sprintf(token_uri, TOKEN_PARAM, config.oauth_get_token, verifier);

	req_url = oauth_sign_url2(token_uri, NULL, OA_HMAC, NULL, CONSUMER_KEY, CONSUMER_SECRET, auth_token, auth_secret);

	free(token_uri);

	if((res = http_get(req_url, &reply, error))) {
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
	free(req_url);
	if(reply)
		free(reply);
	return res;
}
int gog_login(const char *email, const char *password, char **user_token, char **user_secret, char **error) {
	char *key = NULL, *secret = NULL, *verifier = NULL; 

	if(!gog_request_token(&key, &secret, error))
		goto cleanup;
	if(!gog_access_token(email, password, key, secret, &verifier, error))
		goto cleanup;
	if(!gog_token(key, secret, verifier, user_token, user_secret, error))
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
int gog_game_details(const char *token, const char *secret, const char *game, char **error) {
	char *req_url = NULL, *reply = NULL, *game_details_uri = NULL;
	int res;

	game_details_uri = malloc(strlen(config.get_game_details) + strlen(game) + 2);
	sprintf(game_details_uri, "%s%s/", config.get_game_details, game);

	req_url = oauth_sign_url2(game_details_uri, NULL, OA_HMAC, NULL, CONSUMER_KEY, CONSUMER_SECRET, token, secret);
	if((res = http_get(req_url, &reply, error))) {
		puts(reply);
	}

	free(game_details_uri);
	free(req_url);
	if(reply)
		free(reply);

	return 0;
}
int gog_user_details(const char *token, const char *secret, char **error) {
	char *req_url = NULL, *reply = NULL;
	int res;

	req_url = oauth_sign_url2(config.get_user_details, NULL, OA_HMAC, NULL, CONSUMER_KEY, CONSUMER_SECRET, token, secret);
	if((res = http_get(req_url, &reply, error))) {
		puts(reply);
	}


	free(req_url);
	if(reply)
		free(reply);

	return res;
}
int gog_extra_link(const char *token, const char *secret, const char *game, const short file_id, char **error) {
	char *req_url = NULL, *reply = NULL, *extra_link_uri = NULL;
	int res;

	extra_link_uri = malloc(strlen(config.get_extra_link) + strlen(game) + 4);
	sprintf(extra_link_uri, "%s%s/%d/", config.get_extra_link , game, file_id);

	req_url = oauth_sign_url2(extra_link_uri, NULL, OA_HMAC, NULL, CONSUMER_KEY, CONSUMER_SECRET, token, secret);
	if((res = http_get(req_url, &reply, error))) {
		puts(reply);
	}

	free(extra_link_uri);
	free(req_url);
	if(reply)
		free(reply);

	return res;
}
int gog_installer_link(const char *token, const char *secret, const char *game, const short file_id, char **error){
	char *req_url = NULL, *reply = NULL, *installer_link_uri = NULL;
	int res;

	installer_link_uri = malloc(strlen(config.get_installer_link) + strlen(game) + 4);
	sprintf(installer_link_uri, "%s%s/%d/", config.get_installer_link , game, file_id);

	req_url = oauth_sign_url2(installer_link_uri, NULL, OA_HMAC, NULL, CONSUMER_KEY, CONSUMER_SECRET, token, secret);
	if((res = http_get(req_url, &reply, error))) {
		puts(reply);
	}

	free(installer_link_uri);
	free(req_url);
	if(reply)
		free(reply);

	return res;
}
int gog_user_games(const char *token, const char *secret, char **error) {
	char *req_url = NULL, *reply = NULL;
	int res;

	req_url = oauth_sign_url2(config.get_user_games, NULL, OA_HMAC, NULL, CONSUMER_KEY, CONSUMER_SECRET, token, secret);
	if((res = http_get(req_url, &reply, error))) {
		puts(req_url);
	}

	return 0;
}
int gog_download_config(const char *release, char **error) {
	char *release_url = NULL, *reply = NULL;
	struct json_object *content, *config_node;
	int res;

	release_url = malloc(strlen(CONFIG_URL) - 2 + strlen(release));
	sprintf(release_url, CONFIG_URL, release);

	if((res = http_get(release_url, &reply, error))) {
		content = json_tokener_parse(reply);
		config_node = json_object_object_get(content, "config");
		
		config.get_extra_link = strdup(json_object_get_string(json_object_object_get(config_node, "get_extra_link")));
		config.get_game_details = strdup(json_object_get_string(json_object_object_get(config_node, "get_game_details")));
		config.get_installer_link = strdup(json_object_get_string(json_object_object_get(config_node, "get_installer_link")));
		config.get_user_details = strdup(json_object_get_string(json_object_object_get(config_node, "get_user_details")));
		config.get_user_games = strdup(json_object_get_string(json_object_object_get(config_node, "get_user_games")));
		config.oauth_authorize_temp_token = strdup(json_object_get_string(json_object_object_get(config_node, "oauth_authorize_temp_token")));
		config.oauth_get_temp_token = strdup(json_object_get_string(json_object_object_get(config_node, "oauth_get_temp_token")));
		config.oauth_get_token = strdup(json_object_get_string(json_object_object_get(config_node, "oauth_get_token")));
		config.set_app_status = strdup(json_object_get_string(json_object_object_get(config_node, "set_app_status")));

		json_object_put(config_node);
		json_object_put(content);
	}

	free(release_url);
	if(reply)
		free(reply);

	return res;
}
int main() {
	char *token = NULL, *secret = NULL, *error;

	curl_global_init(CURL_GLOBAL_SSL);

	gog_download_config(DEFAULT_RELEASE, &error);

	/*if(gog_login("foo@bar.com", "foobar2000", &token, &secret, &error))
		printf("Token: %s\nSecret: %s\n", token, secret);*/
	token = "3f4a856709f0eefee38ef0fde5104f514a768a6e";
	secret = "0b48035c22968d099f0ddc6b8856ef67f55a835a";

	//gog_user_games(token, secret, &error);
	//gog_game_details(token, secret, "tyrian_2000", &error);
	//gog_installer_link(token, secret, "tyrian_2000", 0, &error);
	gog_extra_link(token, secret, "tyrian_2000", 967, &error); //WORKING
	if(!gog_extra_link(token, secret, "tyrian_2000", 967, &error)) { //NOT EXISTANT
		puts(error);
	}
	//gog_user_details(token, secret, &error);
}
