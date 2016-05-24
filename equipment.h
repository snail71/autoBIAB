#ifndef equipment_h
#define equipment_h

struct equipment
{
	float 			kettleSizeGallons;
	float 			grainAbsorption;
	float			evaporationPerHour;
	float 			trubLoss;	
	int				flowTicksPerGallon;
	int				serialPort;
};

bool load_equipment_file(char *equipFile, struct equipment *equip);

#endif
