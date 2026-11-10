#ifndef DOCTOR_H
#define DOCTOR_H

#include <string>
using namespace std;

struct Doctor {
    string doctorID;
    string doctorName;
    string specialty;
    string address;
};

bool validateDoctor(const Doctor& doctor, string& errorMessage);

#endif