#ifndef controller_h
#define controller_h

#include "equipment.h"
#include "recipe.h"

enum brew_state_t
{
	BREW_STOPPED,
	BREW_RUNNING,
	BREW_PAUSED,
}brew_state;


typedef void (*brewstatecb_t)(enum brew_state_t);
typedef void (*fillcb_t) (float, float); 
typedef void (*tempcb_t) (float);
typedef void (*idlecb_t)();

struct brew_callbacks_t
{
	brewstatecb_t   statecb;
	idlecb_t		idlecb;
	tempcb_t		tempcb;
	fillcb_t		fillcb;
	
};

enum control_state
{
	CTL_IDLE = -1,
	CTL_FILL,
	CTL_STRIKE,
	CTL_GRAININ,
	CTL_MASH1,
	CTL_MASH2,
	CTL_MASH3,
	CTL_MASH4,
	CTL_GRAINOUT,
	CTL_DRAIN,
	CTL_BOIL,	
	
}g_MachineState;

void stop_brew();
void pause_brew();
void start_brew(struct brew_callbacks_t *brewCB, struct recipe *rec, struct equipment *equip);

#endif
