set -x
rm -R mnt
mkdir mnt

#./fuse_example ./mnt
./fuse_example fs.json -d -f "./mnt"
