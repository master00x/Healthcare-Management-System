#include "../Doctors/DoctorStorage.h"
#include <iostream>

using namespace std;

int main() {

    Doctor doctor = {
        "D001",
        "Ahmed Mohamed",
        "Cardiology",
        "Cairo, Egypt"
    };

    string errorMessage;

    if (!validateDoctor(doctor, errorMessage)) {
        cout << errorMessage << endl;
        return 1;
    }

    cout << "Doctor validation passed!" << endl;

    char record[DOCTOR_RECORD_SIZE];

    serializeDoctor(doctor, record);

    cout << "Record size: " << sizeof(record) << " bytes" << endl;

    Doctor restoredDoctor = deserializeDoctor(record);

    cout << "\nRestored Doctor:" << endl;
    cout << "Doctor ID: " << restoredDoctor.doctorID << endl;
    cout << "Name: " << restoredDoctor.doctorName << endl;
    cout << "Specialty: " << restoredDoctor.specialty << endl;
    cout << "Address: " << restoredDoctor.address << endl;

    if (restoredDoctor.doctorID == doctor.doctorID &&
        restoredDoctor.doctorName == doctor.doctorName &&
        restoredDoctor.specialty == doctor.specialty &&
        restoredDoctor.address == doctor.address) {

        cout << "\nSerialization test PASSED!" << endl;
    }
    else {
        cout << "\nSerialization test FAILED!" << endl;
        return 1;
    }

    return 0;
}