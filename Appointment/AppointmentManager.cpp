
#include "AppointmentManager.h"
#include <iostream>
#include <fstream>
#include <cstring>
#include <string>

using namespace std;

bool setField(char *field, int size, const string &value)
{
    if (value.length() > size)
        return false;

    memset(field, ' ', size);
    memcpy(field, value.c_str(), value.length());

    return true;
}

bool doctorExists(string id)
{
    ifstream file("Data/Doctors.dat", ios::binary);

    if (!file)
        return false;

    char record[95];
    char searchID[15];

    memset(searchID, ' ', 15);
    memcpy(searchID, id.c_str(), id.length());

    file.seekg(sizeof(int), ios::beg);

    while (file.read(record, 95))
    {
        if (record[0] == '*')
            continue;

        if (memcmp(record, searchID, 15) == 0)
        {
            file.close();
            return true;
        }
    }

    file.close();
    return false;
}

bool appointmentExists(string id)
{
    ifstream file("Data/Appointments.dat", ios::binary);

    if (!file)
        return false;

    char record[60];
    char searchID[15];

    memset(searchID, ' ', 15);
    memcpy(searchID, id.c_str(), id.length());

    file.seekg(sizeof(int), ios::beg);

    while (file.read(record, 60))
    {
        if (record[0] == '*')
            continue;

        if (memcmp(record, searchID, 15) == 0)
        {
            file.close();
            return true;
        }
    }

    file.close();
    return false;
}

void addAppointment()
{
    Appointment app;

    string id, date, doctorID;

    cout << "\nEnter Appointment ID: ";
    getline(cin, id);

    cout << "Enter Appointment Date: ";
    getline(cin, date);

    cout << "Enter Doctor ID: ";
    getline(cin, doctorID);

    cout << "\n--- Input Verification ---\n";
    cout << "Appointment ID: [" << id << "]\n";
    cout << "Appointment Date: [" << date << "]\n";
    cout << "Doctor ID: [" << doctorID << "]\n";

    if (id.empty() || date.empty() || doctorID.empty())
    {
        cout << "Error: Fields cannot be empty!\n";
        return;
    }

    if (id.length() > 15 ||
        date.length() > 30 ||
        doctorID.length() > 15)
    {
        cout << "Error: Field size exceeded!\n";
        return;
    }

    if (appointmentExists(id))
    {
        cout << "Error: Appointment already exists!\n";
        return;
    }

    if (!doctorExists(doctorID))
    {
        cout << "Error: Doctor ID does not exist!\n";
        return;
    }

    setField(app.appointmentID, 15, id);
    setField(app.appointmentDate, 30, date);
    setField(app.DoctorID, 15, doctorID);

    fstream file(
        "Data/Appointments.dat",
        ios::in | ios::out | ios::binary);

    if (!file)
    {
        cout << "Error opening Appointments.dat!\n";
        return;
    }

    file.seekp(0, ios::end);

    file.write(
        reinterpret_cast<char *>(&app),
        sizeof(Appointment));

    if (!file)
        cout << "Error writing Appointment!\n";
    else
        cout << "Appointment added successfully!\n";

    file.close();
}
void displayAppointment()
{
    ifstream file("Data/Appointments.dat", ios::binary);

    if (!file)
    {
        cout << "Error opening Appointments.dat!\n";
        return;
    }

    Appointment app;
    bool found = false;

    file.seekg(sizeof(int), ios::beg);

    while (file.read(reinterpret_cast<char *>(&app), sizeof(Appointment)))
    {
        if (app.appointmentID[0] == '*')
            continue;

        found = true;

        cout << "\nAppointment ID: ";
        cout.write(app.appointmentID, 15);

        cout << "\nAppointment Date: ";
        cout.write(app.appointmentDate, 30);

        cout << "\nDoctor ID: ";
        cout.write(app.DoctorID, 15);

        cout << "\n-------------------------\n";
    }

    if (!found)
        cout << "No appointments found!\n";

    file.close();
}

void updateAppointmentDate()
{
    string id, newDate;

    cout << "\nEnter Appointment ID: ";
    getline(cin, id);

    cout << "Enter New Appointment Date: ";
    getline(cin, newDate);

    if (id.empty() || newDate.empty())
    {
        cout << "Error: Fields cannot be empty!\n";
        return;
    }

    if (id.length() > 15 || newDate.length() > 30)
    {
        cout << "Error: Field size exceeded!\n";
        return;
    }

    char searchID[15];

    memset(searchID, ' ', 15);
    memcpy(searchID, id.c_str(), id.length());

    fstream file(
        "Data/Appointments.dat",
        ios::in | ios::out | ios::binary);

    if (!file)
    {
        cout << "Error opening Appointments.dat!\n";
        return;
    }

    Appointment app;

    file.seekg(sizeof(int), ios::beg);

    while (true)
    {
        streampos recordPosition = file.tellg();

        if (!file.read(reinterpret_cast<char *>(&app), sizeof(Appointment)))
            break;

        if (app.appointmentID[0] == '*')
            continue;

        if (memcmp(app.appointmentID, searchID, 15) == 0)
        {
            setField(app.appointmentDate, 30, newDate);

            file.clear();
            file.seekp(recordPosition);

            file.write(
                reinterpret_cast<char *>(&app),
                sizeof(Appointment));

            if (!file)
                cout << "Error updating Appointment!\n";
            else
                cout << "Appointment updated successfully!\n";

            file.close();
            return;
        }
    }

    cout << "Appointment not found!\n";

    file.close();
}

void deleteAppointment()
{
    string id;

    cout << "\nEnter Appointment ID: ";
    getline(cin, id);

    if (id.empty())
    {
        cout << "Error: Appointment ID cannot be empty!\n";
        return;
    }

    if (id.length() > 15)
    {
        cout << "Error: Appointment ID size exceeded!\n";
        return;
    }

    char searchID[15];

    memset(searchID, ' ', 15);
    memcpy(searchID, id.c_str(), id.length());

    fstream file(
        "Data/Appointments.dat",
        ios::in | ios::out | ios::binary);

    if (!file)
    {
        cout << "Error opening Appointments.dat!\n";
        return;
    }

    Appointment app;

    file.seekg(sizeof(int), ios::beg);

    while (true)
    {
        streampos recordPosition = file.tellg();

        if (!file.read(reinterpret_cast<char *>(&app), sizeof(Appointment)))
            break;

        if (app.appointmentID[0] == '*')
            continue;

        if (memcmp(app.appointmentID, searchID, 15) == 0)
        {
            char deletedMarker = '*';

            file.clear();
            file.seekp(recordPosition);

            file.write(&deletedMarker, sizeof(char));

            if (!file)
                cout << "Error deleting Appointment!\n";
            else
                cout << "Appointment deleted successfully!\n";

            file.close();
            return;
        }
    }

    cout << "Appointment not found!\n";

    file.close();
}
