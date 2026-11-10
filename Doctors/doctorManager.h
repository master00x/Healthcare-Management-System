#ifndef DOCTOR_MANAGER_H
#define DOCTOR_MANAGER_H

#include "doctor.h"
#include <string>

using namespace std;

void initializeDoctorFile();

bool addDoctor(const Doctor& doctor, string& message);
bool displayDoctor(string doctorID);

bool updateDoctorName(string doctorID, string newName);
bool updateDoctorSpecialty(string doctorID, string newSpecialty);
bool updateDoctorAddress(string doctorID, string newAddress);

bool updateDoctorID(string oldID, string newID);

bool deleteDoctor(string doctorID);

#endif