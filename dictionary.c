#include <string.h>
#include <malloc.h>

char ** names = NULL;
int dict_size = 0;

int to_number(char * name) {
	if (strcmp("", name) == 0) {
		dict_size++;
		names = realloc(names, sizeof(char *) * dict_size);
		names[dict_size-1] = name; // empty string for nameless objects
		return dict_size; // one off because we don't allow a 0 name
	}

	for (int i=0; i<dict_size;i++) {
		if (strcmp(names[i], name) == 0) return i+1;
	}

	// not found; create
	dict_size++;
	names = realloc(names, sizeof(char *) * dict_size);
	names[dict_size-1] = name;
	return dict_size; // one off because we don't allow a 0 name
}

char * to_name(int i) {
	return names[i-1];
}
