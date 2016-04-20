#ifndef _sysfs_helper
#define _sysfs_helper

int read_sysfs_file(const char* filepath, char * buffer, int buflen);
size_t list_dir(const char *path, char ***ls);

#endif
