#include <string.h>
#include <sys/stat.h>
#include <errno.h>

#include "gog.h"
#include "config.pb-c.h"

int load_config() {
	FILE *cfg_file;
	Config *cfg;
	off_t len;
	void *buf;

	create_config_dirs();

	if(config.config_file == NULL)
		config.config_file = DEFAULT_CONFIG_FILE;

	if((cfg_file = fopen(config.config_file, "r"))) {
		fseek(cfg_file, 0, SEEK_END);
		len = ftell(cfg_file);
		rewind(cfg_file);
		buf = malloc(len);

		fread(buf, 1, len, cfg_file);
		cfg = config__unpack(NULL, len, buf);

		fclose(cfg_file);
		free(buf);
	
		if(cfg->token)
			config.token = strdup(cfg->token);
		if(cfg->secret)
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

	create_config_dirs();

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
void create_config_dirs() {
	char *xdg_config_dir, *config_file, *config_dir;
	struct stat st;

	xdg_config_dir = XDG_CONFIG_HOME;
	config_file = DEFAULT_CONFIG_FILE;
	config_dir = DEFAULT_CONFIG_DIR;

	if(stat(xdg_config_dir, &st) == -1) {
		if(errno & ENOENT)
			mkdir(xdg_config_dir, 0755);
	}
	
	if(stat(config_dir, &st) == -1) {
		if(errno & ENOENT)
			mkdir(config_dir, 0700);
	}
}
char *xdg_config_home() {
	char *home_dir, *xdg_config_dir;

	home_dir = getenv("HOME");

	if(home_dir == NULL)
		return strdup ("/tmp");

	xdg_config_dir = getenv("XDG_CONFIG_HOME");
	if(xdg_config_dir == NULL || xdg_config_dir[0] == 0) {
		xdg_config_dir = malloc(strlen(home_dir) + strlen("/.config/") + 1);
		strcpy(xdg_config_dir, home_dir);
		strcat(xdg_config_dir, strdup("/.config/"));
	}
	return xdg_config_dir;
}
char *config_dir_path() {
	char *config_dir, *xdg_config_dir;

	xdg_config_dir = XDG_CONFIG_HOME;
	config_dir = malloc(strlen(xdg_config_dir) + strlen("gogdownloaderi/") + 1);
	strcpy(config_dir, xdg_config_dir);
	strcat(config_dir, strdup("gogdownloader/"));

	return config_dir;

}
char *config_file_path() {
	char *config_file, *config_dir;

	config_dir = DEFAULT_CONFIG_DIR;

	config_file = malloc(strlen(config_dir) + strlen("config.pbf") + 1);
	strcpy(config_file, config_dir);
	strcat(config_file, strdup("config.pbf"));
	free(config_dir);

	return config_file;
}
