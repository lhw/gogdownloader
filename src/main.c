#include "gog.h"
#include "token.h"

int main() {
	struct oauth_t *oauth;

	curl_global_init(CURL_GLOBAL_SSL);
	oauth = malloc(sizeof(struct oauth_t));
	oauth->token = oauth->secret = oauth->error = oauth->verifier = NULL;

	gog_download_config(oauth, DEFAULT_RELEASE);

	/*if(gog_login(oauth, USERNAME, PASSWORD))
		printf("Token: %s\nSecret: %s\n", oauth->token, oauth->secret);*/
	oauth->token = TOKEN; 
	oauth->secret = SECRET;

	gog_user_details(oauth);
	free_message(oauth->msg);
	/*
	gog_game_details(oauth, "beneath_a_steel_sky");
	gog_user_games(oauth);
	gog_installer_link(oauth, "beneath_a_steel_sky", 0);
	gog_installer_crc(oauth, "beneath_a_steel_sky", 0);
	gog_extra_link(oauth, "tyrian_2000", 968); //WORKING
	if(!gog_extra_link(oauth, "tyrian_2000", 967)) { //NOT EXISTANT
		puts(oauth->error);
	}
	*/
	//free(oauth->token);
	//free(oauth->secret);
	free(oauth);
}
