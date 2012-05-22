#ifndef __GOG__
#define __GOG__

#include<string.h>
#include<stdio.h>
#include<stdlib.h>
#include<oauth.h>
#include<curl/curl.h>
#include<json/json.h>

#define CONSUMER_KEY "1f444d14ea8ec776585524a33f6ecc1c413ed4a5"
#define CONSUMER_SECRET "20d175147f9db9a10fc0584aa128090217b9cf88"

#define KEY_LENGTH 40
#define LOGIN_PARAM "%s?username=%s&password=%s"
#define TOKEN_PARAM "%s?oauth_verifier=%s"
#define CONFIG_URL "https://api.gog.com/en/downloader2/status/%s/"
#define DEFAULT_RELEASE "stable"

struct config_t {
	char *oauth_authorize_temp_token;
	char *oauth_get_temp_token;
	char *oauth_get_token;
	char *get_user_games;
	char *get_user_details;
	char *get_installer_link;
	char *get_game_details;
	char *get_extra_link;
	char *set_app_status;
} config;

struct oauth_t {
	char *token;
	char *secret;
	char *error;

	/* only used during login process */
	char *verifier;
};

CURL *curl;

size_t static write_callback(void *buffer, size_t size, size_t nmemb, void *userp);
int http_get(const char *url, char **buffer, char **error_msg);

int gog_download_config(struct oauth_t *oauth, const char *release);

int gog_request_token(struct oauth_t *oauth);
int gog_access_token(struct oauth_t *oauth, const char *email, const char *password);
int gog_token(struct oauth_t *oauth);
int gog_login(struct oauth_t *oauth, const char *email, const char *password);

int gog_user_games(struct oauth_t *oauth);
int gog_game_details(struct oauth_t *oauth, const char *game);
int gog_user_details(struct oauth_t *oauth);
int gog_installer_link(struct oauth_t *oauth, const char *game, const short file_id);
int gog_installer_crc(struct oauth_t *oauth, const char *game, const short file_id);
int gog_extra_link(struct oauth_t *oauth, const char *game, const short file_id);

#endif
