
#include <iostream>
#include <limits>
#include "Appointment/AppointmentManager.h"
#include "Doctors/doctorManager.h"

using namespace std;

void doctorMenu()
{
    int choice;

    do
    {
        cout << "\n===== Doctor Management =====\n";
        cout << "1. Add Doctor\n";
        cout << "2. Display Doctor\n";
        cout << "3. Update Doctor Name\n";
        cout << "4. Update Doctor Specialty\n";
        cout << "5. Update Doctor Address\n";
        cout << "6. Update Doctor ID\n";
        cout << "7. Delete Doctor\n";
        cout << "0. Back to Main Menu\n";
        cout << "Enter your choice: ";

        if (!(cin >> choice))
        {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input!\n";
            continue;
        }

        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        string id, value, newID, message;
        Doctor doctor;

        switch (choice)
        {
        case 1:
            cout << "Enter Doctor ID: ";
            getline(cin, doctor.doctorID);

            cout << "Enter Doctor Name: ";
            getline(cin, doctor.doctorName);

            cout << "Enter Specialty: ";
            getline(cin, doctor.specialty);

            cout << "Enter Address: ";
            getline(cin, doctor.address);

            if (addDoctor(doctor, message))
                cout << message << endl;
            else
                cout << message << endl;
            break;

        case 2:
            cout << "Enter Doctor ID: ";
            getline(cin, id);
            if (!displayDoctor(id))
                cout << "Doctor not found!\n";
            break;

        case 3:
            cout << "Enter Doctor ID: ";
            getline(cin, id);
            cout << "Enter New Name: ";
            getline(cin, value);
            cout << (updateDoctorName(id, value) ? "Doctor updated successfully!\n" : "Update failed!\n");
            break;

        case 4:
            cout << "Enter Doctor ID: ";
            getline(cin, id);
            cout << "Enter New Specialty: ";
            getline(cin, value);
            cout << (updateDoctorSpecialty(id, value) ? "Doctor updated successfully!\n" : "Update failed!\n");
            break;

        case 5:
            cout << "Enter Doctor ID: ";
            getline(cin, id);
            cout << "Enter New Address: ";
            getline(cin, value);
            cout << (updateDoctorAddress(id, value) ? "Doctor updated successfully!\n" : "Update failed!\n");
            break;

        case 6:
            cout << "Enter Old Doctor ID: ";
            getline(cin, id);
            cout << "Enter New Doctor ID: ";
            getline(cin, newID);
            cout << (updateDoctorID(id, newID) ? "Doctor ID updated successfully!\n" : "Update failed!\n");
            break;

        case 7:
            cout << "Enter Doctor ID: ";
            getline(cin, id);
            cout << (deleteDoctor(id) ? "Doctor deleted successfully!\n" : "Delete failed!\n");
            break;

        case 0:
            break;

        default:
            cout << "Invalid choice!\n";
        }

    } while (choice != 0);
}

void appointmentMenu()
{
    int choice;

    do
    {
        cout << "\n===== Appointment Management =====\n";
        cout << "1. Add Appointment\n";
        cout << "2. Display Appointments\n";
        cout << "3. Update Appointment Date\n";
        cout << "4. Delete Appointment\n";
        cout << "0. Back to Main Menu\n";
        cout << "Enter your choice: ";

        if (!(cin >> choice))
        {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input!\n";
            continue;
        }

        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        switch (choice)
        {
        case 1:
            addAppointment();
            break;
        case 2:
            displayAppointment();
            break;
        case 3:
            updateAppointmentDate();
            break;
        case 4:
            deleteAppointment();
            break;
        case 0:
            break;
        default:
            cout << "Invalid choice!\n";
        }

    } while (choice != 0);
}

int main()
{
    initializeDoctorFile();

    int choice;

    do
    {
        cout << "\n===== Healthcare Management System =====\n";
        cout << "1. Doctor Management\n";
        cout << "2. Appointment Management\n";
        cout << "0. Exit\n";
        cout << "Enter your choice: ";

        if (!(cin >> choice))
        {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input!\n";
            continue;
        }

        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        switch (choice)
        {
        case 1:
            doctorMenu();
            break;
        case 2:
            appointmentMenu();
            break;
        case 0:
            cout << "Exiting...\n";
            break;
        default:
            cout << "Invalid choice!\n";
        }

    } while (choice != 0);

    return 0;
}
