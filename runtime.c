#include <stdio.h>
#include <stdint.h>
#include <malloc.h>

#include "selfish.h"

object * call_primitive_method(object * value, object * self) {
    return value->primitive_method(value, self);
}

object * return_thyself(object * value, object * self) {
    return value;
}

object * lookup1(int code, object * o) {
    for (int i=0; i < o->assoc_size; i++) {
        if (o->assocs[i].number == code || o->assocs[i].number == -code) {
            return o->assocs[i].value;
        }
    }
    for (int i=0; i < o->assoc_size; i++) {
        if (o->assocs[i].number < 0) {
            object * result = lookup1(code, o->assocs[i].value); // todo mark as 'been there'
            if (result != NULL) return result;
        }
    }
    return NULL;
}

object * lookup(int code, object * method, object * self) {
    object * result = lookup1(code, method);
    if (result == NULL) return lookup1(code, self);
    return result;
}

type_invocation_method callbacks[];
object * run_method(object * method, object * self) {
    // TODO take explicit args (define struct method instead of just code)
    // and make instance (this may also simplifiy lookup)

    object * o = NULL;
    for (int i=0; i< method->value_size; i++) {
        int code = method->code[i];
        if (code > 0) {
            if (o != NULL) push (o);
            o = lookup(code, method, self);
            if (o == NULL) {
                printf("Could not find object %s\n", to_name(code));
                return NULL;
            }
        } else if (code < 0) {
            if (o == NULL) {
                printf("Nowhere to lookup method %s\n", to_name(-code));
                return NULL;
            }
            object * m = lookup1(-code, o);
            if (m == NULL) {
                printf("Could not find method %s\n", to_name(-code));
                return NULL;
            }
            o = callbacks[m->type](m, o);
        } else { // code == 0
            clear_stack();
            o = NULL;
        }
    }

    return o;
}

type_invocation_method callbacks[] = {
    return_thyself,
    return_thyself,
    run_method,
    call_primitive_method
};

void append_code(object * o, int c) {
	o->code = realloc(o->code, sizeof(int) * (o->value_size+1));
	o->code[o->value_size] = c;
	o->value_size++;
}

void prepend_code(object * o, int c) {
	o->code = realloc(o->code, sizeof(int) * (o->value_size+1));

	for (int i=o->value_size; i>0; i--) {
		o->code[i] = o->code[i-1];
	}
	o->code[0] = c;
	o->value_size++;
}

void append_assoc(object * o, int number, object * value) {
	o->assocs = realloc(o->assocs, sizeof(assoc) * (o->assoc_size+1));
	o->assocs[o->assoc_size].number = number;
	o->assocs[o->assoc_size].value = value;
	o->assoc_size++;
}

