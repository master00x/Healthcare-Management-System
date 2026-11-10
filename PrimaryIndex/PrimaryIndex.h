#ifndef PRIMARY_INDEX_H
#define PRIMARY_INDEX_H

#include <string>

// Shared interface for the Healthcare Management System.
// All positions are absolute byte offsets from the beginning of the .dat file,
// including its sizeof(int) header. -1 means "record not found".
// Functions are declarations only. Implement them in PrimaryIndex.cpp.
namespace PrimaryIndex {

// Initialize/load the two primary indexes from disk.
bool load();
// Persist both indexes to disk.
bool save();
// Recreate indexes by scanning active records in the data files.
bool rebuild();

// Doctor primary index: Doctor ID -> byte offset
int findDoctor(const std::string& doctorID);
bool insertDoctor(const std::string& doctorID, int recordOffset);
bool removeDoctor(const std::string& doctorID);
bool updateDoctorID(const std::string& oldID, const std::string& newID);

// Appointment primary index: Appointment ID -> byte offset
int findAppointment(const std::string& appointmentID);
bool insertAppointment(const std::string& appointmentID, int recordOffset);
bool removeAppointment(const std::string& appointmentID);

} // namespace PrimaryIndex

#endif // PRIMARY_INDEX_H
