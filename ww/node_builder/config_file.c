#include "config_file.h"
#include "loggers/internal_logger.h"
#include "utils/json_helpers.h"


void destroyConfigFile(config_file_t *state)
{
    cJSON_Delete(state->root);

    if (state->file_path != NULL)
    {
        memoryFree(state->file_path);
    }
    if (state->name != NULL)
    {
        memoryFree(state->name);
    }
    if (state->author != NULL)
    {
        memoryFree(state->author);
    }
    mutexDestroy(&(state->guard));

    memoryFree(state);
}

void acquireUpdateLock(config_file_t *state)
{
    mutexLock(&(state->guard));
}

void releaseUpdateLock(config_file_t *state)
{
    mutexUnlock(&(state->guard));
}

// only use if you acquired lock before
void unsafeCommitChanges(config_file_t *state)
{
    char     *string      = cJSON_PrintBuffered(state->root, (int) ((state->file_prebuffer_size) * 2), true);
    size_t    len         = strlen(string);
    const int max_retries = 3;
    for (int i = 0; i < max_retries; i++)
    {
        if (writeFile(state->file_path, string, len))
        {
            return;
        }
        LOGE("WriteFile Error: could not write to \"%s\". retry...", state->file_path);
    }
    LOGE("WriteFile Error: giving up writing to config file \"%s\"", state->file_path);
}

void commitChangesHard(config_file_t *state)
{
    acquireUpdateLock(state);
    unsafeCommitChanges(state);
    releaseUpdateLock(state);
}

// will not write if the mutex is locked
void commitChangesSoft(config_file_t *state)
{
#ifdef OS_WIN
    commitChangesHard(state);
#else
    if (mutexTryLock(&(state->guard)))
    {
        unsafeCommitChanges(state);
        releaseUpdateLock(state);
    }

#endif
}

config_file_t *parseConfigFile(const char *const file_path)
{
    config_file_t *state = memoryAllocate(sizeof(config_file_t));
    memorySet(state, 0, sizeof(config_file_t));
    mutexInit(&(state->guard));

    state->file_path = memoryAllocate(strlen(file_path) + 1);
    strcpy(state->file_path, file_path);

    char *data_json = readFile(file_path);

    if (! data_json)
    {
        LOGF("File Error: config file \"%s\" could not be read", file_path);
        return NULL;
    }
    state->file_prebuffer_size = strlen(data_json);

    cJSON *json = cJSON_ParseWithLength(data_json, state->file_prebuffer_size);
    state->root = json;

    if (json == NULL)
    {
        LOGF("JSON Error: config file \"%s\" could not be parsed", file_path);
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL)
        {
            LOGF("JSON Error: before: %s\n", error_ptr);
        }
        return NULL;
    }
    memoryFree(data_json);

    if (! getStringFromJsonObject((&state->name), json, "name"))
    {
        LOGF("JSON Error: config file \"%s\" -> name (string field) the value was empty or invalid", file_path);
        return NULL;
    }

    getStringFromJsonObjectOrDefault(&(state->author), json, "author", "EMPTY_AUTHOR");
    getIntFromJsonObject(&(state->config_version), json, "config-version");
    getIntFromJsonObject(&(state->core_minimum_version), json, "core-minimum-version");
    getBoolFromJsonObject(&(state->encrypted), json, "encrypted");
    cJSON *nodes = cJSON_GetObjectItemCaseSensitive(json, "nodes");
    if (! (cJSON_IsArray(nodes) && nodes->child != NULL))
    {
        LOGF("JSON Error: config file \"%s\" -> nodes (array field) the array was empty or invalid", file_path);
        exit(1);
    }
    state->nodes = nodes;
    return state;
}
