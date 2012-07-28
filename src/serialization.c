#include "gog.h"
#include "state.pb-c.h"
#include  <google/protobuf-c/protobuf-c.h>

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

	/* copy active sessions */
	dactives = malloc(sizeof(DState__DActive *) * dl->active_count);
	for(int i = 0; i < dl->active_count; i++) {
		dactives[i] = malloc(sizeof(DState__DActive));
		dstate__dactive__init(dactives[i]);
		memcpy(&(dactives[i]->from), &(dl->active[i]), sizeof(off_t) * 4);
	}
	dstate.actives = dactives;
	dstate.n_actives = dl->active_count;

	/* pack the data */
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
int deserialize_download(void **data, struct download_t **out) {
	struct download_t *dl;
	struct active_t *actives;
	DState *dstate;
	int len;

	dl = *out;
	dl = malloc(sizeof(struct download_t));
	len = -1;

	dstate = dstate__unpack(NULL, len, *data);
	dl->available = dstate->available;	
	dl->link = dstate->link;
	dl->message = dstate->msg;
	dl->name = dstate->name;
	dl->type = dstate->type;
	dl->real_size = dstate->real_size;

	/* copy file information */	
	memcpy(&(dstate->reference), &(dl->file->id), sizeof(struct file_t));

	/* copy actives information */
	actives = malloc(sizeof(struct active_t) * dstate->n_actives);
	for(size_t i = 0; i < dstate->n_actives; i++)
		memcpy(&(actives[i]), &(dstate->actives) + sizeof(ProtobufCMessage), sizeof(off_t) * 4);
	dl->active = actives;
	dl->active_count = dstate->n_actives;

	dstate__free_unpacked(dstate, NULL);

	return 1;
}
int deserialize_file(char *file, struct download_t **out) {
	void *buf = NULL;
	FILE *state_file;
	size_t len;

	state_file = fopen(file, "r");
	if(state_file) {
		fseek(state_file, 0, SEEK_END);
		len = ftell(state_file);

		buf = malloc(len);
		if(fread(buf, len, 1, state_file) == len) {
			deserialize_download(buf, out);
			fclose(state_file);
			free(buf);
			return 1;
		}
		free(buf);
		fclose(state_file);
	}

	return 0;
}
