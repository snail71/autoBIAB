#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#define __USE_C99_MATH
#include <stdbool.h>
#include <libxml/parser.h>
#include <unistd.h>
#include "recipe.h"
#include "utils.h"

//xmlNode *find_node(xmlNode *searchNode, char * nodeName)
//{
//	xmlNode *first_child, *node;
//	first_child = searchNode->children;
//	for(node=first_child; node; node = node->next)
//	{
//		if(strcmp(nodeName,(char *)node->name)==0)
//			return node;
//	}
//	
//	return NULL;
//}

xmlNode *find_node(xmlNode *searchNode, char * nodeName, int count)
{
	int found = 0;
	xmlNode *first_child, *node;
	first_child = searchNode->children;
	for(node=first_child; node; node = node->next)
	{
		if(strcmp(nodeName,(char *)node->name)==0)
		{
			if(found == count)
				return node;
			else 
				found ++;
		}
	}
	
	return NULL;
}

bool load_recipe_file(char *recipeFile, struct recipe *rec)
{
	int bumpHopCount;
	int itemsFound = 0;
	xmlDoc	*doc;
	xmlNode *root,*recipeNode,*node, *hopsNode, *hopNode,*hopElementNode,*fermentablesNode, *fermentableNode, *fermentableElement, *mashStepsNode, *mashStepNode, *mashElementNode;
	
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
	
	rec->stepCount = 1;
	rec->currentStep = 0;
	rec->steps[0].type = BOIL;
	sprintf(rec->steps[0].description,"BOIL");
	rec->steps[0].duration = atof((char *)xmlNodeGetContent(node));
	rec->steps[0].atSetpoint = false;
	
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
			sprintf(rec->steps[0].ingredients[rec->steps[0].ingredientCount].description,"%s",(char *)xmlNodeGetContent(hopElementNode));
		
			hopElementNode = find_node(hopNode,"AMOUNT",0);
			if(hopElementNode == NULL)
			{
				printf("Found no hop AMOUNT node\n");
				return false;
			}		
			rec->steps[0].ingredients[rec->steps[0].ingredientCount].amount	 = atof((char *)xmlNodeGetContent(hopElementNode));
		
			hopElementNode = find_node(hopNode,"USE",0);
			if(hopElementNode == NULL)
			{
				printf("Found no hop USE node\n");
				return false;
			}	
			if(strcmp((char *)xmlNodeGetContent(hopElementNode),"Boil") == 0)
			{
				rec->steps[0].ingredients[rec->steps[0].ingredientCount].use = BOIL;
				bumpHopCount = 1;
			}
			else if(strcmp((char *)xmlNodeGetContent(hopElementNode),"First Wort") == 0)
			{
				rec->steps[0].ingredients[rec->steps[0].ingredientCount].use = FIRSTWORT;
				bumpHopCount = 1;
			}
		
			hopElementNode = find_node(hopNode,"TIME",0);
			if(hopElementNode == NULL)
			{
				printf("Found no hop TIME node\n");
				return false;
			}	
			rec->steps[0].ingredients[rec->steps[0].ingredientCount].timeToAdd = atof((char *)xmlNodeGetContent(hopElementNode));
			rec->steps[0].ingredients[rec->steps[0].ingredientCount].added = false;
		
			if(bumpHopCount)
			{
				printf("Found hop: %s [%f] @ [%f]\n",rec->steps[0].ingredients[rec->steps[0].ingredientCount].description,
					rec->steps[0].ingredients[rec->steps[0].ingredientCount].amount,
					rec->steps[0].ingredients[rec->steps[0].ingredientCount].timeToAdd);
			}
		
			rec->steps[0].ingredientCount += bumpHopCount;
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
				
			rec->grainWeight += atof((char *)xmlNodeGetContent(fermentableElement));
		}
		itemsFound ++;
	}
	
	printf("Grain wt: %3.2f\n",rec->grainWeight);
	
	
	return true;
}
