#pragma once

#include <dill/util_list.h>

#include <stdint.h>
#include <stdbool.h>

// ??????
#define CONFIG_FILELIST_READ = 0x01

typedef struct ConfigFileList {
	struct dill_list list;
	char * name;
	intptr_t flags;
} ConfigFileList;

enum {
	CONFIG_ERROR_DEBUG,
	CONFIG_ERROR_DEBUG_KEY,
	CONFIG_ERROR_NO_SUCH_VARIABLE,
	CONFIG_ERROR_BAD_LINE,
	CONFIG_ERROR_COUNT
};

enum {
	CONFIG_TYPE_INT8,
	CONFIG_TYPE_INT16,
	CONFIG_TYPE_INT32,
	CONFIG_TYPE_INT64,
	CONFIG_TYPE_STRING,
	CONFIG_TYPE_BOOL,
	CONFIG_TYPE_COUNT
};

typedef void (*ConfigFailureFunction)(void *, const char *, int64_t, int64_t, const char *, const char *);
typedef bool (*ConfigValidator)(void *, const void *);

typedef struct {
	struct dill_list list;
	void * p;
	ConfigValidator f;
	intptr_t num, /* Length for STRING, shift count for BOOL */
		type,
		len; /* Length of key */
	char * key;
} ConfigVariable;

typedef struct {
	struct dill_list variables, files;
	void * p;
	ConfigFailureFunction fail;
} Config;

bool configInit         (Config *, void *);
void configFail         (Config *, ConfigFailureFunction);
bool configParseLine    (Config *, const char *);
bool configReadLine(Config *cfg, const char *line, size_t len, const char *file, intptr_t lineno);
bool configReadKV(Config *cfg, const char *key, const char *value, size_t keyLen, size_t valueLen, const char *file, intptr_t lineno);
void configClear        (Config *);
void configClearCache   (Config * cfg);

void configAssignInt8   (Config *, const char *,   int8_t *,           ConfigValidator f);
void configAssignInt16  (Config *, const char *,  int16_t *,           ConfigValidator f);
void configAssignInt32  (Config *, const char *,  int32_t *,           ConfigValidator f);
void configAssignInt64  (Config *, const char *,  int64_t *,           ConfigValidator f);
void configAssignString (Config *, const char *,     char *, intptr_t, ConfigValidator f);
void configAssignBool   (Config *, const char *, intptr_t *, intptr_t, ConfigValidator f);
