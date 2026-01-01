#ifndef HAL_PID_H
#define HAL_PID_H

struct pid_pid {
    float k_p;
    float k_i;
    float k_d;

    float tau;

    float lim_min;
    float lim_max;

    float lim_min_int;
    float lim_max_int;

    float t;

    float integrator;
    float prev_error;
    float differentiator;
    float prev_measurement;

    float out;

};

void pid_init(struct pid_pid *pid);

float pid_update(struct pid_pid *pid, float setpoint, float measurement);

#endif //HAL_PID_H
