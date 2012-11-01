#include "gog.h"

#include <termios.h>
#include <unistd.h>

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

		struct termios tty;
		 tcgetattr(STDIN_FILENO, &tty);

		puts("E-Mail: ");
		while((c = getchar()) != '\n' && c != EOF && i < 255)
			email[i++] = c;

		puts("Password: ");
		tty.c_lflag &= ~ECHO;
		tcsetattr(STDIN_FILENO, TCSANOW, &tty);
		while((c = getchar()) != '\n' && c != EOF && i < 255)
			password[i++] = c;
		tty.c_lflag |= ECHO;
		tcsetattr(STDIN_FILENO, TCSANOW, &tty);

		if(gog_login(oauth, email, password)) {
			config.token = oauth->token;
			config.secret = oauth->secret;
		}
	}

	if(gog_game_details(oauth, "beneath_a_steel_sky")) {
		game = oauth->msg->game;
		if(gog_installer_link(oauth, "beneath_a_steel_sky", 0)) {
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
