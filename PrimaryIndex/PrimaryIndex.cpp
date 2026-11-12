#include "PrimaryIndex.h"

#include <algorithm>
#include <cstring>
#include <fstream>
#include <limits>
#include <string>
#include <vector>

namespace PrimaryIndex {
namespace {

// Matches the fixed-width fields specified by the IS321 assignment.
const int KEY_SIZE = 15;
const int DOCTOR_RECORD_SIZE = 95;
const int APPOINTMENT_RECORD_SIZE = 60;
const int APPOINTMENT_ID_POSITION = 30;
const int DATA_HEADER_SIZE = static_cast<int>(sizeof(int));
const char* const DOCTOR_DATA = "Data/Doctors.dat";
const char* const APPOINTMENT_DATA = "Data/Appointments.dat";
const char* const DOCTOR_INDEX = "Data/DoctorPrimary.idx";
const char* const APPOINTMENT_INDEX = "Data/AppointmentPrimary.idx";

static_assert(sizeof(int) == 4, "This index file format requires four-byte int values");

struct Entry {
    std::string key;
    int offset;
};

std::vector<Entry> doctors;
std::vector<Entry> appointments;
bool initialized = false;

bool validKey(const std::string& key) {
    return !key.empty() && key.size() <= static_cast<size_t>(KEY_SIZE) &&
           key.find('\0') == std::string::npos;
}

std::string decodeKey(const char* field) {
    int length = KEY_SIZE;
    while (length > 0 && (field[length - 1] == ' ' || field[length - 1] == '\0'))
        --length;
    return std::string(field, static_cast<size_t>(length));
}

// Binary search: returns the first index position where key may occur.
size_t lowerBound(const std::vector<Entry>& index, const std::string& key) {
    size_t low = 0;
    size_t high = index.size();
    while (low < high) {
        size_t mid = low + (high - low) / 2;
        if (index[mid].key < key)
            low = mid + 1;
        else
            high = mid;
    }
    return low;
}

int search(const std::vector<Entry>& index, const std::string& key) {
    const size_t pos = lowerBound(index, key);
    if (pos < index.size() && index[pos].key == key)
        return index[pos].offset;
    return -1;
}

bool readIndexFile(const char* path, std::vector<Entry>& target) {
    std::ifstream file(path, std::ios::binary);
    if (!file)
        return false;

    char magic[4];
    int count = 0;
    if (!file.read(magic, sizeof(magic)) ||
        std::memcmp(magic, "PI01", sizeof(magic)) != 0 ||
        !file.read(reinterpret_cast<char*>(&count), sizeof(count)) || count < 0)
        return false;

    // Bound the count by actual file length so a corrupt header cannot allocate huge memory.
    file.seekg(0, std::ios::end);
    const std::streamoff length = file.tellg();
    const std::streamoff expected = 8 + static_cast<std::streamoff>(count) * (KEY_SIZE + sizeof(int));
    if (length != expected)
        return false;
    file.seekg(8, std::ios::beg);

    std::vector<Entry> loaded;
    for (int i = 0; i < count; ++i) {
        char field[KEY_SIZE];
        int offset = -1;
        if (!file.read(field, KEY_SIZE) ||
            !file.read(reinterpret_cast<char*>(&offset), sizeof(offset)))
            return false;
        Entry entry = {decodeKey(field), offset};
        if (!validKey(entry.key) || offset < DATA_HEADER_SIZE ||
            (!loaded.empty() && loaded.back().key >= entry.key))
            return false;
        loaded.push_back(entry);
    }
    target.swap(loaded);
    return true;
}

bool writeIndexFile(const char* path, const std::vector<Entry>& index) {
    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    if (!file)
        return false;
    const int count = static_cast<int>(index.size());
    file.write("PI01", 4);
    file.write(reinterpret_cast<const char*>(&count), sizeof(count));
    for (size_t i = 0; i < index.size(); ++i) {
        char field[KEY_SIZE];
        std::memset(field, ' ', sizeof(field));
        std::memcpy(field, index[i].key.data(), index[i].key.size());
        file.write(field, sizeof(field));
        file.write(reinterpret_cast<const char*>(&index[i].offset), sizeof(int));
    }
    file.flush();
    return file.good();
}

bool scanDataFile(const char* path, int recordSize, int keyPos,
                  std::vector<Entry>& target) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        // Data file may not exist yet (before the first record is added).
        target.clear();
        return true;
    }
    file.seekg(0, std::ios::end);
    std::streamoff size = file.tellg();
    if (size < DATA_HEADER_SIZE || (size - DATA_HEADER_SIZE) % recordSize != 0)
        return false;
    file.seekg(DATA_HEADER_SIZE, std::ios::beg);

    std::vector<Entry> scanned;
    std::vector<char> record(static_cast<size_t>(recordSize));
    for (std::streamoff pos = DATA_HEADER_SIZE; pos < size; pos += recordSize) {
        if (!file.read(&record[0], recordSize))
            return false;
        if (record[static_cast<size_t>(keyPos)] == '*')
            continue;
        Entry item = {decodeKey(&record[static_cast<size_t>(keyPos)]), static_cast<int>(pos)};
        if (!validKey(item.key) || search(scanned, item.key) != -1)
            return false;
        const size_t idx = lowerBound(scanned, item.key);
        scanned.insert(scanned.begin() + static_cast<std::ptrdiff_t>(idx), item);
    }
    target.swap(scanned);
    return true;
}

bool ready() {
    return initialized || load();
}

bool insert(std::vector<Entry>& index, const std::string& key, int offset) {
    if (!ready() || !validKey(key) || offset < DATA_HEADER_SIZE)
        return false;
    size_t pos = lowerBound(index, key);
    if (pos < index.size() && index[pos].key == key)
        return false; // Duplicate primary key.
    std::vector<Entry> previous = index;
    Entry entry = {key, offset};
    index.insert(index.begin() + static_cast<std::ptrdiff_t>(pos), entry);
    if (save())
        return true;
    index.swap(previous);
    return false;
}

bool remove(std::vector<Entry>& index, const std::string& key) {
    if (!ready())
        return false;
    const size_t pos = lowerBound(index, key);
    if (pos == index.size() || index[pos].key != key)
        return false;
    std::vector<Entry> previous = index;
    index.erase(index.begin() + static_cast<std::ptrdiff_t>(pos));
    if (save())
        return true;
    index.swap(previous);
    return false;
}

} // anonymous namespace

bool save() {
    if (!initialized)
        return false;
    // Both files must exist in the Data directory. A failed write is reported.
    return writeIndexFile(DOCTOR_INDEX, doctors) &&
           writeIndexFile(APPOINTMENT_INDEX, appointments);
}

bool rebuild() {
    std::vector<Entry> newDoctors;
    std::vector<Entry> newAppointments;
    if (!scanDataFile(DOCTOR_DATA, DOCTOR_RECORD_SIZE, 0, newDoctors) ||
        !scanDataFile(APPOINTMENT_DATA, APPOINTMENT_RECORD_SIZE,
                      APPOINTMENT_ID_POSITION, newAppointments))
        return false;
    doctors.swap(newDoctors);
    appointments.swap(newAppointments);
    initialized = true;
    return save();
}

bool load() {
    if (initialized)
        return true;
    std::vector<Entry> newDoctors;
    std::vector<Entry> newAppointments;
    if (readIndexFile(DOCTOR_INDEX, newDoctors) &&
        readIndexFile(APPOINTMENT_INDEX, newAppointments)) {
        doctors.swap(newDoctors);
        appointments.swap(newAppointments);
        initialized = true;
        return true;
    }
    return rebuild(); // If index files don't exist, scan the data files.
}

int findDoctor(const std::string& doctorID) {
    if (!ready()) return -1;
    return search(doctors, doctorID);
}

bool insertDoctor(const std::string& doctorID, int recordOffset) {
    return insert(doctors, doctorID, recordOffset);
}

bool removeDoctor(const std::string& doctorID) {
    return remove(doctors, doctorID);
}

bool updateDoctorID(const std::string& oldID, const std::string& newID) {
    if (!ready() || !validKey(newID))
        return false;
    const size_t oldPos = lowerBound(doctors, oldID);
    if (oldPos == doctors.size() || doctors[oldPos].key != oldID)
        return false;
    if (oldID == newID)
        return true;
    if (search(doctors, newID) != -1)
        return false;

    std::vector<Entry> previous = doctors;
    const int offset = doctors[oldPos].offset;
    doctors.erase(doctors.begin() + static_cast<std::ptrdiff_t>(oldPos));
    const size_t newPos = lowerBound(doctors, newID);
    Entry updated = {newID, offset};
    doctors.insert(doctors.begin() + static_cast<std::ptrdiff_t>(newPos), updated);
    if (save())
        return true;
    doctors.swap(previous);
    return false;
}

int findAppointment(const std::string& appointmentID) {
    if (!ready()) return -1;
    return search(appointments, appointmentID);
}

bool insertAppointment(const std::string& appointmentID, int recordOffset) {
    return insert(appointments, appointmentID, recordOffset);
}

bool removeAppointment(const std::string& appointmentID) {
    return remove(appointments, appointmentID);
}

} // namespace PrimaryIndex
