#include "gog.h"

int main() {
	struct oauth_t *oauth;
	struct game_details_t *game;
	struct download_t *download;
	int r = 0;

	curl_global_init(CURL_GLOBAL_SSL);
	oauth = calloc(sizeof(struct oauth_t), 1);
	download = malloc(sizeof(struct download_t));

	if(!load_config()) {
		if(config.download_path == NULL) {
			config.download_path = XDG_DOWNLOAD_DIR;
			save_config();
		}
	}

	if(!gog_download_config(oauth, DEFAULT_RELEASE)) {
		print_error(oauth);
		return 1;
	}

	while(config.token == NULL || config.secret == NULL) {
		char email[255], password[255];
		char c;
		int i;

		printf("E-Mail: ");
		get_string(email, sizeof(email));

		printf("Password: ");
		get_password(password, sizeof(password));

		if(gog_login(oauth, email, password)) {
			config.token = oauth->token;
			config.secret = oauth->secret;
			save_config();
		}
		else
			print_error(oauth);
	}

	char title[512];

	printf("Download: ");
	get_string(title, sizeof(title));

	if(gog_game_details(oauth, title)) {
		game = oauth->msg->game;
		if(gog_installer_link(oauth, title, 0)) {
			download = oauth->msg->download;
			download->file = &(game->installers[0]);

			create_partial_download(download, 2);

			do {
				curl_multi_perform(download->multi, &r);
			} while(r);
		}
		else
			print_error(oauth);
	}
	else
		print_error(oauth);
}
