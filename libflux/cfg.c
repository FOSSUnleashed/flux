#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>

#include <string.h>

#include <cfg.h>

#define cfg_foreach(head, it, cur) dill_list_T_foreach(head, Config, list, it, cur)
#define cfgv_foreach(head, it, cur) dill_list_T_foreach(head, ConfigVariable, list, it, cur)

ConfigVariable* configVariableNew(Config * cfg, const char * str, intptr_t type) {
	ConfigVariable * cv = NULL;

	// Invalid type value
	if (type >= CONFIG_TYPE_COUNT) {
		return NULL;
	}

	// Allocate new list node
	if (NULL == (cv = calloc(1, sizeof(ConfigVariable)))) {
		return NULL;
	}

	// Try to allocate the key, cleanup if that fails.
	cv->type = type;
	if (NULL == (cv->key = strdup(str))) {
		free(cv);
		return NULL;
	}
	cv->len = strlen(str);

	dill_list_insert(&cv->list, &cfg->variables);

	return cv;
}

void configVariableFree(ConfigVariable * cv) {
	dill_list_erase(&cv->list);
	free(cv->key);
	free(cv);
}

void configFailure(Config * cfg, const char * linestr, const char * file, intptr_t line, intptr_t errcode, const char * errstr) {
	if (NULL != cfg->fail) {
		cfg->fail(cfg->p, file, line, errcode, errstr, linestr);
	}
}

#define IS_KEY_SEP(c) (':' == (c) || ' ' == (c) || '=' == (c) || '\t' == (c))

bool configReadLine(Config * cfg, const char * line, size_t len, const char * file, intptr_t lineno) {
	const char * str;

	size_t keyLen;

	for (str = line; !IS_KEY_SEP(*str); ++str); // iterate out of the key

	keyLen = str - line;

	for (; IS_KEY_SEP(*str); ++str); // iterate to the start of the value

	return configReadKV(cfg, line, str, keyLen, line + len - str, file, lineno);
}

bool configReadKV(Config *cfg, const char *key, const char *value, size_t keyLen, size_t valueLen, const char *file, intptr_t lineno) {
	ConfigVariable *v;
	struct dill_list *it;
	void * p;
	uint64_t i;
	size_t sz;

	cfgv_foreach(&cfg->variables, it, v) {
		if (keyLen != v->len) { // keylength mismatch, we won't match the key
			continue;
		}

		if (strncasecmp(v->key, key, v->len)) { // key does not match
			continue;
		}

		p = &i;

		switch (v->type) {
			case CONFIG_TYPE_INT8:
				*((int8_t*)(p)) = strtoll(value, NULL, 10);
				if (NULL == v->f || v->f(cfg->p, p))
					*((int8_t*)v->p) = *((int8_t*)p);
				break;
			case CONFIG_TYPE_INT16:
				*((int16_t*)(p)) = strtoll(value, NULL, 10);
				if (NULL == v->f || v->f(cfg->p, p))
					*((int16_t*)v->p) = *((int16_t*)p);
				break;
			case CONFIG_TYPE_INT32:
				*((int32_t*)(p)) = strtoll(value, NULL, 10);
				if (NULL == v->f || v->f(cfg->p, p))
					*((int32_t*)v->p) = *((int32_t*)p);
				break;
			case CONFIG_TYPE_INT64:
				*((int64_t*)(p)) = strtoll(value, NULL, 10);
				if (NULL == v->f || v->f(cfg->p, p))
					*((int64_t*)v->p) = *((int64_t*)p);
				break;
			case CONFIG_TYPE_STRING:
				if (NULL == v->f || v->f(cfg->p, value)) {
					sz = valueLen > (v->num - 1) ? v->num - 1 : valueLen;
					memcpy(v->p, value, sz);
					((char *)v->p)[sz] = 0;
				}
				break;
		}

		return true;
	}

	return false;
}

/*
 * Start public config*() functions
 */
bool configInit         (Config * cfg, void * opt) {
	if (NULL == cfg) {
		return false;
	}

	bzero(cfg, sizeof(Config));
	cfg->p = opt;

	dill_list_init(&cfg->variables);
	dill_list_init(&cfg->files);

	return true;
}

void configFail         (Config * cfg, ConfigFailureFunction fail) {
	cfg->fail = fail;
}

bool configParseLine    (Config * cfg, const char * str) {
	return configReadLine(cfg, str, strlen(str), NULL, 0); // NULL filename 0 linenumber
}

void configClear        (Config * cfg) {
	configClearCache(cfg);
	while (!dill_list_empty(&cfg->variables)) {
		configVariableFree(dill_cont(dill_list_next(&cfg->variables), ConfigVariable, list));
	}
}

void configClearCache   (Config * cfg) {
}

void configAssignInt8   (Config * cfg, const char * name,   int8_t * p,               ConfigValidator f) {
	ConfigVariable *v;

	if (NULL == p) {
		configFailure(cfg, name, NULL, 0, CONFIG_ERROR_DEBUG_KEY, "No pointer for key");
		return;
	}

	v = configVariableNew(cfg, name, CONFIG_TYPE_INT8);
	v->p = p;
	v->f = f;
}

void configAssignInt16  (Config * cfg, const char * name,  int16_t * p,               ConfigValidator f) {
	ConfigVariable *v;

	if (NULL == p) {
		configFailure(cfg, name, NULL, 0, CONFIG_ERROR_DEBUG_KEY, "No pointer for key");
		return;
	}

	v = configVariableNew(cfg, name, CONFIG_TYPE_INT16);
	v->p = p;
	v->f = f;
}

void configAssignInt32  (Config * cfg, const char * name,  int32_t * p,               ConfigValidator f) {
	ConfigVariable *v;

	if (NULL == p) {
		configFailure(cfg, name, NULL, 0, CONFIG_ERROR_DEBUG_KEY, "No pointer for key");
		return;
	}

	v = configVariableNew(cfg, name, CONFIG_TYPE_INT32);
	v->p = p;
	v->f = f;
}

void configAssignInt64  (Config * cfg, const char * name,  int64_t * p,               ConfigValidator f) {
	ConfigVariable *v;

	if (NULL == p) {
		configFailure(cfg, name, NULL, 0, CONFIG_ERROR_DEBUG_KEY, "No pointer for key");
		return;
	}

	v = configVariableNew(cfg, name, CONFIG_TYPE_INT64);
	v->p = p;
	v->f = f;
}

void configAssignString (Config * cfg, const char * name,     char * p, intptr_t num, ConfigValidator f) {
	ConfigVariable *v;

	if (NULL == p || 0 == num) {
		configFailure(cfg, name, NULL, 0, CONFIG_ERROR_DEBUG_KEY, "No pointer for key");
		return;
	}

	v = configVariableNew(cfg, name, CONFIG_TYPE_STRING);
	v->p = p;
	v->f = f;
	v->num = num;
}

void configAssignBool   (Config * cfg, const char * name, intptr_t * p, intptr_t num, ConfigValidator f) {
}
