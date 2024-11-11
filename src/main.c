#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <ctype.h>
#include <iconv.h>
#include "json.h"
#include "http.h"

#define quote(S) ("\"" S "\"")
#define base "https://fiks.fit.cvut.cz/api/v1"
#define boundary "---------------------------419656455689383822581148678"
#define subm_start (boundary"\nContent-Disposition: form-data; name=\"outputFile\"; filename=\"output.txt\"\nContent-Type: text/plain\n")
#define ARMED

/*************************\
* fiks.json: ffiks config *
\*************************/

/* {
 * 	"login": {
 * 		"email": "david@kosatovi.cz",
 * 		"password": "pije314159"
 * 	},
 * 	"problem": "Podivná komunikace II",
 * 	"run": "cat input.txt | ./bin/test > output.txt"
 * }
 */

int main(int argc, char** argv) {
	FILE* fset;
	if (argc > 1) {
		fset = fopen(argv[1], "r");
	} else {
		fset = fopen("fiks.json", "r");
	}
	assert(fset);

	gstr settings = {malloc(1), 0, 1};
	while (!feof(fset)) {
		char buffer[512] = {0};
		fgets(buffer, 511, fset);
		cstring((char*)buffer, 1, strlen(buffer), &settings);
	}

	char* run = walk_struct(settings.s, quote("run"));
	char* problem = walk_struct(settings.s, quote("problem"));
	char* login = walk_struct(settings.s, quote("login"));
	*(end_token(run)-1) = '\0';
	*end_token(problem) = '\0';
	*end_token(login) = '\0';
	run++;

	gstr gs = {malloc(1), 0, 1};
	gstr cookie = {malloc(1), 0, 1};

	*cookie.s = '\0';
	char* path = calloc(1024, 1);

	curl_post_str(base "/login", &gs, login, &cookie);

	gs.len = 0;
	curl_get_str(base "/seasons", &gs, &cookie);

	char *s = gs.s;
	select(array, s, 0);
	select(struct, s, quote("id"));

	int season_id = atoi(s);

	gs.len = 0;
	sprintf(path, base "/season/by-id/%i/rounds", season_id);
	curl_get_str(path, &gs, &cookie);
	
	s = gs.s;
	select(array, s, 0);
	select(struct, s, quote("id"));

	int round_id = atoi(s);

	gs.len = 0;
	sprintf(path, base "/round/by-id/%i/problems", round_id);
	curl_get_str(path, &gs, &cookie);

	int problem_id;
	s = gs.s;
	for (size_t i = 0; i < 10; i++) {
		char* x = walk_array(s, i);
		char* title = walk_struct(x, quote("title"));
		if (!strncmp(title, problem, strlen(problem))) {
			char* xend = end_token(x);
			*xend = '\0';
			select(struct, x, quote("id"));
			problem_id = atoi(x);
			break;
		}
	}

#ifdef ARMED
	gs.len = 0;
	sprintf(path, base "/problem/by-id/%i/sfinx/submission/create", problem_id);
	int status = curl_post_str(path, &gs, "", &cookie);
	int submission_id;
	if (status != 200) {
		gs.len = 0;
		sprintf(path, base "/problem/by-id/%i/sfinx/submissions/contestant", problem_id);
		curl_get_str(path, &gs, &cookie);
		s = gs.s;
		select(array, s, 0);
	} else {
		s = gs.s;
	}
	select(struct, s, quote("submissionId"));
	submission_id = atoi(s);

	sprintf(path, base "/submission/%i/sfinx/input", submission_id);
	FILE* in = fopen("input.txt", "w");
	curl_get_file(path, in, &cookie);

	system(run);

	gs.len = 0;
	sprintf(path, base "/problem/by-id/%i/sfinx/submission/submit", problem_id);
	FILE* out = fopen("output.txt", "r");
	curl_post_file(path, &gs, out, &cookie);

	s = gs.s;
	select(struct, s, quote("score"));
	printf("%s%%\n", s);
#else
	printf("%i\n", problem_id);

	curl_get_file(base "/user/profile", stdout, &cookie);
#endif
	return 0;
}
