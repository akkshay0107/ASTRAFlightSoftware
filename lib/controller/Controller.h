/*
Controller.h - Controller Header file
Description: Header file to Controller.cpp
Author: Vincent Palmerio
*/

#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <ArduinoEigenDense.h>
#include "../math/Integrator.h"


#define MODE_ARRAY_LENGTH (12)
#define K_ARRAY_LENGTH (10)

#define X_VECTOR_LENGTH (7)

//dimensions of controllerInputU
#define U_ROW_LENGTH (4)

//dimensions of kGain matrix
#define K_ROW_LENGTH (12)
#define K_COLUMN_LENGTH (12)

#define BETA_MAX (8)
#define BETA_MIN (-BETA_MAX)

#define GAMMA_MAX (8)
#define GAMMA_MIN (-GAMMA_MAX)

#define THROTTLE_MIN (0)
#define THROTTLE_MAX (1)

#define ALPHA_MAX (8)
#define ALPHA_MIN (-ALPHA_MAX)

#define INNER_GIMBAL_PIN (0)
#define OUTER_GIMBAL_PIN (0)
#define LEFT_TORQUE_VANE_PIN (0)
#define RIGHT_TORQUE_VANE_PIN (0)

#define INNER_GIMBAL_INITIAL_SETTING (100)
#define OUTER_GIMBAL_INITIAL_SETTING (100)
#define LEFT_TORQUE_VANE_INITIAL_SETTING (145)
#define RIGHT_TORQUE_VANE_INITIAL_SETTING (140)

//Global variables
extern Eigen::VectorXd controllerInputU;
extern double *k;
extern Eigen::MatrixXd kGain;
extern Eigen::VectorXd deltaX;

enum K_GAIN {
    TRACK_K_GAIN = 1,
    STABALIZE_K_GAIN = 1,
    LAND_K_GAIN = 1
};

extern Eigen::VectorXd xRef;

extern int initializeController();

extern int updateController();

int getDeltaX(Eigen::VectorXd*, Eigen::VectorXd*);
int controlLaw();
int controlLawRegulate();
int saturation();
int controlServos();
double minMax(double value, double min, double max);

int controlMode(Eigen::VectorXd* x, Eigen::VectorXd* xRef);

#endif



