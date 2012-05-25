#include "gog.h"

int gog_request_token(struct oauth_t *oauth) {
	char *reply = NULL, **rv = NULL;
	int res, rc;

	if((res = http_get_oauth(oauth, config.oauth_get_temp_token, &reply))) {
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
		else {
			oauth->error = reply;
			res = 0;
		}
	}
	if(reply)
		free(reply);
	return res;
}
int gog_access_token(struct oauth_t *oauth, const char *email, const char *password) {
	char *reply = NULL, **rv = NULL, *login_uri = NULL, *user_enc = NULL, *password_enc = NULL;
	int res, rc;

	user_enc = oauth_url_escape(email);
	password_enc = oauth_url_escape(password);

	login_uri = malloc(strlen(LOGIN_PARAM) - 6 + strlen(config.oauth_authorize_temp_token) + strlen(user_enc) + strlen(password_enc));
	sprintf(login_uri, LOGIN_PARAM, config.oauth_authorize_temp_token, user_enc, password_enc);

	free(user_enc);
	free(password_enc);

	if((res = http_get_oauth(oauth, login_uri, &reply))) {
		rc = oauth_split_url_parameters(reply, &rv);
		qsort(rv, rc, sizeof(char *), oauth_cmpstringp);
		if(rc == 2 && !strncmp(rv[1], "oauth_verifier=", 14)) {
			oauth->verifier = malloc(KEY_LENGTH + 1);
			strcpy(oauth->verifier, rv[1]+15);

			free(rv);
			free(reply);

			return 1;
		}
		else {
			oauth->error = reply;
			res = 0;
		}
	}
	free(login_uri);
	if(reply)
		free(reply);
	return res;
}
int gog_token(struct oauth_t *oauth) {
	char *reply = NULL, **rv = NULL, *token_uri = NULL;
	int res, rc;

	token_uri = malloc(strlen(TOKEN_PARAM) - 4 + strlen(config.oauth_get_token) + KEY_LENGTH);
	sprintf(token_uri, TOKEN_PARAM, config.oauth_get_token, oauth->verifier);

	if((res = http_get_oauth(oauth, token_uri, &reply))) {
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
		else {
			oauth->error = reply;
			res = 0;
		}
	}
	free(token_uri);
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
	char *reply = NULL, *game_details_uri = NULL;
	int res;
	struct json_object *answer, *obj;
	struct array_list *list;

	game_details_uri = malloc(strlen(config.get_game_details) + strlen(game) + 2);
	sprintf(game_details_uri, "%s%s/", config.get_game_details, game);

	if((res = http_get_oauth(oauth, game_details_uri, &reply))) {
		struct message_t *msg = setup_handler(oauth, reply);

		msg->content.game = (struct game_details_t *)malloc(sizeof(struct game_details_t));

		answer = json_tokener_parse(reply);
		obj = json_object_object_get(answer, "game");

		msg->content.game->icon = strdup(json_object_get_string(json_object_object_get(obj, "icon")));

		list = json_object_get_array(json_object_object_get(obj, "extras"));
		extract_files(list, msg->content.game->extras);

		list = json_object_get_array(json_object_object_get(obj, "installers"));
		extract_files(list, msg->content.game->installers);

		array_list_free(list);

		json_object_put(obj);
		json_object_put(answer);
	}

	free(game_details_uri);
	if(reply)
		free(reply);

	return 0;
}
int gog_user_details(struct oauth_t *oauth) {
	char *req_url = NULL, *reply = NULL;
	int res;

	if((res = http_get_oauth(oauth, config.get_user_details, &reply))) {
		puts(reply);
	}

	if(reply)
		free(reply);

	return res;
}
int gog_extra_link(struct oauth_t *oauth, const char *game, const short file_id) {
	char *reply = NULL, *extra_link_uri = NULL;
	int res;

	extra_link_uri = malloc(strlen(config.get_extra_link) + strlen(game) + 7);
	sprintf(extra_link_uri, "%s%s/%d/", config.get_extra_link , game, file_id);

	if((res = http_get_oauth(oauth, extra_link_uri, &reply))) {
		puts(reply);
	}

	free(extra_link_uri);
	if(reply)
		free(reply);

	return res;
}
int gog_installer_link(struct oauth_t *oauth, const char *game, const short file_id) {
	char *reply = NULL, *installer_link_uri = NULL;
	int res;

	installer_link_uri = malloc(strlen(config.get_installer_link) + strlen(game) + 7);
	sprintf(installer_link_uri, "%s%s/%d/", config.get_installer_link , game, file_id);

	if((res = http_get_oauth(oauth, installer_link_uri, &reply))) {
		puts(reply);
	}

	free(installer_link_uri);
	if(reply)
		free(reply);

	return res;
}
int gog_user_games(struct oauth_t *oauth) {
	char *reply = NULL;
	int res;

	if((res = http_get_oauth(oauth, config.get_user_games, &reply))) {
		puts(reply);
	}

	if(reply)
		free(reply);

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
	char *reply = NULL, *file_crc_uri = NULL;
	int res;

	file_crc_uri = malloc(strlen(config.get_installer_link) + strlen(game) + 8);
	sprintf(file_crc_uri, "%s%s/%d/crc/", config.get_installer_link , game, file_id);

	if((res = http_get_oauth(oauth, file_crc_uri, &reply))) {
		puts(reply);
	}

	free(file_crc_uri);
	if(reply)
		free(reply);

	return res;
}
