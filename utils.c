#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <assert.h>
#include <sys/types.h>
#define __USE_C99_MATH
#include <stdbool.h>
#include <time.h>

float kg_to_lbs(float kg)
{
	return kg * 2.2046;
}

float liters_to_gallons(float liters)
{
	return liters * .264172;
}

void log_data(float sp, float temp, float heatlvl)
{
	FILE *fp;
	
	char dt[255];
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	sprintf(dt,"%d-%d-%d %d:%d:%d",tm.tm_year + 1900,tm.tm_mon + 1, tm.tm_mday,tm.tm_hour,tm.tm_min,tm.tm_sec);	
	
	fp = fopen("session.log","a");
	fprintf(fp,"%s,%3.2f,%3.2f,%3.2f\n",dt,sp,temp,heatlvl);
	fclose(fp);
}

char** str_split(char* a_str, const char a_delim)
{
    char** result    = 0;
    size_t count     = 0;
    char* tmp        = a_str;
    char* last_comma = 0;
    char delim[2];
    delim[0] = a_delim;
    delim[1] = 0;

    /* Count how many elements will be extracted. */
    while (*tmp)
    {
        if (a_delim == *tmp)
        {
            count++;
            last_comma = tmp;
        }
        tmp++;
    }

    /* Add space for trailing token. */
    count += last_comma < (a_str + strlen(a_str) - 1);

    /* Add space for terminating null string so caller
       knows where the list of returned strings ends. */
    count++;

    result = malloc(sizeof(char*) * count);

    if (result)
    {
        size_t idx  = 0;
        char* token = strtok(a_str, delim);

        while (token)
        {
            assert(idx < count);
            *(result + idx++) = strdup(token);
            token = strtok(0, delim);
        }
        assert(idx == count - 1);
        *(result + idx) = 0;
    }

    return result;
}

