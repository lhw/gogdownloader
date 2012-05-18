#ifndef __GOG__
#define __GOG__

#include<string.h>
#include<stdio.h>
#include<stdlib.h>
#include<oauth.h>
#include<curl/curl.h>

#define CONSUMER_KEY "1f444d14ea8ec776585524a33f6ecc1c413ed4a5"
#define CONSUMER_SECRET "20d175147f9db9a10fc0584aa128090217b9cf88"

#define KEY_LENGTH 40
#define LOGIN_PARAM "%s?username=%s&password=%s\0"
#define TOKEN_PARAM "%s?oauth_verifier=%s\0"

struct config_t {
	char *oauth_authorize_temp_token;
	char *oauth_get_temp_token;
	char *oauth_get_token;
	char *get_user_games;
	char *get_user_details;
	char *get_installer_link;
	char *get_game_details;
	char *get_extra_link;
} config;

CURL *curl;

size_t static write_callback(void *buffer, size_t size, size_t nmemb, void *userp);
char *oauth_http_get3(const char *url);
int gog_request_token(char **token, char **secret);
int gog_access_token(const char *email, const char *password, const char *key, const char *secret, char **verifier);
int gog_token(const char *auth_token, const char *auth_secret, const char *verifier, char **token, char **secret);
int gog_login(const char *email, const char *password, char **token, char **secret);

#endif
