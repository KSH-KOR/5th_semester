#define FUSE_USE_VERSION 26

#include <stdio.h>
#include <fuse.h>
#include <string.h>
#include <errno.h>
#include <json-c/json.h>
#include <libgen.h>
#include <pthread.h>

#define MAX_FILENAME_LENGTH 256
#define MAX_ENTRIES 1024
#define MAX_DATA_SIZE 4096

struct file_entry {
    char name[MAX_FILENAME_LENGTH];
    int inode;
};


struct file {
    int inode;
    char type[MAX_FILENAME_LENGTH];
    char data[MAX_DATA_SIZE];
    struct file_entry entries[MAX_ENTRIES];
    int num_entries;
};

struct fs {
    struct file root;
    struct file* files;
    int num_files;
    const char* json_file;
};

static struct fs filesystem;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;


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

void save_filesystem() {
    struct json_object* fs_json = json_object_new_array();

    for (int i = 0; i < filesystem.num_files; i++) {
        struct file* file = &filesystem.files[i];
        struct json_object* file_json = json_object_new_object();

        json_object_object_add(file_json, "inode", json_object_new_int(file->inode));
        json_object_object_add(file_json, "type", json_object_new_string(file->type));

        if (strcmp(file->type, "reg") == 0) {
            json_object_object_add(file_json, "data", json_object_new_string(file->data));
        } else if (strcmp(file->type, "dir") == 0) {
            struct json_object* entries_json = json_object_new_array();
            for (int j = 0; j < file->num_entries; j++) {
                struct file_entry* entry = &file->entries[j];
                struct json_object* entry_json = json_object_new_object();
                json_object_object_add(entry_json, "name", json_object_new_string(entry->name));
                json_object_object_add(entry_json, "inode", json_object_new_int(entry->inode));
                json_object_array_add(entries_json, entry_json);
            }
            json_object_object_add(file_json, "entries", entries_json);
        }

        json_object_array_add(fs_json, file_json);
    }

    FILE* file = fopen(filesystem.json_file, "w");
    if (file) {
        fprintf(file, "%s", json_object_to_json_string_ext(fs_json, JSON_C_TO_STRING_PRETTY));
        fclose(file);
    }

    json_object_put(fs_json);
}

static int getattr_callback(const char* path, struct stat* stbuf) {
  pthread_mutex_lock(&mutex);
    memset(stbuf, 0, sizeof(struct stat));

    if (strcmp(path, "/") == 0) {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
        pthread_mutex_unlock(&mutex);
        return 0;
    }

    for (int i = 0; i < filesystem.num_files; i++) {
        struct file* file = &filesystem.files[i];
        for (int j = 0; j < file->num_entries; j++) {
            if (strcmp(path, file->entries[j].name) == 0) {
                if (strcmp(file->type, "dir") == 0) {
                    stbuf->st_mode = S_IFDIR | 0755;
                    stbuf->st_nlink = 2;
                } else if (strcmp(file->type, "reg") == 0) {
                    stbuf->st_mode = S_IFREG | 0777;
                    stbuf->st_nlink = 1;
                    stbuf->st_size = strlen(file->data);
                }
                pthread_mutex_unlock(&mutex);
                return 0;
            }
        }
    }
  pthread_mutex_unlock(&mutex);
  return -ENOENT;
}

static int readdir_callback(const char* path, void* buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info* fi) {
    pthread_mutex_lock(&mutex);

    if (strcmp(path, "/") == 0) {
        filler(buf, ".", NULL, 0);
        filler(buf, "..", NULL, 0);

        for (int i = 0; i < filesystem.num_files; i++) {
            struct file* file = &filesystem.files[i];
            for (int j = 0; j < file->num_entries; j++) {
                struct file_entry* entry = &file->entries[j];
                filler(buf, entry->name, NULL, 0);
            }
        }

        pthread_mutex_unlock(&mutex);
        return 0;
    }

    pthread_mutex_unlock(&mutex);
    return -ENOENT;
}


static int open_callback(const char* path, struct fuse_file_info* fi) {
    return 0;
}

static int read_callback(const char* path, char* buf, size_t size, off_t offset, struct fuse_file_info* fi) {
    for (int i = 0; i < filesystem.num_files; i++) {
        struct file* file = &filesystem.files[i];
        if (strcmp(path, file->entries[i].name) == 0 && strcmp(file->type, "reg") == 0) {
            size_t len = strlen(file->data);
            if (offset >= len) {
                return 0;
            }
            if (offset + size > len) {
                memcpy(buf, file->data + offset, len - offset);
                return len - offset;
            }
            memcpy(buf, file->data + offset, size);
            return size;
        }
    }
    return -ENOENT;
}

static int write_callback(const char* path, const char* buf, size_t size, off_t offset, struct fuse_file_info* fi) {
    for (int i = 0; i < filesystem.num_files; i++) {
        struct file* file = &filesystem.files[i];
        if (strcmp(path, file->entries[i].name) == 0 && strcmp(file->type, "reg") == 0) {
            size_t len = strlen(file->data);
            if (offset >= len) {
                return 0;
            }
            if (offset + size > len) {
                memcpy(file->data + offset, buf, len - offset);
                file->data[len] = '\0';
                return len - offset;
            }
            memcpy(file->data + offset, buf, size);
            file->data[offset + size] = '\0';
            return size;
        }
    }
    return -ENOENT;
}

static int create_callback(const char* path, mode_t mode, struct fuse_file_info* fi) {
    if (filesystem.num_files >= MAX_ENTRIES) {
        return -ENOSPC; // No space left on device
    }

    struct file* new_file = &filesystem.files[filesystem.num_files++];
    new_file->inode = filesystem.num_files - 1;
    strcpy(new_file->type, "reg");
    new_file->data[0] = '\0';

    char* dir_name = strdup(path);
    char* base_name = basename(dir_name);

    struct file* parent_dir = &filesystem.files[0];
    strcpy(parent_dir->entries[parent_dir->num_entries].name, base_name);
    parent_dir->entries[parent_dir->num_entries++].inode = new_file->inode;

    free(dir_name);

    return 0;
}

static int mkdir_callback(const char* path, mode_t mode) {
    if (filesystem.num_files >= MAX_ENTRIES) {
        return -ENOSPC; // No space left on device
    }

    struct file* new_dir = &filesystem.files[filesystem.num_files++];
    new_dir->inode = filesystem.num_files - 1;
    strcpy(new_dir->type, "dir");
    new_dir->num_entries = 0;

    char* dir_name = strdup(path);
    char* base_name = basename(dir_name);

    struct file* parent_dir = &filesystem.files[0];
    strcpy(parent_dir->entries[parent_dir->num_entries].name, base_name);
    parent_dir->entries[parent_dir->num_entries++].inode = new_dir->inode;

    free(dir_name);

    return 0;
}

static int unlink_callback(const char* path) {
    for (int i = 0; i < filesystem.num_files; i++) {
        struct file* file = &filesystem.files[i];
        if (strcmp(path, file->entries[i].name) == 0) {
            if (strcmp(file->type, "dir") == 0) {
                return -EISDIR; // Is a directory
            }

            struct file* parent_dir = &filesystem.files[0];
            for (int j = 0; j < parent_dir->num_entries; j++) {
                if (strcmp(parent_dir->entries[j].name, path) == 0) {
                    for (int k = j; k < parent_dir->num_entries - 1; k++) {
                        parent_dir->entries[k] = parent_dir->entries[k + 1];
                    }
                    parent_dir->num_entries--;
                    break;
                }
            }

            for (int j = i; j < filesystem.num_files - 1; j++) {
                filesystem.files[j] = filesystem.files[j + 1];
            }
            filesystem.num_files--;

            return 0;
        }
    }
    return -ENOENT;
}

static int rmdir_callback(const char* path) {
    for (int i = 0; i < filesystem.num_files; i++) {
        struct file* file = &filesystem.files[i];
        if (strcmp(path, file->entries[i].name) == 0) {
            if (strcmp(file->type, "reg") == 0) {
                return -ENOTDIR; // Not a directory
            }

            struct file* parent_dir = &filesystem.files[0];
            for (int j = 0; j < parent_dir->num_entries; j++) {
                if (strcmp(parent_dir->entries[j].name, path) == 0) {
                    for (int k = j; k < parent_dir->num_entries - 1; k++) {
                        parent_dir->entries[k] = parent_dir->entries[k + 1];
                    }
                    parent_dir->num_entries--;
                    break;
                }
            }

            for (int j = i; j < filesystem.num_files - 1; j++) {
                filesystem.files[j] = filesystem.files[j + 1];
            }
            filesystem.num_files--;

            return 0;
        }
    }
    return -ENOENT;
}

static struct fuse_operations operations = {
    .getattr = getattr_callback,
    .readdir = readdir_callback,
    .open = open_callback,
    .read = read_callback,
    .write = write_callback,
    .create = create_callback,
    .mkdir = mkdir_callback,
    .unlink = unlink_callback,
    .rmdir = rmdir_callback,
};

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

int main(int argc, char* argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <json_file> <mount_point>\n", argv[0]);
        return 1;
    }

    load_filesystem(argv[1]);
    print_filesystem();

    int fuse_stat = fuse_main(argc - 1, argv + 1, &operations, NULL);

    save_filesystem();

    return fuse_stat;
}
