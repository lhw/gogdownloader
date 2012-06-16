#include "gog.h"
#include "token.h"

int main() {
	struct oauth_t *oauth;
	struct game_details_t *game;
	struct download_t *download;

	curl_global_init(CURL_GLOBAL_SSL);
	oauth = malloc(sizeof(struct oauth_t));
	download = malloc(sizeof(struct download_t));
	oauth->token = oauth->secret = oauth->error = oauth->verifier = NULL;

	gog_download_config(oauth, DEFAULT_RELEASE);

#if 0
	if(gog_login(oauth, USERNAME, PASSWORD))
		printf("Token: %s\nSecret: %s\n", oauth->token, oauth->secret);
#endif
	oauth->token = TOKEN;
	oauth->secret = SECRET;

	if(gog_game_details(oauth, "beneath_a_steel_sky")) {
		game = oauth->msg->game;
		if(gog_installer_link(oauth, "beneath_a_steel_sky", 0)) {
			download = oauth->msg->download;
			download->file = &(game->installers[0]);

			create_partial_download(download, 2);

			free_download(download);
		}
		free_game(game);
	}


	free_message(oauth->msg);
	free(oauth);
}
