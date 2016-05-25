#ifndef recipe_h
#define recipe_h
#include <time.h>
#include "equipment.h"


enum stepType
{	
	FILL,
	STRIKE,
	GRAININ,
	MASH,
	GRAINOUT,
	DRAIN,
	BOIL,
	FIRSTWORT,	
};


struct ingredient
{
	enum stepType 	use;
	char			description[255];
	float 			timeToAdd;
	float 			amount;
	bool			added;
	time_t			timeAdded;
};

struct brewStep
{
	enum stepType		type;
	char				description[255];
	float  				setpoint;
	float 				time;
	float 				duration;
	bool				atSetpoint;
	int 				countsNeeded;
	int 				countsCompleted;
	float 				volumeTotal;
	float 				volumeCompleted;
	int					ingredientCount;
	struct ingredient	ingredients[25];
	time_t				startTime;
	time_t				endTime;
	
};

struct recipe
{
	char 			name[255];
	float 			batch_size;
	float 			grainTemp;
	float 			grainWeight;
	
	//int				stepCount;
	//int				currentStep;
	int				mashStepCount;
	int				mashCurrentStep;
	struct brewStep steps[25];
};

bool load_recipe_file(char *recipeFile, struct recipe *rec, struct equipment *equip);

#endif
