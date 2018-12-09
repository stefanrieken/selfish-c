#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

typedef enum type_code {
    INT,
    STRING,
    METHOD,
    PRIMITIVE
} type_code;

// Note: other args are taken from the stack, which is a global (yes yes)
// but since at least these two args can be passed by register we try to optimize for that
typedef struct object object;
typedef object * (* type_invocation_method) (object * method, object * self);

typedef struct assoc {
    uint32_t number;
    object * value;
} assoc;

typedef struct object {
    uint32_t flags;
    type_code type;
    union {
        void * value;
        int int_value;
        char * string_value;
        int * code;
        type_invocation_method primitive_method;
    };
    int value_size;
    int assoc_size; // should not be required in tmmh
    assoc * assocs;
} object;

//
// object utils
//
extern void append_code(object * o, int c);
extern void prepend_code(object * o, int c);
extern void insert_code(object * o, int c);
extern void append_assoc(object * o, int number, object * value);

extern object * new_object(type_code type);

//
// stack
//
extern void push (object * o);
extern object * pop();
extern void clear_stack();
extern void print_stack();
//
// callbacks
//
extern type_invocation_method callbacks[];


//
// dictionary
//
extern int to_number(char * name);
extern char * to_name(int number);


//
// parse
//
void reset_linecount();
void expected(char expected, char got);
void unexpected(const char * message);
void weird(const char * message, char got);
bool parseCode(object * o);
