#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>

int read_sysfs_file(const char* filepath, char * buffer, int buflen)
{
	FILE *file;
	file = fopen(filepath,"r");
	if(file != NULL)
	{
		size_t bytes = fread(buffer,1,buflen,file);
		fclose(file);
		return bytes;
	}
	return -1;
}

size_t list_dir(const char *path, char ***ls)
{
	size_t count = 0;
	
	DIR *dp = NULL;
	struct dirent *ep = NULL;
	dp = opendir(path);
	if(dp == NULL)
	{
		printf("Error opening path [%s]/n",path);
		return 0;
	}
	*ls = NULL;
	ep = readdir(dp);
	while(ep != NULL)
	{
		count ++;
		ep = readdir(dp);
	}
	
	rewinddir(dp);
	*ls = calloc(count,sizeof(char *));
	count = 0;
	ep = readdir(dp);
	while(ep != NULL)
	{
		if(strcmp(ep->d_name,".") != 0 && strcmp(ep->d_name,"..") != 0)  
		    (*ls)[count++] = strdup(ep->d_name);
		ep = readdir(dp);
	}
	
	closedir(dp);
	return count;
}



