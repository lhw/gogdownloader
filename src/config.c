#include "gog.h"
#include "config.pb-c.h"

int load_config() {
	FILE *cfg_file;
	Config *cfg;
	off_t len;
	void *buf;

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
