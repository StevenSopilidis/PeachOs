#include "pparser.h"
#include "../config.h"
#include "../status.h"
#include "../string/string.h"
#include "../memory/memory.h"
#include "../memory/heap/kheap.h"

// func that checks if the provided path is valid
// @filename: path to check
// returns int indicating wether the path is valid
// a valid path is something like 0:/xxx/xxx
// where 0-> is the drive number and the rest is path
// for now reletive path is not supported
static int pathparser_path_valid_format(const char* filename) {
    int len = strnlen(filename, PEACHOS_MAX_PATH);
    return (len >= 3 && is_digit(filename[0]) && memcmp((void*)&filename[1], ":/", 2) == 0);
}

// function for getting the drive number from path
// will move pointer to the first part of path
static int pathparser_get_drive_by_path(const char** path) {
    if(!pathparser_path_valid_format(*path)) {
        return -EBADPATH;
    }
    int drive_num = to_numeric_digit(*path[0]);
    // add 3 bytes to skip drive number (e.g 0:/, 1:/ etc)
    *path += 3;
    return drive_num;
}

static struct path_root* pathparser_create_root(int drive_number) {
    struct path_root* root = kzalloc(sizeof(struct path_root));
    root->drive_num = drive_number;
    root->first = NULL;
    return root;
}

// function for getting one part of path
// will move path to next part of path
static const char* pathparser_get_path_part(const char** path) {
    char* result_path_part = kzalloc(PEACHOS_MAX_PATH);
    int i = 0;
    while (**path != '/' && **path != '\0') {
        result_path_part[i] = **path;
        *path += 1;
        i++;   
    }
    if (**path == '/') {
        // skip forwrard slash to avoid problems
        *path += 1;
    }
    if (i == 0) {
        // if there is no part left release allocated data
        kfree(result_path_part);
        result_path_part = 0;
    }
    return result_path_part;
}

static struct path_part* pathparser_parse_path_part(struct path_part* last_part, const char** path) {
    const char* path_part_start = pathparser_get_path_part(path);
    if (!path_part_start)
        return 0;
    struct path_part* part = kzalloc(sizeof(struct path_part));
    part->part = path_part_start;
    part->next = NULL;
    if (last_part) {
        last_part->next = part;
    }
    return part;
}

void path_parser_free(struct path_root* root) {
    struct path_part* part = root->first;
    while (part)
    {
        struct path_part* next_part = part->next;
        kfree((void*)part->part);
        part = next_part;
    }
    kfree((void* )root);
}

struct path_root* pathparser_parse(const char* path, const char* current_directory_path) {
    int res = 0;
    const char* tmp_path = path;
    struct path_root* path_root = 0;
    if(strlen(path) > PEACHOS_MAX_PATH)
        goto out;
    res = pathparser_get_drive_by_path(&tmp_path);
    if (res < 0)
        goto out;
    path_root = pathparser_create_root(res);
    if (!path_root)
        goto out;
    struct path_part* first_path = pathparser_parse_path_part(NULL, &tmp_path);
    if (!first_path)
        goto out;
    path_root->first = first_path;
    struct path_part* part = pathparser_parse_path_part(first_path, &tmp_path);
    while (part)
    {
        part =  pathparser_parse_path_part(part, &tmp_path);
    }
out:
    return path_root;    
}