#ifndef _PATHPARSER_H_
#define _PATHPARSER_H_

struct path_root {
    int drive_num;
    struct path_part* first;
};

// for e.g 0:/test.txt
// path_root.drive_num == 0
// the other half is the first
// so int this case the first is the test.txt
// and it would link to the next part of the path
// so for every text inside /<text>/ text is a path_part
struct path_part {
    const char* part;
    struct path_part* next;
};

struct path_root* pathparser_parse(const char* path, const char* current_directory_path);
void path_parser_free(struct path_root* root);

#endif