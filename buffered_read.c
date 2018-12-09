#include "buffered_read.h"

int char_buffer = -2;

int line = 1;
int col = 0;

void reset_linecount() {
	line = 1;
	col = 0;
}

void expected(char expected, char got) {
	printf("Expected '%c', got '%c' at line %d:%d\n", expected, got, line, col);
}

void unexpected(const char * message) {
	printf(message);
	printf(" at line %d:%d\n", line, col);
}

void weird(const char * message, char got) {
	printf("Parse error at char '%c' at line %d:%d\n", got, line, col);
}

int buffered_read()
{
	int c;
	if (char_buffer != -2)
	{
		c = char_buffer;
		char_buffer = -2;
	}
	else {
		c = getchar();

		if (c == '\n') {
			line++;
			col = 0;
		} else col++;
	}

	return c;
}

void buffer_return(int c)
{
	char_buffer = c;
}
