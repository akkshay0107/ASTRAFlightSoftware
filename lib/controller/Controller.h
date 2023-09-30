/*
Controller.h - Controller Header file
Description: Header file to Controller.cpp
Author: Ani Gokhale, Vincent Palmerio
Created: 9/13/2023
Last updated: 9/13/2023
*/


#include "../estimator/Estimator.h";


#define U_ARRAY_LENGTH 12
#define MODE_ARRAY_LENGTH 12
#define K_ARRAY_LENGTH 10

#define NO_ERROR_CODE = 0
#define GENERAL_ERROR_CODE = -1
#define MEMORY_ALLOCATION_ERROR_CODE = -2

//Global variables
float controllerInput_u[U_ARRAY_LENGTH];
float mode[MODE_ARRAY_LENGTH];
float k[K_ARRAY_LENGTH];

//malloc's global variables in this header file, 
    //returns error code as int (NO_ERROR_CODE, GENERAL_ERROR_CODE, MEMORY_ALLOCATION_ERROR_CODE)
int setup();

//control law function that:
    //takes in estimatedState_x (from Estimator.h)
    //takes in k
    //takes in mode
    //modifies controllerInput_u
    //returns error code
int controlLaw_u();

//calculates mode
    //takes in estimatedState_x
    //modifies mode
    //returns error code
int controlMode_mode();





