#include "gog.h"
#include "token.h"

int main() {
	struct oauth_t *oauth;

	curl_global_init(CURL_GLOBAL_SSL);
	oauth = malloc(sizeof(struct oauth_t));
	oauth->token = oauth->secret = oauth->error = oauth->verifier = NULL;

	gog_download_config(oauth, DEFAULT_RELEASE);

	if(gog_login(oauth, USERNAME, PASSWORD))
		printf("Token: %s\nSecret: %s\n", oauth->token, oauth->secret);

	if(gog_installer_link(oauth, "beneath_a_steel_sky", 0)) {
		
	}


	free_message(oauth->msg);
	free(oauth);
}
