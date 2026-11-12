#include "secondaryIndexManager.h"
#include "../PrimaryIndex/PrimaryIndex.h"
using namespace std;

vector<string> intersectSorted(const vector<string> &a, const vector<string> &b)
{
    vector<string> out;
    size_t i = 0, j = 0;
    while (i < a.size() && j < b.size())
    {
        if (a[i] == b[j])
        {
            out.push_back(a[i]);
            i++;
            j++;
        }
        else if (a[i] < b[j])
            i++;
        else
            j++;
    }
    return out;
}

SecondaryIndexManager::SecondaryIndexManager(const string &dir)
    : nameIndex(dir + "/DoctorNameIndex.idx", dir + "/DoctorNameIndex.lst", 30, 15),specialtyIndex(dir + "/DoctorSpecialtyIndex.idx", dir + "/DoctorSpecialtyIndex.lst", 20, 15),
        appointmentDoctorIndex(dir + "/AppointmentDoctorIndex.idx", dir + "/AppointmentDoctorIndex.lst", 15, 15)
{
}

bool SecondaryIndexManager::open(bool reset)
{
    bool ok = nameIndex.open(reset);
    ok = specialtyIndex.open(reset) && ok;
    ok = appointmentDoctorIndex.open(reset) && ok;
    return ok;
}

void SecondaryIndexManager::close()
{
    nameIndex.close();
    specialtyIndex.close();
    appointmentDoctorIndex.close();
}

bool SecondaryIndexManager::rebuild(const vector<DoctorKeys> &doctors,
                                    const vector<AppointmentKeys> &appointments)
{
    if (!open(true))
        return false;
    bool ok = true;
    for (const DoctorKeys &d : doctors)
        ok = onDoctorAdded(d.id, d.name, d.specialty) && ok;
    for (const AppointmentKeys &a : appointments)
        ok = onAppointmentAdded(a.id, a.doctorID) && ok;
    return ok;
}


bool SecondaryIndexManager::onDoctorAdded(const string &id, const string &name, const string &specialty)
{
    bool a = nameIndex.insert(name, id);
    bool b = specialtyIndex.insert(specialty, id);
    if (a && b)
        return true;
    if (a)
        nameIndex.remove(name, id);
    if (b)
        specialtyIndex.remove(specialty, id);
    return false;
}

bool SecondaryIndexManager::onDoctorDeleted(const string &id, const string &name, const string &specialty)
{
    bool a = nameIndex.remove(name, id);
    bool b = specialtyIndex.remove(specialty, id);
    return a && b;
}

bool SecondaryIndexManager::onDoctorNameUpdated(const string &id, const string &oldName,const string &newName)
{
    return nameIndex.updateKey(id, oldName, newName);
}

bool SecondaryIndexManager::onDoctorSpecialtyUpdated(const string &id, const string &oldSpecialty, const string &newSpecialty)
{
    return specialtyIndex.updateKey(id, oldSpecialty, newSpecialty);
}

bool SecondaryIndexManager::onDoctorIDUpdated(const string &oldID, const string &newID,const string &name, const string &specialty)
{
    bool a = nameIndex.updatePrimaryKey(name, oldID, newID);
    bool b = specialtyIndex.updatePrimaryKey(specialty, oldID, newID);
    bool c = true;
    if (!appointmentIDsByDoctor(oldID).empty())
        c = appointmentDoctorIndex.renameKey(oldID, newID);
    return a && b && c;
}


bool SecondaryIndexManager::onAppointmentAdded(const string &appointmentID,const string &doctorID)
{
    return appointmentDoctorIndex.insert(doctorID, appointmentID);
}

bool SecondaryIndexManager::onAppointmentDeleted(const string &appointmentID,const string &doctorID)
{
    return appointmentDoctorIndex.remove(doctorID, appointmentID);
}



vector<string> SecondaryIndexManager::doctorIDsByName(const string &name) const
{
    return nameIndex.search(name);
}

vector<string> SecondaryIndexManager::doctorIDsBySpecialty(const string &specialty) const
{
    return specialtyIndex.search(specialty);
}

vector<string> SecondaryIndexManager::appointmentIDsByDoctor(const string &doctorID) const
{
    return appointmentDoctorIndex.search(doctorID);
}

vector<string> SecondaryIndexManager::doctorIDsByNameAndSpecialty(
    const string &name, const string &specialty) const
{
    return intersectSorted(nameIndex.search(name), specialtyIndex.search(specialty));
}

void SecondaryIndexManager::printAll() const
{
    nameIndex.print("Doctor Name index");
    specialtyIndex.print("Doctor Specialty index");
    appointmentDoctorIndex.print("Appointment -> Doctor ID index");
}

// ---------------------------------------- lookups through the primary index ---

vector<int> SecondaryIndexManager::doctorOffsetsByName(const string &name) const
{
    return nameIndex.searchOffsets(name, PrimaryIndex::findDoctor);
}

vector<int> SecondaryIndexManager::doctorOffsetsBySpecialty(const string &specialty) const
{
    return specialtyIndex.searchOffsets(specialty, PrimaryIndex::findDoctor);
}

vector<int> SecondaryIndexManager::doctorOffsetsByNameAndSpecialty(
    const string &name, const string &specialty) const
{
    // intersect the two ID lists first, then ask the primary index for each offset
    vector<int> offsets;
    for (const string &id : doctorIDsByNameAndSpecialty(name, specialty))
    {
        int off = PrimaryIndex::findDoctor(id);
        if (off != -1)
            offsets.push_back(off);
    }
    return offsets;
}

vector<int> SecondaryIndexManager::appointmentOffsetsByDoctor(const string &doctorID) const
{
    return appointmentDoctorIndex.searchOffsets(doctorID, PrimaryIndex::findAppointment);
}
