#include "gog.h"
#include "generated/state.pb-c.h"

int serialize_download(struct download_t *dl, char **out) {
	DState dstate = DSTATE__INIT;
	DState__DFile dfile = DSTATE__DFILE__INIT;
	DState__DActive **dactives;

	void *buf = NULL;
	unsigned len;

	/* copy all the simple types */
	dstate.available = dl->available;	
	dstate.link = dl->link;
	dstate.msg = dl->message;
	dstate.name = dl->name;
	dstate.type = dl->type;
	dstate.real_size = dl->real_size;

	/* copy file information */	
	dfile.id = dl->file->id;
	dfile.name = dl->file->name;
	dfile.path = dl->file->path;
	dfile.size = dl->file->size;

	/* add the reference */
	dstate.reference = &dfile;

	dactives = malloc(sizeof(DState__DActive *) * dl->active_count);
	for(int i = 0; i < dl->active_count; i++) {
		dactives[i] = malloc(sizeof(DState__DActive));
		dstate__dactive__init(dactives[i]);
		dactives[i]->from = dl->active[i].from;
		dactives[i]->to = dl->active[i].to;
		dactives[i]->current = dl->active[i].current;
		dactives[i]->chunk_size= dl->active[i].chunk_size;
	}
	dstate.actives = dactives;
	dstate.n_actives = dl->active_count;

	len = dstate__get_packed_size(&dstate);
	buf = malloc(len);
	dstate__pack(&dstate, buf);

	for(int i = 0; i < dl->active_count; i++)
		free(dactives[i]);
	free(dactives);

	return len;
}
int serialize_to_file(struct download_t *dl, char *file) {
	char *buf = NULL;
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
