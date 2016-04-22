#ifndef pid_H
#define pid_H

struct _pid {
int pv; /*integer that contains the process value*/
int sp; /*integer that contains the set point*/
float integral;
float pgain;
float igain;
float dgain;
int deadband;
int last_error;
};

void pid_init(_pid *warm, int process_point, int set_point);
void pid_tune(_pid *pid, float p_gain, float i_gain, float d_gain, int dead_band);
void pid_setinteg(_pid *pid,float new_integ);
void pid_bumpless(_pid *pid);
float pid_calc(_pid *pid);

#endif