#include <malloc.h>
#include "selfish.h"

// a hello, world demo in 'manually compiled Selfish'
object * primitive_hello(object * method, object * self) {
    object * arg = pop();
    if (arg == NULL || arg->type != STRING) {
        printf("Missing argument.\n");
    } else {
        printf("Hello, %s!\n", arg->string_value);
    }
    return NULL;
}

object * primitive_inspect(object * method, object * self) {
    for (int i=0; i<self->assoc_size; i++) {
        printf("%s ", to_name(self->assocs[i].number));
    }
    printf("\n");
    return NULL;
}

int main(void)
{
    object * arg = new_object(STRING);
    arg->string_value = "world";
    arg->value_size = sizeof(arg->string_value);

    push (arg);

    object * hello = new_object(PRIMITIVE);
    hello->primitive_method = primitive_hello;

    // now the Selfish code
    int selfName = to_number("self");
    int helloName = to_number("hello");

    object * method = new_object(METHOD);
    //method->code[] = { 1, -2, 0 }; // "world" self .hello;
    append_code(method, selfName);
    append_code(method, -helloName);
    append_code(method, 0);

    append_assoc(method, selfName, method);
    append_assoc(method, helloName, hello);

    callbacks[method->type](method, method);
 
    printf("Now doing a REPL. Please type stuff. (Note: only 'inspect' and '\"bla\" self.hello' currently work.)\n");

    int inspectName = to_number("inspect");
    object * inspect = malloc(sizeof(object));
    inspect->type = PRIMITIVE;
    inspect->primitive_method = primitive_inspect;

    object * repl = new_object(METHOD);
    append_assoc(repl, selfName, repl);
    append_assoc(repl, helloName, method);
    append_assoc(repl, inspectName, inspect);

    repl->type = METHOD;

    while(1==1) {
        printf("> ");
        free(repl->code);
        repl->code=NULL;
        repl->value_size = 0;
        parseCode(repl);
        object * result = callbacks[repl->type](repl,repl);
        if (result != NULL) push(result);
        print_stack();
    }

    return 0;
}
