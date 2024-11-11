#include <string.h>
#define eprintf(...) fprintf(stderr, __VA_ARGS__)

int skip_shit(char** s);

void skip_space(char** s) {
	while (**s == ' ' || **s == '\n' || **s == '\t') (*s)++;
}

int skip_bool(char** s) {
	skip_space(s);
	if (!strncmp(*s, "true", strlen("true"))) {
		*s += strlen("true");
		return strlen("true");
	}
	if (!strncmp(*s, "false", strlen("false"))) {
		*s += strlen("false");
		return strlen("false");
	}
	return 0;
}

int skip_null(char** s) {
	skip_space(s);
	if (!strncmp(*s, "null", strlen("null"))) {
		*s += strlen("null");
		return strlen("null");
	}
	return 0;
}

int skip_string(char** s) {
	skip_space(s);
	char* start = *s;
	if (**s != '"') goto rewind;
	(*s)++;
	while (**s != '"') {
		if (**s == '\\') (*s)++;
		if (!**s) goto rewind;
		(*s)++;
		if (!**s) goto rewind;
	}
	(*s)++;
	return *s-start;
rewind:
	*s = start;
	return 0;
}

int skip_num(char** s) {
	skip_space(s);
	char* start = *s;
	if (**s == '-') **s++;
	if (**s > '9' || **s < '0') goto rewind;
	(*s)++;
	while (**s >= '0' && **s <= '9') (*s)++;
	if (**s == '.') {
		(*s)++;
		while (**s >= '0' && **s <= '9') (*s)++;
	}
	return *s-start;
rewind:
	*s = start;
	return 0;
}

int skip_arr(char** s) {
	skip_space(s);
	char* start = *s;
	if (**s != '[') goto rewind;
	skip_space(s);
	(*s)++;
	while (**s != ']') {
		if (!skip_shit(s)) goto rewind;
		skip_space(s);
		if (!**s) goto rewind;
		if (**s == ',') (*s)++;
		skip_space(s);
	}
	if (**s != ']') goto rewind;
	(*s)++;
	return *s-start;
rewind:
	*s = start;
	return 0;
}

int skip_struct(char** s) {
	skip_space(s);
	char* start = *s;
	if (**s != '{') goto rewind;
	skip_space(s);
	(*s)++;
	while (**s != '}') {
		if (!skip_string(s)) goto rewind;
		skip_space(s);
		if (!**s) goto rewind;
		if (**s == ':') {
			(*s)++;
		} else {
			goto rewind;
		}
		if (!skip_shit(s)) goto rewind;
		skip_space(s);
		if (!**s) goto rewind;
		if (**s == ',') (*s)++;
		skip_space(s);
	}
	(*s)++;
	return *s-start;
rewind:
	*s = start;
	return 0;
}

int skip_shit(char** s) {
	char* start = *s;
	if (!skip_string(s) && !skip_num(s) && !skip_arr(s) && !skip_struct(s) && !skip_bool(s) && !skip_null(s)) return 0;
	return *s-start;
}

char* walk_struct(char* s, char* key) {
	skip_space(&s);
	if (*s++ != '{') return NULL;
	skip_space(&s);
	while (strncmp(s, key, strlen(key))) {
		if (!skip_string(&s)) return NULL;
		skip_space(&s);
		if (*s++ != ':') return NULL;
		skip_space(&s);
		if (!skip_shit(&s)) return NULL;
		skip_space(&s);
		if (*s++ != ',') return NULL;
		skip_space(&s);
	}
	if (!skip_string(&s)) return NULL;
	skip_space(&s);
	if (*s++ != ':') return NULL;
	skip_space(&s);
	return s;
}

char* walk_array(char* s, int n) {
	skip_space(&s);
	if (*s++ != '[') return NULL;
	skip_space(&s);
	for (size_t i = 0; i < n; i++) {
		if (!skip_shit(&s)) return NULL;
		skip_space(&s);
		if (*s++ != ',') return NULL;
		skip_space(&s);
	}
	return s;
}

char* end_token(char* s) {
	if (!skip_shit(&s)) return NULL;
	return s;
}

//mutates the string
#define select(F, S, X) S = walk_##F(S, X); {char* send = end_token(S); *send = '\0';}
