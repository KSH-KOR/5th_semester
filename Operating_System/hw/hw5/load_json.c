#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <json-c/json.h>

struct file_entry {
    char name[256];
    int inode;
};

struct file {
    int inode;
    char type[4];
    char data[4098];
    struct file_entry entries[16];
    int num_entries;
};

struct fs {
    const char* json_file;
    int num_files;
    struct file* files;
};

static struct fs filesystem;

void load_filesystem(const char* json_file) {
    struct json_object* fs_json = json_object_from_file(json_file);
    filesystem.json_file = json_file;
    filesystem.num_files = json_object_array_length(fs_json);
    filesystem.files = (struct file*)malloc(sizeof(struct file) * filesystem.num_files);

    for (int i = 0; i < filesystem.num_files; i++) {
        struct json_object* file_json = json_object_array_get_idx(fs_json, i);
        struct file* file = &filesystem.files[i];

        json_object_object_foreach(file_json, key, val) {
            if (strcmp(key, "inode") == 0) {
                file->inode = json_object_get_int(val);
            } else if (strcmp(key, "type") == 0) {
                strcpy(file->type, json_object_get_string(val));
            } else if (strcmp(key, "data") == 0) {
                strcpy(file->data, json_object_get_string(val));
            } else if (strcmp(key, "entries") == 0) {
                int num_entries = json_object_array_length(val);
                file->num_entries = num_entries;
                for (int j = 0; j < num_entries; j++) {
                    struct json_object* entry_json = json_object_array_get_idx(val, j);
                    struct file_entry* entry = &file->entries[j];
                    json_object_object_foreach(entry_json, entry_key, entry_val) {
                        if (strcmp(entry_key, "name") == 0) {
                            strcpy(entry->name, json_object_get_string(entry_val));
                        } else if (strcmp(entry_key, "inode") == 0) {
                            entry->inode = json_object_get_int(entry_val);
                        }
                    }
                }
            }
        }
    }

    json_object_put(fs_json);
}

void print_filesystem() {
    printf("File System:\n");

    for (int i = 0; i < filesystem.num_files; i++) {
        struct file* file = &filesystem.files[i];

        printf("File Inode: %d\n", file->inode);
        printf("File Type: %s\n", file->type);

        if (strcmp(file->type, "reg") == 0) {
            printf("File Data: %s\n", file->data);
        } else if (strcmp(file->type, "dir") == 0) {
            printf("Directory Entries:\n");

            for (int j = 0; j < file->num_entries; j++) {
                struct file_entry* entry = &file->entries[j];

                printf("Name: %s\n", entry->name);
                printf("Inode: %d\n", entry->inode);
            }
        }

        printf("----------------------\n");
    }
}


int main() {
    const char* json_file = "fs.json";
    load_filesystem(json_file);

    // Access the file system data structure and perform operations
    print_filesystem();
    return 0;
}

