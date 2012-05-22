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
int gog_request_token(struct oauth_t *oauth) {
	char *req_url = NULL, *reply = NULL, **rv = NULL;
	int res, rc;

	req_url = oauth_sign_url2(config.oauth_get_temp_token, NULL, OA_HMAC, NULL, CONSUMER_KEY, CONSUMER_SECRET, NULL, NULL);

	if((res = http_get(req_url, &reply, &(oauth->error)))) {
		rc = oauth_split_url_parameters(reply, &rv);
		qsort(rv, rc, sizeof(char *), oauth_cmpstringp);
		if(rc == 3 && !strncmp(rv[1], "oauth_token=", 11) && !strncmp(rv[2], "oauth_token_secret=", 18)) {
			oauth->token = malloc(KEY_LENGTH + 1);
			oauth->secret = malloc(KEY_LENGTH + 1);

			strcpy(oauth->token, rv[1]+12);
			strcpy(oauth->secret, rv[2]+19);

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
int gog_access_token(struct oauth_t *oauth, const char *email, const char *password) {
	char *req_url = NULL, *reply = NULL, **rv = NULL, *login_uri = NULL, *user_enc = NULL, *password_enc = NULL;
	int res, rc;

	user_enc = oauth_url_escape(email);
	password_enc = oauth_url_escape(password);

	login_uri = malloc(strlen(LOGIN_PARAM) - 6 + strlen(config.oauth_authorize_temp_token) + strlen(user_enc) + strlen(password_enc));
	sprintf(login_uri, LOGIN_PARAM, config.oauth_authorize_temp_token, user_enc, password_enc);

	req_url = oauth_sign_url2(login_uri, NULL, OA_HMAC, NULL, CONSUMER_KEY, CONSUMER_SECRET, oauth->token, oauth->secret);

	free(user_enc);
	free(password_enc);
	free(login_uri);

	if((res = http_get(req_url, &reply, &(oauth->error)))) {
		rc = oauth_split_url_parameters(reply, &rv);
		qsort(rv, rc, sizeof(char *), oauth_cmpstringp);
		if(rc == 2 && !strncmp(rv[1], "oauth_verifier=", 14)) {
			oauth->verifier = malloc(KEY_LENGTH + 1);
			strcpy(oauth->verifier, rv[1]+15);

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
int gog_token(struct oauth_t *oauth) {
	char *req_url = NULL, *reply = NULL, **rv = NULL, *token_uri = NULL;
	int res, rc;

	token_uri = malloc(strlen(TOKEN_PARAM) - 4 + strlen(config.oauth_get_token) + KEY_LENGTH);
	sprintf(token_uri, TOKEN_PARAM, config.oauth_get_token, oauth->verifier);

	req_url = oauth_sign_url2(token_uri, NULL, OA_HMAC, NULL, CONSUMER_KEY, CONSUMER_SECRET, oauth->token, oauth->secret);

	free(token_uri);

	if((res = http_get(req_url, &reply, &(oauth->error)))) {
		rc = oauth_split_url_parameters(reply, &rv);
		qsort(rv, rc, sizeof(char *), oauth_cmpstringp);
		if(rc == 2 && !strncmp(rv[0], "oauth_token=", 11) && !strncmp(rv[1], "oauth_token_secret=", 18)) {
			strcpy(oauth->token, rv[0]+12);
			strcpy(oauth->secret, rv[1]+19);

			free(rv);
			free(reply);
			free(oauth->verifier);

			return 1;
		}
	}
	free(req_url);
	if(oauth->verifier)
		free(oauth->verifier);
	if(reply)
		free(reply);
	return res;
}
int gog_login(struct oauth_t *oauth, const char *email, const char *password) {
	int res;

	if(!(res = gog_request_token(oauth)))
		return res;
	if(!(res = gog_access_token(oauth, email, password)))
		return res;
	if(!(res = gog_token(oauth)))
		return res;

	return 1;
}
int gog_game_details(struct oauth_t *oauth, const char *game) {
	char *req_url = NULL, *reply = NULL, *game_details_uri = NULL;
	int res;

	game_details_uri = malloc(strlen(config.get_game_details) + strlen(game) + 2);
	sprintf(game_details_uri, "%s%s/", config.get_game_details, game);

	req_url = oauth_sign_url2(game_details_uri, NULL, OA_HMAC, NULL, CONSUMER_KEY, CONSUMER_SECRET, oauth->token, oauth->secret);
	if((res = http_get(req_url, &reply, &(oauth->error)))) {
		puts(reply);
	}

	free(game_details_uri);
	free(req_url);
	if(reply)
		free(reply);

	return 0;
}
int gog_user_details(struct oauth_t *oauth) {
	char *req_url = NULL, *reply = NULL;
	int res;

	req_url = oauth_sign_url2(config.get_user_details, NULL, OA_HMAC, NULL, CONSUMER_KEY, CONSUMER_SECRET, oauth->token, oauth->secret);
	if((res = http_get(req_url, &reply, &(oauth->error)))) {
		puts(reply);
	}


	free(req_url);
	if(reply)
		free(reply);

	return res;
}
int gog_extra_link(struct oauth_t *oauth, const char *game, const short file_id) {
	char *req_url = NULL, *reply = NULL, *extra_link_uri = NULL;
	int res;

	extra_link_uri = malloc(strlen(config.get_extra_link) + strlen(game) + 4);
	sprintf(extra_link_uri, "%s%s/%d/", config.get_extra_link , game, file_id);

	req_url = oauth_sign_url2(extra_link_uri, NULL, OA_HMAC, NULL, CONSUMER_KEY, CONSUMER_SECRET, oauth->token, oauth->secret);

	if((res = http_get(req_url, &reply, &(oauth->error)))) {
		puts(reply);
	}

	free(extra_link_uri);
	free(req_url);
	if(reply)
		free(reply);

	return res;
}
int gog_installer_link(struct oauth_t *oauth, const char *game, const short file_id) {
	char *req_url = NULL, *reply = NULL, *installer_link_uri = NULL;
	int res;

	installer_link_uri = malloc(strlen(config.get_installer_link) + strlen(game) + 4);
	sprintf(installer_link_uri, "%s%s/%d/", config.get_installer_link , game, file_id);

	req_url = oauth_sign_url2(installer_link_uri, NULL, OA_HMAC, NULL, CONSUMER_KEY, CONSUMER_SECRET, oauth->token, oauth->secret);
	if((res = http_get(req_url, &reply, &(oauth->error)))) {
		puts(reply);
	}

	free(installer_link_uri);
	free(req_url);
	if(reply)
		free(reply);

	return res;
}
int gog_user_games(struct oauth_t *oauth) {
	char *req_url = NULL, *reply = NULL;
	int res;

	req_url = oauth_sign_url2(config.get_user_games, NULL, OA_HMAC, NULL, CONSUMER_KEY, CONSUMER_SECRET, oauth->token, oauth->secret);
	if((res = http_get(req_url, &reply, &(oauth->error)))) {
		puts(req_url);
	}

	return 0;
}
int gog_download_config(struct oauth_t *oauth, const char *release) {
	char *release_url = NULL, *reply = NULL;
	struct json_object *content, *config_node;
	int res;

	release_url = malloc(strlen(CONFIG_URL) - 2 + strlen(release));
	sprintf(release_url, CONFIG_URL, release);

	if((res = http_get(release_url, &reply, &(oauth->error)))) {
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
int gog_installer_crc(struct oauth_t *oauth, const char *game, const short file_id) {
	char *req_url = NULL, *reply = NULL, *file_crc_uri = NULL;
	int res;

	file_crc_uri = malloc(strlen(config.get_installer_link) + strlen(game) + 8);
	sprintf(file_crc_uri, "%s%s/%d/crc/", config.get_installer_link , game, file_id);

	req_url = oauth_sign_url2(file_crc_uri, NULL, OA_HMAC, NULL, CONSUMER_KEY, CONSUMER_SECRET, oauth->token, oauth->secret);
	if((res = http_get(req_url, &reply, &(oauth->error)))) {
		puts(reply);
	}

	free(file_crc_uri);
	free(req_url);
	if(reply)
		free(reply);

	return res;
}
int main() {
	struct oauth_t *oauth;

	curl_global_init(CURL_GLOBAL_SSL);
	oauth = malloc(sizeof(struct oauth_t));
	oauth->token = oauth->secret = oauth->error = oauth->verifier = NULL;

	gog_download_config(oauth, DEFAULT_RELEASE);

	/*if(gog_login(oauth, "foo@foo.bar", "foobar2000"))
		printf("Token: %s\nSecret: %s\n", oauth->token, oauth->secret);*/
	oauth->token = "c67cfe7a15be042114b8a7df6acc11c0edcb8017";
	oauth->secret = "b2594007ba4dae9556448f03f4026a5c295989ea";

	gog_game_details(oauth, "beneath_a_steel_sky");
	/*
	gog_user_games(oauth);
	gog_installer_link(oauth, "beneath_a_steel_sky", 0);
	gog_installer_crc(oauth, "beneath_a_steel_sky", 0);
	gog_user_details(oauth);
	gog_extra_link(oauth, "tyrian_2000", 968); //WORKING
	if(!gog_extra_link(oauth, "tyrian_2000", 967)) { //NOT EXISTANT
		puts(oauth->error);
	}
	*/
	//free(oauth->token);
	//free(oauth->secret);
	free(oauth);
}
