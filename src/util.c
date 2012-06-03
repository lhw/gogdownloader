#include "gog.h"
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
	struct json_object *answer, *file;

	answer = json_tokener_parse(reply);
	file = json_object_object_get(answer, "file");

	out->available = json_object_get_int(json_object_object_get(file, "available"));
	if(out->available) {
		out->link = strdup(json_object_get_string(json_object_object_get(file, "link")));
		out->message = strdup(json_object_get_string(json_object_object_get(file, "message")));
		if(strlen(out->message) < 2)
			free(out->message);

		out->name = strdup(json_object_get_string(json_object_object_get(file, "name")));
		out->type = strdup(json_object_get_string(json_object_object_get(file, "type")));

		json_object_put(file);
		json_object_put(answer);
		
		return 1;
	}
	json_object_put(file);
	json_object_put(answer);
	return 0;
}
int free_message(struct message_t *msg) {
	if(!msg)
		return 1;
	switch(msg->type) {
		case GAME:
			for(int i = 0; i < msg->game.extras_count; i++) {
				if(msg->game.extras[i].name)
					free(msg->game.extras[i].name);
				free(msg->game.extras[i].path);
			}
			free(msg->game.extras);
			for(int i = 0; i < msg->game.installers_count; i++)
				free(msg->game.installers[i].path);
			free(msg->game.installers);

			free(msg->game.icon);

			free(msg);

			return 1;
		case USER:
			free(msg->user.avatar);
			free(msg->user.email);
			free(msg->user.nick);
			free(msg);

			return 1;;
		case DOWNLOAD:
			if(msg->download.available) {
				free(msg->download.link);
				if(msg->download.message)
					free(msg->download.message);
				if(msg->download.name)
					free(msg->download.name);
				if(msg->download.type)
					free(msg->download.type);
			}
			free(msg);

			return 1;
		default:
			return 1;
	}

	return 0;
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
