gcc -Wall -g -o autoBIAB autoBIAB.c sysfs_helper.c pid.c `pkg-config --cflags --libs gtk+-3.0`
