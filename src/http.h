#include <string.h>
#include <assert.h>
#include <curl/curl.h>
typedef struct {
	char *s;
	size_t len;
	size_t cap;
} gstr;

size_t procheader(char* data, size_t size, size_t nmemb, gstr* cookie) {
	assert(size == 1);
	if (strncmp(data, "set-cookie:", strlen("set-cookie:"))) return nmemb;
	data += strlen("set-cookie:");
	nmemb -= strlen("set-cookie:");
	skip_space(&data);
	if (cookie->len + nmemb > cookie->cap) {
		cookie->cap = cookie->len+nmemb;
		cookie->s = realloc(cookie->s, cookie->cap);
	}
	memcpy(cookie->s, data, nmemb);
	cookie->len += nmemb;
	for (size_t i = 0; i < nmemb; i++) {
		if (cookie->s[i] == ';') {
			cookie->s[i] = '\0';
			break;
		}
	}
	return nmemb+strlen("set-cookie:");
}

ssize_t cstring(char* data, size_t size, size_t nmemb, gstr* str) {
	assert(size == 1);
	if (str->len + nmemb > str->cap) {
		while (str->len + nmemb > str->cap) {
			str->cap *= 2;
		}
		str->s = realloc(str->s, str->cap);
	}
	memcpy(str->s+str->len, data, nmemb);
	str->len += nmemb;
	str->s[str->len] = '\0';
	return nmemb;
}

int curl_get_str(char* url, gstr* str, gstr* cookie) {
	CURL* curl;
	CURLcode res;
	long http_code = 0;

	curl = curl_easy_init();
	if (!curl) goto cleanup;
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, str);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, cstring);
	curl_easy_setopt(curl, CURLOPT_URL, url);
	if (cookie->len) {
		curl_easy_setopt(curl, CURLOPT_COOKIE, cookie->s);
	}
	res = curl_easy_perform(curl);
	assert(res == CURLE_OK);
	curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &http_code);
cleanup:
	curl_easy_cleanup(curl);
	return http_code;
}

int curl_get_file(char* url, FILE* str, gstr* cookie) {
	CURL* curl;
	CURLcode res;
	long http_code = 0;

	curl = curl_easy_init();
	if (!curl) goto cleanup;
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, str);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, fwrite);
	curl_easy_setopt(curl, CURLOPT_URL, url);
	if (cookie->len) {
		curl_easy_setopt(curl, CURLOPT_COOKIE, cookie->s);
	}
	res = curl_easy_perform(curl);
	assert(res == CURLE_OK);
	curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &http_code);
cleanup:
	curl_easy_cleanup(curl);
	return http_code;
}

int curl_post_file(char* url, gstr* rsp, FILE* str, gstr* cookie) {
	CURL* curl;
	CURLcode res;
	long http_code = 0;

	struct curl_slist *ll = NULL;

	curl = curl_easy_init();
	if (!curl) goto cleanup;
	ll = curl_slist_append(ll, "Origin: https://fiks.fit.cvut.cz");
	curl_mime* mime1 = curl_mime_init(curl);
	curl_mimepart* part1 = curl_mime_addpart(mime1);
	curl_mime_filedata(part1, "output.txt");
	curl_mime_name(part1, "outputFile");
	curl_mime_type(part1, "text/plain");
	curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime1);
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, ll);
	curl_easy_setopt(curl, CURLOPT_READDATA, str);
	curl_easy_setopt(curl, CURLOPT_READFUNCTION, fread);
	curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, procheader);
	curl_easy_setopt(curl, CURLOPT_HEADERDATA, cookie);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, rsp);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, cstring);
	if (cookie->len) {
		curl_easy_setopt(curl, CURLOPT_COOKIE, cookie->s);
	}
	curl_easy_setopt(curl, CURLOPT_URL, url);
	res = curl_easy_perform(curl);
	assert(res == CURLE_OK);
	curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &http_code);
cleanup:
	curl_easy_cleanup(curl);
	return http_code;
}

int curl_post_str(char* url, gstr* rsp, char* str, gstr* cookie) {
	CURL* curl;
	CURLcode res;
	long http_code = 0;

	struct curl_slist *ll = NULL;

	curl = curl_easy_init();
	if (!curl) goto cleanup;
	ll = curl_slist_append(ll, "Origin: https://fiks.fit.cvut.cz");
	ll = curl_slist_append(ll, "Content-Type: application/json");
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, ll);
	curl_easy_setopt(curl, CURLOPT_POST, 1);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, str);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(str));
	curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, procheader);
	curl_easy_setopt(curl, CURLOPT_HEADERDATA, cookie);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, rsp);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, cstring);
	if (cookie->len) {
		curl_easy_setopt(curl, CURLOPT_COOKIE, cookie->s);
	}
	curl_easy_setopt(curl, CURLOPT_URL, url);
	res = curl_easy_perform(curl);
	assert(res == CURLE_OK);
	curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &http_code);
cleanup:
	curl_easy_cleanup(curl);
	return http_code;
}

