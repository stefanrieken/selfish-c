#include "selfish.h"

object ** stack = NULL;
int stack_size = 0;

void push (object * o) {
    stack = realloc(stack, sizeof(object *) * (stack_size+1));
    stack[stack_size] = o;
    stack_size++;
}

object * pop() {
    if (stack_size == 0) return 0;
    --stack_size;
    object * o = stack[stack_size];
    stack = realloc(stack, sizeof(object *) * stack_size);
    return o;
}

void clear_stack() {
    stack_size = 0;
    stack = realloc(stack, sizeof(object *) * stack_size);
}

void print_stack() {
	for (int i=0; i<stack_size; i++) {
		if (stack[i]->type == STRING) {
			printf("[a string]");
		} else
		printf("[an object]"); // minimal viable product
	}
	printf("\n");
}
