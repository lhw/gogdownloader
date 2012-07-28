#include "gog.h"
#include "generated/state.pb-c.h"

int serialize_download(struct download_t *dl, void **out) {
	DState dstate = DSTATE__INIT;
	DState__DFile dfile = DSTATE__DFILE__INIT;
	DState__DActive **dactives;

	unsigned len;

	/* copy all the simple types */
	dstate.available = dl->available;	
	dstate.link = dl->link;
	dstate.msg = dl->message;
	dstate.name = dl->name;
	dstate.type = dl->type;
	dstate.real_size = dl->real_size;

	/* copy file information */	
	memcpy(&(dfile.id), dl->file, sizeof(struct file_t));

	/* add the reference */
	dstate.reference = &dfile;

	dactives = malloc(sizeof(DState__DActive *) * dl->active_count);
	for(int i = 0; i < dl->active_count; i++) {
		dactives[i] = malloc(sizeof(DState__DActive));
		dstate__dactive__init(dactives[i]);
		memcpy(&(dactives[i]->from), &(dl->active[i]), sizeof(off_t) * 4);
	}
	dstate.actives = dactives;
	dstate.n_actives = dl->active_count;

	len = dstate__get_packed_size(&dstate);
	*out = malloc(len);
	dstate__pack(&dstate, *out);

	for(int i = 0; i < dl->active_count; i++)
		free(dactives[i]);
	free(dactives);

	return len;
}
int serialize_to_file(struct download_t *dl, char *file) {
	void *buf = NULL;
	FILE *state_file;
	unsigned len;

	if((len = serialize_download(dl, &buf))) {
		state_file = fopen(file, "w+");
		if(len == fwrite(buf, len, 1, state_file)) {
			fclose(state_file);
			return 1;
		}
		fclose(state_file);
	}
	return 0;
}
int deserialize_download(char **data, struct download_t **out) {

	return 0;
}
int deserialize_file(char *file, struct download_t **out) {

	return 0;
}
