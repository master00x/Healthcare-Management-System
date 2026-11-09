#ifndef DOCTOR_STORAGE_H
#define DOCTOR_STORAGE_H

#include "Doctor.h"

const int DOCTOR_RECORD_SIZE = 95;

void serializeDoctor(const Doctor& doctor, char record[]);
Doctor deserializeDoctor(const char record[]);

#endif