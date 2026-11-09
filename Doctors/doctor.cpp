#include "Doctor.h"

bool validateDoctor(const Doctor& doctor, string& errorMessage) {

    if (doctor.doctorID.empty()) {
        errorMessage = "Doctor ID cannot be empty.";
        return false;
    }

    if (doctor.doctorID.length() > 15) {
        errorMessage = "Doctor ID is too long.";
        return false;
    }

    if (doctor.doctorName.length() > 30) {
        errorMessage = "Doctor Name is too long.";
        return false;
    }

    if (doctor.specialty.length() > 20) {
        errorMessage = "Specialty is too long.";
        return false;
    }

    if (doctor.address.length() > 30) {
        errorMessage = "Address is too long.";
        return false;
    }

    errorMessage = "";
    return true;
}