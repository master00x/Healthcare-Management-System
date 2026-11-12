#include "AppointmentManager.h"
#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include "../SecondaryIndex/secondaryIndexManager.h"
#include "../PrimaryIndex/PrimaryIndex.h"
#include "../FileManagement/fileManager.h"
extern SecondaryIndexManager secIdx;
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
    return PrimaryIndex::findDoctor(id) != -1;
}

bool appointmentExists(string id)
{
    return PrimaryIndex::findAppointment(id) != -1;
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

    int offset = insertRecord("Data/Appointments.dat", reinterpret_cast<char *>(&app), APPPOINTMENT_SIZE);

    if (offset == -1){
        cout << "Error writing Appointment!\n";
        return;
    }

    if (!PrimaryIndex::insertAppointment(id, offset)){
        cout << "Error updating Appointment Primary Index!\n";
        return;
    }

    if (!secIdx.onAppointmentAdded(id, doctorID)){
        cout << "Error updating Appointment Secondary Index!\n";
        return;
    }

    cout << "Appointment added successfully!\n";
}

void displayAppointment(){
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


void updateAppointmentDate(){
    string id, newDate;

    cout << "Enter Appointment ID: ";
    getline(cin, id);

    cout << "Enter New Appointment Date: ";
    getline(cin, newDate);

    if (id.empty() || newDate.empty() || id.length() > 15 || newDate.length() > 30){
        cout << "Invalid input!\n";
        return;
    }

    int offset = PrimaryIndex::findAppointment(id);

    if (offset == -1){
        cout << "Appointment not found!\n";
        return;
    }

    fstream file("Data/Appointments.dat", ios::in | ios::out | ios::binary);

    if (!file){
        cout << "Cannot open Appointments.dat!\n";
        return;
    }

    Appointment app;

    file.seekg(offset, ios::beg);

    if (!file.read(reinterpret_cast<char*>(&app), sizeof(app))){
        cout << "Error reading Appointment!\n";
        return;
    }

    setField(app.appointmentDate, 30, newDate);

    file.seekp(offset, ios::beg);

    file.write(reinterpret_cast<char*>(&app), sizeof(app));

    if (!file){
        cout << "Error updating Appointment!\n";
        return;
    }

    cout << "Appointment updated successfully!\n";
}



void deleteAppointment(){
    string id;

    cout << "Enter Appointment ID: ";
    getline(cin, id);

    int offset = PrimaryIndex::findAppointment(id);

    if (offset == -1){
        cout << "Appointment not found!\n";
        return;
    }

    char record[APPPOINTMENT_SIZE];

    if (!readRecord("Data/Appointments.dat", offset, record, APPPOINTMENT_SIZE)){
        cout << "Error reading Appointment!\n";
        return;
    }

    // Doctor ID begins at byte 45
    string doctorID(record + 45, 15);

    while (!doctorID.empty() && doctorID.back() == ' '){
        doctorID.pop_back();
    }

    // Appointment ID begins at byte 30
    if (!deleteRecord("Data/Appointments.dat", offset, APPPOINTMENT_SIZE,30)){
        cout << "Error deleting Appointment!\n";
        return;
    }

    if (!PrimaryIndex::removeAppointment(id)){
        cout << "Error updating Appointment Primary Index!\n";
        return;
    }

    if (!secIdx.onAppointmentDeleted(id, doctorID)){
        cout << "Error updating Appointment Secondary Index!\n";
        return;
    }

    cout << "Appointment deleted successfully!\n";
}
