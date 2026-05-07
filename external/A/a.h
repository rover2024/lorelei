#pragma once

#ifdef __cplusplus
extern "C" {
#endif

double sqrt_A(double x);
double exp_A(double x, double y);
double log_A(double x, double y, double z);
double cos_A(double x, double y, double z, double w);
double sin_A(double x, double y, double z, double w, double v);
double tan_A(double x, double y, double z, double w, double v, double u);

void consume(double x);

void set_times(int times);

#ifdef __cplusplus
}
#endif
