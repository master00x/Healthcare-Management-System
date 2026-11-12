#include "doctorManager.h"
#include "doctorStorage.h"
#include "../PrimaryIndex/PrimaryIndex.h"
#include "../FileManagement/fileManager.h"

#include <iostream>
#include <fstream>
#include "../SecondaryIndex/secondaryIndexManager.h"
extern SecondaryIndexManager secIdx;

using namespace std;

const string DOCTOR_FILE = "Data/Doctors.dat";

// Initialize Doctors file
void initializeDoctorFile()
{

    ifstream checkFile(DOCTOR_FILE, ios::binary);

    if (checkFile)
    {
        checkFile.seekg(0, ios::end);

        if (checkFile.tellg() >= sizeof(int))
        {
            checkFile.close();
            return;
        }

        checkFile.close();
    }

    ofstream file(DOCTOR_FILE, ios::binary);

    int head = -1;

    file.write((char *)&head, sizeof(int));

    file.close();
}

// Find doctor and return its position
int findDoctor(string doctorID)
{
    return PrimaryIndex::findDoctor(doctorID);
}

// Add doctor
bool addDoctor(const Doctor &doctor, string &message)
{

    string errorMessage;

    if (!validateDoctor(doctor, errorMessage))
    {
        message = errorMessage;
        return false;
    }

    initializeDoctorFile();

    if (findDoctor(doctor.doctorID) != -1)
    {
        message = "Doctor ID already exists.";
        return false;
    }

    char record[DOCTOR_RECORD_SIZE];

    serializeDoctor(doctor, record);

    int offset = insertRecord(DOCTOR_FILE, record, DOCTOR_RECORD_SIZE);

    if (offset == -1)
    {
        message = "Error writing Doctor record.";
        return false;
    }

    if (!PrimaryIndex::insertDoctor(doctor.doctorID, offset))
    {
        message = "Error updating Doctor Primary Index.";
        return false;
    }

    if (!secIdx.onDoctorAdded(doctor.doctorID, doctor.doctorName, doctor.specialty))
    {
        message = "Error updating Doctor Secondary Index.";
        return false;
    }

    message = "Doctor added successfully.";
    return true;
}

// Display doctor
bool displayDoctor(string doctorID)
{

    int position = findDoctor(doctorID);

    if (position == -1)
    {
        cout << "Doctor not found." << endl;
        return false;
    }

    ifstream file(DOCTOR_FILE, ios::binary);

    if (!file)
        return false;

    char record[DOCTOR_RECORD_SIZE];

    file.seekg(position);

    file.read(record, DOCTOR_RECORD_SIZE);

    file.close();

    Doctor doctor = deserializeDoctor(record);

    cout << "\nDoctor Information" << endl;
    cout << "Doctor ID: " << doctor.doctorID << endl;
    cout << "Name: " << doctor.doctorName << endl;
    cout << "Specialty: " << doctor.specialty << endl;
    cout << "Address: " << doctor.address << endl;

    return true;
}

// Update doctor name
bool updateDoctorName(string doctorID, string newName)
{

    if (newName.length() > 30)
        return false;

    int position = findDoctor(doctorID);

    if (position == -1)
        return false;

    fstream file(DOCTOR_FILE, ios::in | ios::out | ios::binary);

    if (!file)
        return false;

    char record[DOCTOR_RECORD_SIZE];

    file.seekg(position);
    file.read(record, DOCTOR_RECORD_SIZE);

    Doctor doctor = deserializeDoctor(record);
    string oldName = doctor.doctorName;
    doctor.doctorName = newName;

    serializeDoctor(doctor, record);

    file.seekp(position);
    file.write(record, DOCTOR_RECORD_SIZE);

    file.close();

    secIdx.onDoctorNameUpdated(doctorID, oldName, newName);
    return true;
}

// Update doctor specialty
bool updateDoctorSpecialty(string doctorID, string newSpecialty)
{

    if (newSpecialty.length() > 20)
        return false;

    int position = findDoctor(doctorID);

    if (position == -1)
        return false;

    fstream file(DOCTOR_FILE, ios::in | ios::out | ios::binary);

    if (!file)
        return false;

    char record[DOCTOR_RECORD_SIZE];

    file.seekg(position);
    file.read(record, DOCTOR_RECORD_SIZE);

    Doctor doctor = deserializeDoctor(record);
    string oldSpecialty = doctor.specialty;

    doctor.specialty = newSpecialty;

    serializeDoctor(doctor, record);

    file.seekp(position);
    file.write(record, DOCTOR_RECORD_SIZE);

    file.close();
    secIdx.onDoctorSpecialtyUpdated(doctorID, oldSpecialty, newSpecialty);
    return true;
}

// Update doctor address
bool updateDoctorAddress(string doctorID, string newAddress)
{

    if (newAddress.length() > 30)
        return false;

    int position = findDoctor(doctorID);

    if (position == -1)
        return false;

    fstream file(DOCTOR_FILE, ios::in | ios::out | ios::binary);

    if (!file)
        return false;

    char record[DOCTOR_RECORD_SIZE];

    file.seekg(position);
    file.read(record, DOCTOR_RECORD_SIZE);

    Doctor doctor = deserializeDoctor(record);

    doctor.address = newAddress;

    serializeDoctor(doctor, record);

    file.seekp(position);
    file.write(record, DOCTOR_RECORD_SIZE);

    file.close();

    return true;
}

// Update doctor ID
bool updateDoctorID(string oldID, string newID)
{

    if (newID.empty() || newID.length() > 15)
        return false;

    int position = findDoctor(oldID);

    if (position == -1)
        return false;

    if (oldID != newID && findDoctor(newID) != -1)
        return false;

    fstream file(DOCTOR_FILE, ios::in | ios::out | ios::binary);

    if (!file)
        return false;

    char record[DOCTOR_RECORD_SIZE];

    file.seekg(position);
    file.read(record, DOCTOR_RECORD_SIZE);

    Doctor doctor = deserializeDoctor(record);

    doctor.doctorID = newID;

    serializeDoctor(doctor, record);

    file.seekp(position);
    file.write(record, DOCTOR_RECORD_SIZE);

    file.close();

    if (oldID != newID)
    {
        if (!PrimaryIndex::updateDoctorID(oldID, newID))
            return false;

        if (!secIdx.onDoctorIDUpdated(oldID, newID, doctor.doctorName, doctor.specialty))
            return false;
    }

    return true;
}

// Delete doctor

bool deleteDoctor(string doctorID){
    int position = PrimaryIndex::findDoctor(doctorID);

    if (position == -1)
        return false;

    char record[DOCTOR_RECORD_SIZE];

    // Read Doctor information before deleting
    if (!readRecord(DOCTOR_FILE, position, record, DOCTOR_RECORD_SIZE))
        return false;

    Doctor doctor = deserializeDoctor(record);

    // Delete from data file and update AVAIL LIST
    if (!deleteRecord(DOCTOR_FILE, position, DOCTOR_RECORD_SIZE,0))
        return false;

    // Remove Doctor ID from primary index
    if (!PrimaryIndex::removeDoctor(doctorID))
        return false;

    // Remove Doctor from secondary indexes
    if (!secIdx.onDoctorDeleted(doctor.doctorID, doctor.doctorName, doctor.specialty))
        return false;

    return true;
}

