#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "pid.h"


struct _pid warm,cold,*pid;
float process_point, set_point,dead_band;
float p_gain, i_gain, d_gain, integral_val,new_integ;;

/*------------------------------------------------------------------------
pid_init

DESCRIPTION This function initializes the pointers in the _pid structure
to the process variable and the setpoint. *pv and *sp are
integer pointers.
------------------------------------------------------------------------*/
void pid_init(struct _pid *warm, float process_point, float set_point)
{
struct _pid *pid;

pid = warm;
pid->pv = process_point;
pid->sp = set_point;
}


/*------------------------------------------------------------------------
pid_tune

DESCRIPTION Sets the proportional gain (p_gain), integral gain (i_gain),
derivitive gain (d_gain), and the dead band (dead_band) of
a pid control structure _pid.
------------------------------------------------------------------------*/

void pid_tune(struct _pid *pid, float p_gain, float i_gain, float d_gain, float dead_band)
{
pid->pgain = p_gain;
pid->igain = i_gain;
pid->dgain = d_gain;
pid->deadband = dead_band;
pid->integral= integral_val;
pid->last_error=0;
}

/*------------------------------------------------------------------------
pid_setinteg

DESCRIPTION Set a new value for the integral term of the pid equation.
This is useful for setting the initial output of the
pid controller at start up.
------------------------------------------------------------------------*/
void pid_setinteg(struct _pid *pid,float new_integ)
{
pid->integral = new_integ;
pid->last_error = 0;
}

/*------------------------------------------------------------------------
pid_bumpless

DESCRIPTION Bumpless transfer algorithim. When suddenly changing
setpoints, or when restarting the PID equation after an
extended pause, the derivative of the equation can cause
a bump in the controller output. This function will help
smooth out that bump. The process value in *pv should
be the updated just before this function is used.
------------------------------------------------------------------------*/
void pid_bumpless(struct _pid *pid)
{

pid->last_error = (pid->sp)-(pid->pv);

}

/*------------------------------------------------------------------------
pid_calc

DESCRIPTION Performs PID calculations for the _pid structure *a. This function uses the positional form of the pid equation, and incorporates an integral windup prevention algorithim. Rectangular integration is used, so this function must be repeated on a consistent time basis for accurate control.

RETURN VALUE The new output value for the pid loop.

USAGE #include "control.h"*/


float pid_calc(struct _pid *pid)
{
float err;
float pterm, dterm, result, ferror;

err = (float)(abs)(pid->sp) - (pid->pv);
if ((err) > pid->deadband)
{
ferror = (float) err; /*do integer to float conversion only once*/
pterm = pid->pgain * ferror;
if (pterm > 100 || pterm < -100)
pid->integral = 0.0;
else
{
pid->integral += pid->igain * ferror;
if (pid->integral > 100.0) pid->integral = 100.0;
else if (pid->integral < 0.0) pid->integral = 0.0;
}
dterm = ((float)(err - pid->last_error)) * pid->dgain;
result = pterm + pid->integral + dterm;
}
else result = pid->integral;
pid->last_error = err;
return (result);
}

