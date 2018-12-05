#include <stdio.h>
#include <malloc.h>
#include <stdbool.h>
#include "buffered_read.h"
#include "selfish.h"

typedef bool (* condition) (int ch);

bool string_condition(int ch) {
	return ch != '"'; // TODO escapes
}

bool name_condition(int ch) {
	return (ch >= 'A' && ch <= 'Z') ||
		(ch >= 'a' && ch <= 'z') ||
		(ch >= '0' && ch <= '9') ||  ch == '_';
}

char * parse(condition cond, int ch) {
	char * str = malloc(sizeof(char) * 2);
	int str_size = 0;

	while (cond(ch)) {
		str = realloc(str, sizeof(char) * (str_size+1));
		str[str_size] = ch;
		str_size++;

		ch = buffered_read();
	}

	buffer_return(ch);
	str = realloc(str, sizeof(char) * (str_size+1));
	str[str_size] = '\0';
	return str;
}

void parseName(int ch, object * o) {
	char * name = parse(name_condition, ch);
	append_code(o, to_number(name));
}

void parseDotInvocation(object * o) {
	parseName(buffered_read(), o);
	o->code[o->value_size-1] = -(o->code[o->value_size-1]); // negate
	// TODO bracket arguments
}

void parseString(object * o) {
	char * str = parse(string_condition, buffered_read());
	buffered_read(); // drop final '"'

	object * s = malloc(sizeof(object));
	s->type = STRING;
	s->string_value = str;
	s->value_size = sizeof(str);

	int num = to_number("");
	append_assoc(o, num, s);
	append_code(o, num);
}

void parseInt(int ch, object * o) {
	int value = 0;
	while (ch >= '0' && ch <= '9') {
		value = value * 10 + ch;
		ch = buffered_read();
	}
	buffer_return(ch);

	object * i = malloc(sizeof(object));
	i->type = INT;
	i->int_value = value;

	int num = to_number("");
	append_assoc(o, num, i);
	append_code(o, num);
}


void parseCode(object * o) {
	int ch = buffered_read();
	int i = 1;

	while (ch != EOF) {
		while (ch == ' ' || ch == '\t') {
			ch = buffered_read();
			i++;
		}

		if (ch == '.') {
			parseDotInvocation(o);
		} else if (ch == '"') {
			parseString(o);
		} else if (ch >= '0' && ch <= '9') {
			parseInt(ch, o);
		} else if ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') || ch == '_') {
			parseName(ch, o);
		} else {
			if (ch != '\n') {
				printf("Parse error at char %d ('%c')\n", i, ch);
				// TODO clear object
			}
			return;
		}

		ch = buffered_read();
		i++;
	}
	if (ch != EOF) buffer_return(ch);
}

