#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <assert.h>
#include <sys/types.h>
#define __USE_C99_MATH
#include <stdbool.h>
#include <time.h>
#include <libxml/parser.h>
#include <unistd.h>
#include "utils.h"
#include "equipment.h"

bool load_equipment_file(char *equipFile, struct equipment *equip)
{
	xmlDoc	*doc;
	xmlNode *root,*equipmentElementNode;	
	
	if(access(equipFile,F_OK) == -1)
		return false;
	
	doc = xmlReadFile(equipFile,NULL,0);
	root = xmlDocGetRootElement(doc);
	
	equipmentElementNode = find_node(root,"KETTLE_VOLUME",0);
	if(equipmentElementNode == NULL)
	{
		printf("Found no KETTLE_VOLUME node\n");
		return false;
	}
	equip->kettleSizeGallons = atof((char *)xmlNodeGetContent(equipmentElementNode));
	
	equipmentElementNode = find_node(root,"GRAIN_ABSORPTION",0);
	if(equipmentElementNode == NULL)
	{
		printf("Found no GRAIN_ABSORPTION node\n");
		return false;
	}
	equip->grainAbsorption = atof((char *)xmlNodeGetContent(equipmentElementNode));
	
	equipmentElementNode = find_node(root,"EVAPORATION",0);
	if(equipmentElementNode == NULL)
	{
		printf("Found no EVAPORATION node\n");
		return false;
	}
	equip->evaporationPerHour = atof((char *)xmlNodeGetContent(equipmentElementNode));
	
	equipmentElementNode = find_node(root,"TRUB_LOSS",0);
	if(equipmentElementNode == NULL)
	{
		printf("Found no TRUB_LOSS node\n");
		return false;
	}
	equip->trubLoss = atof((char *)xmlNodeGetContent(equipmentElementNode));
	
	equipmentElementNode = find_node(root,"FLOW_TICKS_PER_GAL",0);
	if(equipmentElementNode == NULL)
	{
		printf("Found no FLOW_TICKS_PER_GAL node\n");
		return false;
	}
	equip->flowTicksPerGallon = atoi((char *)xmlNodeGetContent(equipmentElementNode));
	
	equipmentElementNode = find_node(root,"SERIAL_PORT",0);
	if(equipmentElementNode == NULL)
	{
		printf("Found no SERIAL_PORT node\n");
		return false;
	}
	equip->serialPort = atoi((char *)xmlNodeGetContent(equipmentElementNode));
	
	return true;
}
