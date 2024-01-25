set -x
gcc -Wall fuse-example.c -D_FILE_OFFSET_BITS=64 $(pkg-config fuse json-c --cflags --libs) -pthread -o fuse_example