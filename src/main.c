#include "gog.h"
#include "token.h"

int main() {
	struct oauth_t *oauth;
	struct game_details_t *game;
	struct download_t *download;
	int r = 0;

	curl_global_init(CURL_GLOBAL_SSL);
	oauth = calloc(sizeof(struct oauth_t), 1);
	download = malloc(sizeof(struct download_t));

	if(!gog_download_config(oauth, DEFAULT_RELEASE)) {
		print_error(oauth);
		return 1;
	}

	if(gog_login(oauth, USERNAME, PASSWORD))
		printf("Token: %s\nSecret: %s\n", oauth->token, oauth->secret);
	else
		print_error(oauth);
#if 0
	oauth->token = TOKEN;
	oauth->secret = SECRET;
#endif

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
