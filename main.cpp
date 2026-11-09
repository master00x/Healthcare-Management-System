
#include <iostream>
#include <limits>
#include "Appointment/AppointmentManager.h"

using namespace std;

int main()
{
    int choice;

    do
    {
        cout << "\n===== Healthcare Management System =====\n";
        cout << "1. Add Appointment\n";
        cout << "2. Display Appointments\n";
        cout << "3. Update Appointment Date\n";
        cout << "4. Delete Appointment\n";
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
            cout << "Exiting...\n";
            break;

        default:
            cout << "Invalid choice!\n";
        }

    } while (choice != 0);

    return 0;
}
