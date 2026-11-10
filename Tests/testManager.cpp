#include "../Doctors/doctorManager.h"
#include <iostream>

using namespace std;

int main() {

    Doctor doctor = {
        "D999",
        "Ahmed Mohamed",
        "Cardiology",
        "Cairo, Egypt"
    };

    string message;

    // Test Add
    if (addDoctor(doctor, message))
        cout << message << endl;
    else
        cout << message << endl;

    // Test Display
    displayDoctor("D999");

    // Test Update Name
    if (updateDoctorName("D999", "Ahmed Ali"))
        cout << "\nName updated successfully." << endl;
    else
        cout << "\nName update failed." << endl;

    // Test Update Specialty
    if (updateDoctorSpecialty("D999", "Surgery"))
        cout << "Specialty updated successfully." << endl;
    else
        cout << "Specialty update failed." << endl;

    // Test Update Address
    if (updateDoctorAddress("D999", "Giza, Egypt"))
        cout << "Address updated successfully." << endl;
    else
        cout << "Address update failed." << endl;

    // Test Update ID
    if (updateDoctorID("D999", "D100"))
        cout << "ID updated successfully." << endl;
    else
        cout << "ID update failed." << endl;

    // Display after updates
    displayDoctor("D100");

    // Test Delete
    if (deleteDoctor("D100"))
        cout << "\nDoctor deleted successfully." << endl;
    else
        cout << "\nDelete failed." << endl;

    // Try displaying deleted doctor
    displayDoctor("D100");

    return 0;
}