#ifndef pid_H
#define pid_H

struct _pid {
float pv; /*integer that contains the process value*/
float sp; /*integer that contains the set point*/
float integral;
float pgain;
float igain;
float dgain;
float deadband;
float last_error;
};

void pid_init(struct _pid *warm, float process_point, float set_point);
void pid_tune(struct _pid *pid, float p_gain, float i_gain, float d_gain, float dead_band);
void pid_setinteg(struct _pid *pid,float new_integ);
void pid_bumpless(struct _pid *pid);
float pid_calc(struct _pid *pid);

#endif
