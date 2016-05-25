#ifndef utils_h
#define utils_h

#include <libxml/parser.h>

void log_data(float sp, float temp, float heatlvl);
char** str_split(char* a_str, const char a_delim);
float liters_to_gallons(float liters);
float celc_to_fahr(float celc);
float kg_to_lbs(float kg);
xmlNode *find_node(xmlNode *searchNode, char * nodeName, int count);
#endif
