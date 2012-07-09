#include "gog.h"
struct message_t *setup_handler(struct oauth_t *oauth, char *reply) {
	struct json_object *answer;

	/*if(oauth->msg != NULL)
		free_message(oauth->msg);*/
	oauth->msg = malloc(sizeof(struct message_t));

	answer = json_tokener_parse(reply);
	oauth->msg->result = !strcmp(json_object_get_string(json_object_object_get(answer, "result")), "ok");
	oauth->msg->timestamp = json_object_get_int(json_object_object_get(answer, "timestamp"));

	json_object_put(answer);

	return oauth->msg;
}
int extract_files(struct array_list *list, struct file_t **out) {
	struct json_object *item, *name;
	int len;
	char *size;

	if((len = array_list_length(list)) > 0) {
		*out = calloc(sizeof(struct file_t), len);
		for(int i = 0; i < len; i++) {
			item = (struct json_object *)array_list_get_idx(list, i);
			(*out)[i].id = json_object_get_int(json_object_object_get(item, "id"));
			(*out)[i].path = strdup(json_object_get_string(json_object_object_get(item, "path")));

			name = json_object_object_get(item, "name");
			if(name != NULL)
				(*out)[i].name = strdup(json_object_get_string(name));
			json_object_put(name);

			size = strdup(json_object_get_string(json_object_object_get(item, "size_mb")));
			char *comma = strchr(size, ',');
			if(comma != NULL)
				*comma = '.';
			(*out)[i].size = strtof(size, NULL);
			free(size);

			json_object_put(item);
		}
		return len;
	}
	return 0;
}
int extract_download(const char *reply, struct download_t *out) {
	struct json_object *answer, *file, *tmp;

	answer = json_tokener_parse(reply);
	file = json_object_object_get(answer, "file");

	out->available = json_object_get_int(json_object_object_get(file, "available"));
	if(out->available) {
		out->link = strdup(json_object_get_string(json_object_object_get(file, "link")));
		out->message = strdup(json_object_get_string(json_object_object_get(file, "message")));
		if(strlen(out->message) < 2) {
			free(out->message);
			out->message = NULL;
		}

		if((tmp = json_object_object_get(file, "name")) != NULL)
			out->name = strdup(json_object_get_string(tmp));
		else
			out->name = NULL;

		if((tmp = json_object_object_get(file, "type")) != NULL)
			out->type = strdup(json_object_get_string(tmp));
		else
			out->type = NULL;

		json_object_put(file);
		json_object_put(answer);
		
		return 1;
	}
	json_object_put(file);
	json_object_put(answer);
	return 0;
}
int receive_download_links(struct oauth_t *oauth, char *url) {
	char *reply;

	if(http_get_json(oauth, url, &reply)) {
		struct message_t *msg = setup_handler(oauth, reply);

		if(msg->result) {
			msg->type = DOWNLOAD;
			msg->download = malloc(sizeof(struct download_t));
			if(extract_download(reply, msg->download)) {
				free(reply);
				free(url);
				return 1;

			}
		}
	}
	if(reply)
		free(reply);
	free(url);
	return 0;
}
void free_message(struct message_t *msg) {
	if(!msg)
		return;
	switch(msg->type) {
		case GAME:
			free_game(msg->game);
			break;
		case USER:
			free_user(msg->user);
			break;
		case DOWNLOAD:
			free_download(msg->download);
			break;
	}
	free(msg);
}
void free_game(struct game_details_t *game) {
	for(int i = 0; i < game->extras_count; i++) {
		if(game->extras[i].name)
			free(game->extras[i].name);
		free(game->extras[i].path);
	}
	free(game->extras);
	for(int i = 0; i < game->installers_count; i++)
		free(game->installers[i].path);
	free(game->installers);
	free(game->icon);
}
void free_user(struct user_details_t *user) {
	free(user->avatar);
	free(user->email);
	free(user->nick);
}
void free_download(struct download_t *download) {
	if(download->available) {
		free(download->link);
		if(download->message)
			free(download->message);
		if(download->name)
			free(download->name);
		if(download->type)
			free(download->type);
	}
	for(int i = 0; i < download->active_count; i++)
		free_active(&(download->active[i]));

	curl_multi_cleanup(download->multi);
	free(download);
}
void free_active(struct active_t *active) {
	if(active->file)
		fclose(active->file);
	curl_easy_cleanup(active->curl);
}
int file_exists(char *path) {
	FILE *fp = fopen(path, "r");
	if(!fp) {
		fclose(fp);
		return 0;
	}
	else {
		fclose(fp);
		return 1;
	}
}
void print_error(struct oauth_t *oauth) {
	if(oauth->error && strlen(oauth->error) > 1)
		printf("ERROR: %s\n", oauth->error);
}
