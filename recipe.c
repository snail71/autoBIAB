#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#define __USE_C99_MATH
#include <stdbool.h>
#include <libxml/parser.h>
#include <unistd.h>
#include "recipe.h"
#include "utils.h"
#include "equipment.h"
#include "controller.h"


void create_fill_step( struct recipe *rec,struct equipment *equip)
{
	float losses = 0;
	float totalWater = 0;
	losses += rec->grainWeight * equip->grainAbsorption;
	
	losses += equip->trubLoss;
	
	losses += equip->evaporationPerHour * (rec->steps[CTL_BOIL].duration / 60.0);
	printf("Batch size: %3.2f\n",rec->batch_size);
	printf("Brew loss: %3.2f\n",losses);
	totalWater = losses + rec->batch_size;
	
	printf("Total water: %3.2f\n",totalWater);
	
	rec->steps[CTL_FILL].volumeTotal = totalWater;
	rec->steps[CTL_FILL].type = FILL;
	rec->steps[CTL_FILL].volumeCompleted = 0;
	rec->steps[CTL_FILL].countsNeeded = (int)(totalWater * (float)equip->flowTicksPerGallon);
	rec->steps[CTL_FILL].countsCompleted = 0;
}


bool load_recipe_file(char *recipeFile, struct recipe *rec,struct equipment *equip)
{
	int bumpHopCount;
	int itemsFound = 0;
	xmlDoc	*doc;
	xmlNode *root,*recipeNode,*node, *hopsNode, *hopNode,*hopElementNode,
		*fermentablesNode, *fermentableNode, *fermentableElement, *mashNode,
		*mashStepsNode, *mashStepNode, *mashElementNode;
	
	if(access(recipeFile,F_OK) == -1)
		return false;
	
	doc = xmlReadFile(recipeFile,NULL,0);
	root = xmlDocGetRootElement(doc);
	
	
	recipeNode = find_node(root,"RECIPE",0);
	if(recipeNode == NULL)
	{
		printf("Found no recipe node\n");
		return false;
	}
	
	node = find_node(recipeNode,"NAME",0);
	if(node == NULL)
	{
		printf("Found no recipe name\n");
		return false;
	}
	
	
	sprintf(rec->name,"%s",(char *)xmlNodeGetContent(node));
	printf("Recipe name: %s\n",rec->name);
	
	node = find_node(recipeNode,"BATCH_SIZE",0);
	if(node == NULL)
	{
		printf("Found no batch size\n");
		return false;
	}
	
	rec->batch_size = liters_to_gallons( atof((char *)xmlNodeGetContent(node)));
	
	printf("Batch size: %3.2f\n",rec->batch_size);
	
	node = find_node(recipeNode,"BOIL_TIME",0);
	if(node == NULL)
	{
		printf("Found no boil time\n");
		return false;
	}
	
	
	rec->steps[CTL_BOIL].type = BOIL;
	sprintf(rec->steps[CTL_BOIL].description,"BOIL");
	rec->steps[CTL_BOIL].duration = atof((char *)xmlNodeGetContent(node));
	rec->steps[CTL_BOIL].atSetpoint = false;
	
	hopsNode = find_node(recipeNode,"HOPS",0);
	if(hopsNode == NULL)
	{
		printf("Found no hops node\n");
		return false;
	}
	
	hopNode = hopsNode; //force not null for first loop
	while(hopNode != NULL)
	{
		bumpHopCount = 0;
		
		hopNode = find_node(hopsNode,"HOP",itemsFound);
		if(hopNode != NULL)
		{
			
			hopElementNode = find_node(hopNode,"NAME",0);
			if(hopElementNode == NULL)
			{
				printf("Found no hop NAME node\n");
				return false;
			}		
			sprintf(rec->steps[CTL_BOIL].ingredients[rec->steps[CTL_BOIL].ingredientCount].description,"%s",(char *)xmlNodeGetContent(hopElementNode));
		
			hopElementNode = find_node(hopNode,"AMOUNT",0);
			if(hopElementNode == NULL)
			{
				printf("Found no hop AMOUNT node\n");
				return false;
			}		
			rec->steps[CTL_BOIL].ingredients[rec->steps[CTL_BOIL].ingredientCount].amount	 = atof((char *)xmlNodeGetContent(hopElementNode));
		
			hopElementNode = find_node(hopNode,"USE",0);
			if(hopElementNode == NULL)
			{
				printf("Found no hop USE node\n");
				return false;
			}	
			if(strcmp((char *)xmlNodeGetContent(hopElementNode),"Boil") == 0)
			{
				rec->steps[CTL_BOIL].ingredients[rec->steps[CTL_BOIL].ingredientCount].use = BOIL;
				bumpHopCount = 1;
			}
			else if(strcmp((char *)xmlNodeGetContent(hopElementNode),"First Wort") == 0)
			{
				rec->steps[CTL_BOIL].ingredients[rec->steps[0].ingredientCount].use = FIRSTWORT;
				bumpHopCount = 1;
			}
		
			hopElementNode = find_node(hopNode,"TIME",0);
			if(hopElementNode == NULL)
			{
				printf("Found no hop TIME node\n");
				return false;
			}	
			rec->steps[CTL_BOIL].ingredients[rec->steps[CTL_BOIL].ingredientCount].timeToAdd = atof((char *)xmlNodeGetContent(hopElementNode));
			rec->steps[CTL_BOIL].ingredients[rec->steps[CTL_BOIL].ingredientCount].added = false;
		
			if(bumpHopCount)
			{
				printf("Found hop: %s [%f] @ [%f]\n",rec->steps[CTL_BOIL].ingredients[rec->steps[CTL_BOIL].ingredientCount].description,
					rec->steps[CTL_BOIL].ingredients[rec->steps[CTL_BOIL].ingredientCount].amount,
					rec->steps[CTL_BOIL].ingredients[rec->steps[CTL_BOIL].ingredientCount].timeToAdd);
			}
		
			rec->steps[CTL_BOIL].ingredientCount += bumpHopCount;
			itemsFound ++;
		}
	}
	
	fermentablesNode = find_node(recipeNode,"FERMENTABLES",0);
	if(fermentablesNode == NULL)
	{
		printf("Found no fermentables node\n");
		return false;
	}
		
	itemsFound = 0;
	rec->grainWeight = 0;
	fermentableNode = fermentablesNode; //force not null for first loop
	while(fermentableNode != NULL)
	{		
		fermentableNode = find_node(fermentablesNode,"FERMENTABLE",itemsFound);
		if(fermentableNode != NULL)
		{
			fermentableElement = find_node(fermentableNode,"NAME",0);
			if(fermentableElement != NULL)
			{
				printf("Fermentable: %s\n",(char *)xmlNodeGetContent(fermentableElement));
			}
			fermentableElement = find_node(fermentableNode,"AMOUNT",0);
			if(fermentableElement == NULL)
			{
				printf("Found no AMOUNT for fermentable node\n");
				return false;	
			}
				
			rec->grainWeight += kg_to_lbs(atof((char *)xmlNodeGetContent(fermentableElement)));
		}
		itemsFound ++;
	}
	
	printf("Grain wt: %3.2f lbs\n",rec->grainWeight);
	
	create_fill_step(rec,equip);
	
	
	
	mashNode = find_node(recipeNode,"MASH",0);
	if(mashNode == NULL)
	{
		printf("Found no MASH node\n");
		return false;
	}
	
	mashElementNode = find_node(mashNode,"GRAIN_TEMP",0);
	if(mashElementNode == NULL)
	{
		printf("Found no GRAIN_TEMP node\n");
		return false;	
	}
	rec->grainTemp = celc_to_fahr(atof((char *)xmlNodeGetContent(mashElementNode)));
	printf("Grain temp: %3.2f\n",rec->grainTemp);
	
	mashStepsNode = find_node(mashNode,"MASH_STEPS",0);
	if(mashStepsNode == NULL)
	{
		printf("Found no MASH_STEPS node\n");
		return false;
	}
	
	mashStepNode = mashStepsNode; //force not null for first loop
	itemsFound = 0;
	while(mashStepNode != NULL)
	{		
		mashStepNode = find_node(mashStepsNode,"MASH_STEP",itemsFound);
		if(mashStepNode != NULL)
		{
			rec->steps[CTL_MASH + itemsFound].type = MASH;
			mashElementNode = find_node(mashStepNode,"NAME",0);
			if(mashElementNode != NULL)
			{
				
				sprintf(rec->steps[CTL_MASH + itemsFound].description,"%s",(char *)xmlNodeGetContent(mashElementNode));
			}
			
			mashElementNode = find_node(mashStepNode,"STEP_TEMP",0);
			if(mashElementNode != NULL)
			{
				
				rec->steps[CTL_MASH + itemsFound].setpoint = celc_to_fahr( atof( (char *)xmlNodeGetContent(mashElementNode)));
			}
			
			mashElementNode = find_node(mashStepNode,"STEP_TIME",0);
			if(mashElementNode != NULL)
			{
				
				rec->steps[CTL_MASH + itemsFound].time = atof( (char *)xmlNodeGetContent(mashElementNode));
			}
			rec->mashCurrentStep = 0;
			rec->mashStepCount ++;
			
			if(itemsFound == 0)
			{
				rec->steps[CTL_STRIKE].type = STRIKE;
				rec->steps[CTL_STRIKE].setpoint = rec->steps[CTL_MASH + itemsFound].setpoint;
				sprintf(rec->steps[CTL_STRIKE].description,"%s","Strike");
				rec->steps[CTL_GRAININ].type = GRAININ;
				rec->steps[CTL_GRAININ].setpoint = rec->steps[CTL_MASH + itemsFound].setpoint;
				sprintf(rec->steps[CTL_GRAININ].description,"%s","Grain in");
			}
		}
		itemsFound ++;
	}
	
	// TODO: parse mash steps
	
	//equipmentNode = find_node(recipeNode,"EQUIPMENT",0);
	//if(equipmentNode == NULL)
	//{
	//	printf("Found no EQUIPMENT node\n");
	//	return false;
	//}
	
	//equipmentElementNode = find_node(equipmentNode,"TRUB_CHILLER_LOSS",0);
	//if(equipmentElementNode == NULL)
	//{
	//	printf("Found no TRUB_CHILLER_LOSS element\n");
	//	return false;
	//}
	//rec->trubLoss = liters_to_gallons( atof((char *)xmlNodeGetContent(equipmentElementNode)));
	//printf("Trub loss: %3.2f\n",rec->trubLoss );
	
	//equipmentElementNode = find_node(equipmentNode,"EVAP_RATE",0);
	//if(equipmentElementNode == NULL)
	//{
	//	printf("Found no EVAP_RATE element\n");
	//	return false;
	//}
	//rec->evaporationRate = liters_to_gallons( atof((char *)xmlNodeGetContent(equipmentElementNode)));
	//printf("Evap rate: %3.2f\n",rec->evaporationRate);
	
	//equipmentElementNode = find_node(equipmentNode,"ABSORPTION",0);
	//if(equipmentElementNode == NULL)
	//{
	//	printf("Found no ABSORPTION element\n");
	//	return false;
	//}
	//rec->grainAbsorption = atof((char *)xmlNodeGetContent(equipmentElementNode));
	//printf("Absorption: %3.2f\n",rec->grainAbsorption);
	
	return true;
}
