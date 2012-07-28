#ifndef __GOG__
#define __GOG__

#include<string.h>
#include<stdio.h>
#include<stdlib.h>
#include<oauth.h>
#include<errno.h>
#include<sys/stat.h>
#include<sys/types.h>

#include<curl/curl.h>
#include<json/json.h>

#define CONSUMER_KEY "1f444d14ea8ec776585524a33f6ecc1c413ed4a5"
#define CONSUMER_SECRET "20d175147f9db9a10fc0584aa128090217b9cf88"

#define KEY_LENGTH 40
#define LOGIN_PARAM "%s?username=%s&password=%s"
#define TOKEN_PARAM "%s?oauth_verifier=%s"
#define CONFIG_URL "https://api.gog.com/en/downloader2/status/%s/"
#define CONFIG_ENTRY(a) (strdup(json_object_get_string(json_object_object_get(config_node, a))))
#define DEFAULT_RELEASE "stable"

/** parsed configuration from the gog server */
struct config_t {
	/** used in gog_request_token */ 
	char *oauth_authorize_temp_token;
	/** used in gog_access_token */
	char *oauth_get_temp_token;
	/** used in gog_token */
	char *oauth_get_token;
	/** not implemented server-side */
	char *get_user_games;
	/** avatar, emails and such */
	char *get_user_details;
	/** the file links for the actual game */
	char *get_installer_link;
	/** list of all files and extras */
	char *get_game_details;
	/** the file links for the extras */
	char *get_extra_link;
	/** no idea what this is. not implemented */
	char *set_app_status;
} config;

/** struct containing download information */
struct download_t {
	/** if the file is actually available on the servers */
	int available;
	/** the url to the file on the CDN */
	char *link;
	/** uncertain. hasn't been used yet in any download */
	char *message;
	/** used for extras. contain short description */
	char *name;
	/** categories for extras */
	char *type;

	/** backreference */
	struct file_t *file;
	/** the real size of the file in bytes */
	off_t real_size;

	/** currently active connections */
	struct active_t *active;
	/** connection count */
	int active_count;
	/** the curl multi handle */
	CURLM *multi;
};

/** struct containt file information */
struct file_t {
	/** file id used for api calls */
	int id;
	/** real name of the package */
	char *name;
	/** path for the download */
	char *path;
	/** approximate double representing the file size */
	float size;
};

/** details of the game */
struct game_details_t {
	/** all extras for the game */
	struct file_t *extras;
	/** all game files */
	struct file_t *installers;
	/** extra count */
	short extras_count;
	/** files count */
	short installers_count;
	/** game title */
	char *title;
	/** game icon */
	char *icon;
};

/** details of the user */
struct user_details_t {
	/** big avatar if available small otherwise */
	char *avatar;
	/** user email */
	char *email;
	/** user id. long integer */
	long id;
	/** weird original field but contains username */
	char *nick;
};

/** active file download */
struct active_t {
	/** original start offset */
	off_t from;
	/** ending offset */
	off_t to;
	/** currently latest offset */
	off_t current;
	/** calculated chunk offset */
	off_t chunk_size;

	/** active file info */
	struct download_t *info;

	/** OS file handle */
	FILE *file;
	/** active curl handle */
	CURL *curl;
};

/** message types */
enum type_t {
	DOWNLOAD = 1,
	GAME,
	USER
};

/** parsed message from the gog api */
struct message_t {
	/** was the request successful */
	int result;
	/** answer timestamp */
	int timestamp;
	/** message type one of type_t */
	enum type_t type;

	/** message is one of these, check type */
	struct download_t *download;
	/** message is one of these, check type  */
	struct game_details_t *game;
	/** message is one of these, check type */
	struct user_details_t *user;
};

/** auth information */
struct oauth_t {
	/** token changes from request->auth->token */
	char *token;
	/** secret changes from request->auth->secret */
	char *secret;
	/** if a call fails this field is set */
	char *error;

	/** the transfered message parsed by the appropiate function */
	struct message_t *msg;

	/** only used during login process */
	char *verifier;
};

/* http.c */
size_t static write_callback(void *buffer, size_t size, size_t nmemb, void *userp);
size_t static file_write_callback(void *buffer, size_t size, size_t nmemb, void *userp);
int http_get(const char *url, char **buffer, char **error_msg);
int http_get_oauth(struct oauth_t *oauth, const char *url, char **buffer);
int http_get_json(struct oauth_t *oauth, const char *url, char **buffer);
off_t get_remote_file_size(char *url);
int create_download_handle(struct active_t *a);
int create_partial_download(struct download_t *dl, int n);

/* util.c */
struct message_t *setup_handler(struct oauth_t *oauth, char *reply);
int extract_files(struct array_list *list, struct file_t **out);
int extract_download(const char *reply, struct download_t *out);
int receive_download_links(struct oauth_t *oauth, char *url);
void free_message(struct message_t *msg);
void free_game(struct game_details_t *game);
void free_user(struct user_details_t *user);
void free_download(struct download_t *download);
void free_active(struct active_t *active);

int file_exists(char *path);
void print_error(struct oauth_t *oauth);

/* serialization.c */
int serialize_download(struct download_t *dl, void **out);
int serialize_to_file(struct download_t *dl, char *file);
int deserialize_download(char **data, struct download_t **out);
int deserialize_file(char *file, struct download_t **out);

/* api.c */
/** \brief Downloads the gog api configuration containing all urls
  * \ingroup api
  * \param oauth struct containing all OAuth information
  * \param release the api version
  * \return boolean
  */
int gog_download_config(struct oauth_t *oauth, const char *release);

/** \brief First part of the OAauth token exchange
  * \ingroup api
  * \param oauth struct containing all OAuth information
  * \return boolean
  */
int gog_request_token(struct oauth_t *oauth);
/** \brief Second part of the OAuth token exchange.
  *
  * Handles the authentication and returns OAauth verifier
  *
  * \ingroup api
  * \param oauth struct containing all OAuth information
  * \return boolean
  */
int gog_access_token(struct oauth_t *oauth, const char *email, const char *password);
/** \brief Returns the final token and secret 
  *
  * The return values can henceforth be used for logging in
  *
  * \ingroup api
  * \param oauth struct containing all OAuth information
  * \return boolean
  */
int gog_token(struct oauth_t *oauth);
/** \brief Calls all OAuth functions required for login
  * \ingroup api
  * \param oauth struct containing all OAuth information
  * \return boolean
  */
int gog_login(struct oauth_t *oauth, const char *email, const char *password);

int gog_user_games(struct oauth_t *oauth);
int gog_game_details(struct oauth_t *oauth, const char *game);
int gog_user_details(struct oauth_t *oauth);
int gog_installer_link(struct oauth_t *oauth, const char *game, const short file_id);
int gog_installer_crc(struct oauth_t *oauth, const char *game, const short file_id);
int gog_extra_link(struct oauth_t *oauth, const char *game, const short file_id);

#endif
