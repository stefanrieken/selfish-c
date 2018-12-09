#include <stdio.h>
#include <malloc.h>
#include <stdbool.h>
#include <string.h>
#include "buffered_read.h"
#include "selfish.h"

typedef bool (* condition) (int ch);

object * new_object(type_code type) {

	object * o = malloc(sizeof(object));
	o->type = type;

	o->flags = 0;
	o->value = NULL;
	o->value_size=0;
	o->assoc_size=0;
	o->assocs = NULL;

	return o;
}

bool whitespace_condition(int ch) {
	return ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n';
}

bool whitespace_condition_no_eol(int ch) {
	return ch == ' ' || ch == '\t' || ch == '\r';
}

int readChar(condition cond) {
	int ch = buffered_read();
	while (cond(ch)) {
		ch = buffered_read();
	}
	return ch;
}

bool string_condition(int ch) {
	return ch != '"'; // TODO escapes
}

bool name_condition(int ch) {
	return (ch >= 'A' && ch <= 'Z') ||
		(ch >= 'a' && ch <= 'z') ||
		(ch >= '0' && ch <= '9') ||  ch == '_';
}

// used by parseCode in case it parses args
bool arg_condition(int ch) {
	return ch != ',' && ch != ')' && ch != EOF;
}

// used by parseCode in case it parses unbracketed code
bool code_condition(int ch) {
	return ch != EOF;
}

// used by parseCode in case it parses unbracketed code
bool bracketed_condition(int ch) {
	return ch != '}' && ch != EOF;
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

bool parseName(int ch, object * o) {
	char * name = parse(name_condition, ch);
	append_code(o, to_number(name));
	return true;
}

bool parse_code(object * o, condition cond);
bool parseBracketArguments(object * o) {
	int ch = readChar(whitespace_condition_no_eol); // TODO only works for REPL
	if (ch == '(') {
		int base_position = o->value_size - 2;

		int position = o->value_size;
		parse_code(o, arg_condition);
		while (o->value_size > position) {
			// stuff got added
			// e.g. a.b(c.d, e.f); . Either c.d was added and must be inserted before a.b,
			// or e.f was added and must be inserted before both.
			// this is for now assuming there is always an explicit 'a' (or 'self')
			// in other words, base position remains the same during the whole procedure.
			// how do we move the stuff? by bubbling
			for (int j = 0; j< o->value_size - base_position - 1; j++) {
				for (int i=base_position; i<o->value_size-1; i++) {
					int tmp = o->code[i];
					o->code[i] = o->code[i+1];
					o->code[i+1] = tmp;
				}
			}

			position = o->value_size;
			ch = readChar(whitespace_condition);
			if (ch == ',') 	parse_code(o, arg_condition);
		}
		if (ch != ')') {
			printf("Expected ')' got %c\n", ch);
			return false; // TODO abort all parsing
		}
		return true;
	} else {
		buffer_return(ch);
		return true; // no args though
	}
	return false;
}

bool parseDotInvocation(object * o) {
	parseName(buffered_read(), o);
	o->code[o->value_size-1] = -(o->code[o->value_size-1]); // negate
	return parseBracketArguments(o);
}

object * parseString() {
	char * str = parse(string_condition, buffered_read());
	buffered_read(); // drop final '"'

	object * s = new_object(STRING);
	s->string_value = str;
	s->value_size = sizeof(str);

	return s;
}

object * parseInt(int ch) {
	int value = 0;
	while (ch >= '0' && ch <= '9') {
		value = value * 10 + ch;
		ch = buffered_read();
	}
	buffer_return(ch);

	object * i = new_object(INT);
	i->int_value = value;

	return i;
}

// parse code in brackets.
object * parseObject() {
	object * newObject = new_object(METHOD);
	if (!parse_code(newObject, bracketed_condition)) {
		printf("Expected object body.");
		return NULL;
	} else {
		int ch = readChar(whitespace_condition);
		if (ch != '}') 	{
			printf("Expected '}'");
			return NULL;
		}
		return newObject;
	}
}

object * parseLiteral(int ch) {
	object * literal = NULL;
	if (ch == '"') {
		literal = parseString();
	} else if (ch >= '0' && ch <= '9') {
		literal = parseInt(ch);
	} else if (ch == '{') {
		literal = parseObject();
	}

	return literal;
}

bool parse_code(object * o, condition cond) {
	int ch = readChar(whitespace_condition_no_eol); // TODO this condition is only interesting for REPL

	bool success = true;
	while (success && cond(ch)) {
		object * literal = parseLiteral(ch);
		if (literal != NULL) {
			int num = to_number("");
			append_assoc(o, num, literal);
			append_code(o, num);
		} else if (ch == '.') {
			success = parseDotInvocation(o);
		} else if ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') || ch == '_') {
			success = parseName(ch, o);
		} else {
			if (ch != '\n') {
				printf("Parse error at char '%c'\n", ch);
				return false;
				// TODO clear object
			}
			return true;
		}

		ch = readChar(whitespace_condition_no_eol);
	}
	if (ch != EOF) buffer_return(ch);
	return true;
}

// parse unbracketed code (i.e. top level object)
bool parseCode(object * o) {	
	return parse_code(o, code_condition);
}
