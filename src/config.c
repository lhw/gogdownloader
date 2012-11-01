#include "gog.h"
#include "config.pb-c.h"

int load_config() {
	FILE *cfg_file;
	Config *cfg;
	off_t len;
	void *buf;

	if(config.config_file == NULL)
		config.config_file = DEFAULT_CONFIG_FILE;

	if((cfg_file = fopen(config.config_file, "r"))) {
		fseek(cfg_file, 0, SEEK_END);
		len = ftell(cfg_file);
		buf = malloc(len);

		fread(buf, len, 1, cfg_file);
		cfg = config__unpack(NULL, len, buf);

		fclose(cfg_file);
		free(buf);
	
		config.token = strdup(cfg->token);
		config.secret = strdup(cfg->secret);
		config.download_path = strdup(cfg->download_path);

		config__free_unpacked(cfg, NULL);

		return 1;
	}
	return 0;
}
int save_config() {
	FILE *cfg_file;
	Config cfg = CONFIG__INIT;
	void *buf;
	off_t len;

	if(config.config_file == NULL)
		config.config_file = DEFAULT_CONFIG_FILE;

	if((cfg_file = fopen(config.config_file, "w+"))) {
		cfg.token = config.token;
		cfg.secret = config.secret;
		cfg.download_path = config.download_path;

		len = config__get_packed_size(&cfg);
		buf = malloc(len);
		config__pack(&cfg, buf);

		fwrite(buf, len, 1, cfg_file);

		fclose(cfg_file);
		free(buf);

		return 1;
	}
	return 0;
}
static char *config_file_path() {
	char *config_file, *home_dir, *xdg_config_dir;

	home_dir = getenv("HOME");

	if(home_dir == NULL)
		return strdup ("/tmp");

	xdg_config_dir = getenv("XDG_CONFIG_HOME");
	if(xdg_config_dir == NULL || xdg_config_dir[0] == 0) {
		xdg_config_dir = malloc(strlen(home_dir) + strlen("/.config") + 1);
		strcpy(xdg_config_dir, home_dir);
		strcat(xdg_config_dir, "/.config");
	}

	config_file = malloc(strlen(xdg_config_dir) + strlen("gogdownloader/config.pbf") + 1);
	strcpy(config_file, xdg_config_dir);
	strcat(config_file, strdup("gogdownloader/config.pbf"));
	free(xdg_config_dir);

	return config_file;
}
static char *xdg_user_dir_lookup(const char *type) {
	FILE *file;
	char *home_dir, *config_home, *config_file;
	char buffer[512];
	char *user_dir;
	char *p, *d;
	int len;
	int relative;

	home_dir = getenv("HOME");

	if(home_dir == NULL)
		return strdup("/tmp");

	config_home = getenv("XDG_CONFIG_HOME");
	if(config_home == NULL || config_home[0] == 0) {
		config_file = malloc(strlen(home_dir) + strlen("/.config/user-dirs.dirs") + 1);
		strcpy(config_file, home_dir);
		strcat(config_file, "/.config/user-dirs.dirs");
	}
	else {
		config_file = malloc(strlen(config_home) + strlen("/user-dirs.dirs") + 1);
		strcpy(config_file, config_home);
		strcat(config_file, "/user-dirs.dirs");
	}

	file = fopen(config_file, "r");
	free(config_file);
	if(file == NULL)
		goto error;

	user_dir = NULL;
	while(fgets(buffer, sizeof(buffer), file)) {
		/* Remove newline at end */
		len = strlen(buffer);
		if(len > 0 && buffer[len-1] == '\n')
			buffer[len-1] = 0;

			p = buffer;
			while(*p == ' ' || *p == '\t')
				p++;

			if(strncmp(p, "XDG_", 4) != 0)
				continue;
			p += 4;
			if(strncmp(p, type, strlen(type)) != 0)
				continue;
			p += strlen(type);
			if(strncmp(p, "_DIR", 4) != 0)
				continue;
			p += 4;

			while(*p == ' ' || *p == '\t')
				p++;

			if(*p != '=')
				continue;
			p++;

			while(*p == ' ' || *p == '\t')
				p++;

			if(*p != '"')
				continue;
			p++;

			relative = 0;
			if(strncmp(p, "$HOME/", 6) == 0) {
				p += 6;
				relative = 1;
			}
			else if(*p != '/')
				continue;

			if(relative) {
				user_dir = malloc(strlen(home_dir) + 1 + strlen(p) + 1);
				strcpy(user_dir, home_dir);
				strcat(user_dir, "/");
			}
			else {
				user_dir = malloc(strlen(p) + 1);
				*user_dir = 0;
			}

			d = user_dir + strlen(user_dir);
			while(*p && *p != '"') {
				if((*p == '\\') &&(*(p+1) != 0))
					p++;
				*d++ = *p++;
			}
			*d = 0;
		}
	fclose(file);

	if(user_dir)
		return user_dir;

 error:
	/* Special case desktop for historical compatibility */
	if(strcmp(type, "DESKTOP") == 0) {
		user_dir = malloc(strlen(home_dir) + strlen("/Desktop") + 1);
		strcpy(user_dir, home_dir);
		strcat(user_dir, "/Desktop");
		return user_dir;
	}
	else
		return strdup(home_dir);
}
