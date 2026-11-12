#ifndef SECONDARY_INDEX_MANAGER_H
#define SECONDARY_INDEX_MANAGER_H

#include <string>
#include <vector>
#include "secondaryIndex.h"

using namespace std;

struct DoctorKeys
{
    string id, name, specialty;
};
struct AppointmentKeys
{
    string id, doctorID;
};

class SecondaryIndexManager
{
public:
    SecondaryIndexManager(const string &dataDir = "Data");

    bool open(bool reset = false);
    void close();


    bool rebuild(const vector<DoctorKeys> &doctors,const vector<AppointmentKeys> &appointments);


    bool onDoctorAdded(const string &id, const string &name, const string &specialty);
    bool onDoctorDeleted(const string &id, const string &name, const string &specialty);
    bool onDoctorNameUpdated(const string &id, const string &oldName, const string &newName);
    bool onDoctorSpecialtyUpdated(const string &id, const string &oldSpecialty,const string &newSpecialty);

    bool onDoctorIDUpdated(const string &oldID, const string &newID,const string &name, const string &specialty);

    bool onAppointmentAdded(const string &appointmentID, const string &doctorID);
    bool onAppointmentDeleted(const string &appointmentID, const string &doctorID);

    vector<string> doctorIDsByName(const string &name) const;
    vector<string> doctorIDsBySpecialty(const string &specialty) const;
    vector<string> appointmentIDsByDoctor(const string &doctorID) const;
    vector<string> doctorIDsByNameAndSpecialty(const string &name,const string &specialty) const;
    vector<int> doctorOffsetsByName(const string &name) const;
    vector<int> doctorOffsetsBySpecialty(const string &specialty) const;
    vector<int> doctorOffsetsByNameAndSpecialty(const string &name, const string &specialty) const;
    vector<int> appointmentOffsetsByDoctor(const string &doctorID) const;

    void printAll() const;

private:
    SecondaryIndex nameIndex;
    SecondaryIndex specialtyIndex;
    SecondaryIndex appointmentDoctorIndex;
};

vector<string> intersectSorted(const vector<string> &a, const vector<string> &b);

#endif
