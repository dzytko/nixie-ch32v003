#include "pid.h"

void pid_init(struct pid_pid *pid) {
    pid->integrator = 0.0f;
    pid->prev_error = 0.0f;

    pid->differentiator = 0.0f;
    pid->prev_measurement = 0.0f;

    pid->out = 0.0f;
}

float pid_update(struct pid_pid *pid, const float setpoint, const float measurement) {
    const float error = setpoint - measurement;
    const float proportional = pid->k_p * error;

    pid->integrator = pid->integrator + 0.5f * pid->k_i * pid->t * (error + pid->prev_error);

    if (pid->integrator > pid->lim_max_int) {
        pid->integrator = pid->lim_max_int;
    }
    else if (pid->integrator < pid->lim_min_int) {
        pid->integrator = pid->lim_min_int;
    }

    pid->differentiator = -(2.0f * pid->k_d * (measurement -pid->prev_measurement) +
                            (2.0f * pid->tau - pid->t) * pid->differentiator) /
                                    (2.0f * pid->tau + pid->t);
    pid->differentiator = 2.0f * pid->k_d * (measurement -pid->prev_measurement);
    pid->differentiator +=  (2.0f * pid->tau - pid->t) * pid->differentiator;
    pid->differentiator /= -(2.0f * pid->tau + pid->t);

    pid->out = proportional + pid->integrator + pid->differentiator;

    if (pid->out > pid->lim_max) {
        pid->out = pid->lim_max;
    }
    else if (pid->out < pid->lim_min) {
        pid->out = pid->lim_min;
    }

    pid->prev_error = error;
    pid->prev_measurement = measurement;

    return pid->out;
}