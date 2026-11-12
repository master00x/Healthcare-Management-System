#include "queryEngine.h"

#include "../Appointment/Appointment.h"
#include "../Doctors/doctorStorage.h"
#include "../PrimaryIndex/PrimaryIndex.h"
#include "../SecondaryIndex/secondaryIndexManager.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <fstream>
#include <vector>

namespace
{
bool isWordCharacter(char value)
{
    unsigned char character = static_cast<unsigned char>(value);
    return std::isalnum(character) != 0 || value == '_';
}

string lowercase(const string &value)
{
    string result = value;
    transform(result.begin(), result.end(), result.begin(), [](unsigned char character) {
        return static_cast<char>(std::tolower(character));
    });
    return result;
}

size_t findKeyword(const string &lowerQuery, const string &keyword, size_t start)
{
    size_t position = start;
    while ((position = lowerQuery.find(keyword, position)) != string::npos)
    {
        const bool leftBoundary = position == 0 || !isWordCharacter(lowerQuery[position - 1]);
        const size_t afterKeyword = position + keyword.size();
        const bool rightBoundary = afterKeyword == lowerQuery.size() ||
                                   !isWordCharacter(lowerQuery[afterKeyword]);
        if (leftBoundary && rightBoundary)
            return position;
        ++position;
    }
    return string::npos;
}

string trim(string value)
{
    const string whitespace = " \t\n\r";
    size_t first = value.find_first_not_of(whitespace);
    if (first == string::npos)
        return "";
    size_t last = value.find_last_not_of(whitespace);
    value = value.substr(first, last - first + 1);

    if (!value.empty() && value.back() == ';')
    {
        value.pop_back();
        value = trim(value);
    }
    if (value.size() >= 2 &&
        ((value.front() == '\'' && value.back() == '\'') ||
         (value.front() == '"' && value.back() == '"')))
        value = value.substr(1, value.size() - 2);
    return value;
}

size_t findEquals(const string &condition)
{
    char quote = '\0';
    for (size_t i = 0; i < condition.size(); ++i)
    {
        if (quote != '\0')
        {
            if (condition[i] == quote)
                quote = '\0';
        }
        else if (condition[i] == '\'' || condition[i] == '"')
            quote = condition[i];
        else if (condition[i] == '=')
            return i;
    }
    return string::npos;
}

vector<string> splitConditions(const string &conditions)
{
    vector<string> parts;
    string lowerConditions = lowercase(conditions);
    char quote = '\0';
    size_t partStart = 0;
    for (size_t i = 0; i < conditions.size(); ++i)
    {
        if (quote != '\0')
        {
            if (conditions[i] == quote)
                quote = '\0';
            continue;
        }
        if (conditions[i] == '\'' || conditions[i] == '"')
        {
            quote = conditions[i];
            continue;
        }
        if (i + 3 <= conditions.size() &&
            lowerConditions.compare(i, 3, "and") == 0 &&
            (i == 0 || !isWordCharacter(conditions[i - 1])) &&
            (i + 3 == conditions.size() || !isWordCharacter(conditions[i + 3])))
        {
            parts.push_back(conditions.substr(partStart, i - partStart));
            partStart = i + 3;
            i += 2;
        }
    }
    parts.push_back(conditions.substr(partStart));
    return parts;
}

string normalizeName(const string &value)
{
    string result;
    for (unsigned char character : value)
    {
        if (std::isalnum(character) != 0)
            result += static_cast<char>(std::tolower(character));
    }
    return result;
}

bool isDoctorTable(const string &table)
{
    const string name = normalizeName(table);
    return name == "doctor" || name == "doctors";
}

bool isAppointmentTable(const string &table)
{
    const string name = normalizeName(table);
    return name == "appointment" || name == "appointments";
}

int fieldIndex(bool doctorTable, const string &field)
{
    const string name = normalizeName(field);
    if (doctorTable)
    {
        if (name == "doctorid" || name == "id")
            return 0;
        if (name == "doctorname" || name == "name")
            return 1;
        if (name == "specialty")
            return 2;
        if (name == "address")
            return 3;
    }
    else
    {
        if (name == "appointmentid" || name == "id")
            return 0;
        if (name == "appointmentdate" || name == "date")
            return 1;
        if (name == "doctorid")
            return 2;
    }
    return -1;
}

string readField(const char *data, size_t length)
{
    size_t first = 0;
    while (first < length && (data[first] == ' ' || data[first] == '\0'))
        ++first;
    size_t last = length;
    while (last > first && (data[last - 1] == ' ' || data[last - 1] == '\0'))
        --last;
    return string(data + first, last - first);
}

bool scanOffsets(const string &path, int recordSize, int markerOffset,
                 vector<int> &offsets, string &error)
{
    ifstream file(path, ios::binary);
    if (!file)
    {
        error = "Cannot open " + path + ".";
        return false;
    }

    file.seekg(0, ios::end);
    const streamoff fileSize = file.tellg();
    if (fileSize < static_cast<streamoff>(sizeof(int)) ||
        (fileSize - sizeof(int)) % recordSize != 0)
    {
        error = "The data file has an invalid size: " + path + ".";
        return false;
    }

    vector<char> record(static_cast<size_t>(recordSize));
    for (streamoff offset = sizeof(int); offset < fileSize; offset += recordSize)
    {
        file.seekg(offset, ios::beg);
        if (!file.read(record.data(), recordSize))
        {
            error = "Failed to read " + path + ".";
            return false;
        }
        if (record[static_cast<size_t>(markerOffset)] != '*')
            offsets.push_back(static_cast<int>(offset));
    }
    return true;
}

bool readRecordAt(ifstream &file, int offset, int recordSize,
                  vector<char> &record, string &error, const string &path)
{
    if (offset < static_cast<int>(sizeof(int)))
    {
        error = "The index contains an invalid record offset for " + path + ".";
        return false;
    }
    file.clear();
    file.seekg(offset, ios::beg);
    if (!file.read(record.data(), recordSize))
    {
        error = "Failed to read a record from " + path + ".";
        return false;
    }
    return true;
}
} // namespace

bool parseQuery(const string &query, ParsedQuery &result)
{
    result.selectedField.clear();
    result.tableName.clear();
    result.conditions.clear();

    const string lowerQuery = lowercase(query);
    const size_t selectPos = findKeyword(lowerQuery, "select", 0);
    if (selectPos == string::npos || !trim(query.substr(0, selectPos)).empty())
        return false;

    const size_t selectEnd = selectPos + 6;
    const size_t fromPos = findKeyword(lowerQuery, "from", selectEnd);
    if (fromPos == string::npos)
        return false;

    result.selectedField = trim(query.substr(selectEnd, fromPos - selectEnd));
    if (result.selectedField.empty())
        return false;

    const size_t tableStart = fromPos + 4;
    const size_t wherePos = findKeyword(lowerQuery, "where", tableStart);
    const size_t tableEnd = wherePos == string::npos ? query.size() : wherePos;
    result.tableName = trim(query.substr(tableStart, tableEnd - tableStart));
    if (result.tableName.empty())
        return false;

    if (wherePos == string::npos)
        return true;

    const string conditions = query.substr(wherePos + 5);
    if (trim(conditions).empty())
        return false;

    const vector<string> parts = splitConditions(conditions);
    for (const string &part : parts)
    {
        const size_t equalPos = findEquals(part);
        if (equalPos == string::npos)
            return false;

        Condition condition;
        condition.field = trim(part.substr(0, equalPos));
        condition.value = trim(part.substr(equalPos + 1));
        if (condition.field.empty() || condition.value.empty())
            return false;
        result.conditions.push_back(condition);
    }
    return true;
}

bool executeQuery(const ParsedQuery &query, const SecondaryIndexManager &indexes,
                  ostream &output, string &error)
{
    error.clear();
    const bool doctorTable = isDoctorTable(query.tableName);
    const bool appointmentTable = isAppointmentTable(query.tableName);
    if (!doctorTable && !appointmentTable)
    {
        error = "Unknown table '" + query.tableName + "'. Use Doctors or Appointments.";
        return false;
    }

    const bool isDoctor = doctorTable;
    const int selectedIndex = query.selectedField == "*" ? -1 :
                              fieldIndex(isDoctor, query.selectedField);
    if (query.selectedField != "*" && selectedIndex == -1)
    {
        error = "Unknown selected field '" + query.selectedField + "'.";
        return false;
    }

    vector<int> conditionIndexes;
    for (const Condition &condition : query.conditions)
    {
        const int index = fieldIndex(isDoctor, condition.field);
        if (index == -1)
        {
            error = "Unknown condition field '" + condition.field + "'.";
            return false;
        }
        conditionIndexes.push_back(index);
    }

    const string path = isDoctor ? "Data/Doctors.dat" : "Data/Appointments.dat";
    const int recordSize = isDoctor ? DOCTOR_RECORD_SIZE : APPPOINTMENT_SIZE;
    const int markerOffset = isDoctor ? 0 : 30;
    vector<int> offsets;
    bool hasIndexedCandidate = false;

    if (isDoctor)
    {
        string name;
        string specialty;
        string doctorID;
        for (size_t i = 0; i < query.conditions.size(); ++i)
        {
            if (conditionIndexes[i] == 0 && doctorID.empty())
                doctorID = query.conditions[i].value;
            else if (conditionIndexes[i] == 1 && name.empty())
                name = query.conditions[i].value;
            else if (conditionIndexes[i] == 2 && specialty.empty())
                specialty = query.conditions[i].value;
        }

        if (!name.empty() && !specialty.empty())
        {
            offsets = indexes.doctorOffsetsByNameAndSpecialty(name, specialty);
            hasIndexedCandidate = true;
        }
        else if (!name.empty())
        {
            offsets = indexes.doctorOffsetsByName(name);
            hasIndexedCandidate = true;
        }
        else if (!specialty.empty())
        {
            offsets = indexes.doctorOffsetsBySpecialty(specialty);
            hasIndexedCandidate = true;
        }
        else if (!doctorID.empty())
        {
            const int offset = PrimaryIndex::findDoctor(doctorID);
            if (offset != -1)
                offsets.push_back(offset);
            hasIndexedCandidate = true;
        }
    }
    else
    {
        string appointmentID;
        string doctorID;
        for (size_t i = 0; i < query.conditions.size(); ++i)
        {
            if (conditionIndexes[i] == 0 && appointmentID.empty())
                appointmentID = query.conditions[i].value;
            else if (conditionIndexes[i] == 2 && doctorID.empty())
                doctorID = query.conditions[i].value;
        }
        if (!appointmentID.empty())
        {
            const int offset = PrimaryIndex::findAppointment(appointmentID);
            if (offset != -1)
                offsets.push_back(offset);
            hasIndexedCandidate = true;
        }
        else if (!doctorID.empty())
        {
            offsets = indexes.appointmentOffsetsByDoctor(doctorID);
            hasIndexedCandidate = true;
        }
    }

    if (!hasIndexedCandidate &&
        !scanOffsets(path, recordSize, markerOffset, offsets, error))
        return false;

    ifstream file(path, ios::binary);
    if (!file)
    {
        error = "Cannot open " + path + ".";
        return false;
    }

    vector<char> record(static_cast<size_t>(recordSize));
    size_t matchCount = 0;
    for (int offset : offsets)
    {
        if (!readRecordAt(file, offset, recordSize, record, error, path))
            return false;
        if (record[static_cast<size_t>(markerOffset)] == '*')
            continue;

        vector<string> values;
        if (isDoctor)
        {
            Doctor doctor = deserializeDoctor(record.data());
            values = {doctor.doctorID, doctor.doctorName, doctor.specialty, doctor.address};
        }
        else
        {
            values = {
                readField(record.data() + 30, 15),
                readField(record.data(), 30),
                readField(record.data() + 45, 15)};
        }

        bool matches = true;
        for (size_t i = 0; i < query.conditions.size(); ++i)
        {
            if (values[static_cast<size_t>(conditionIndexes[i])] != query.conditions[i].value)
            {
                matches = false;
                break;
            }
        }
        if (!matches)
            continue;

        if (matchCount > 0)
            output << '\n';
        if (query.selectedField == "*")
        {
            if (isDoctor)
            {
                output << "Doctor ID: " << values[0] << " | Name: " << values[1]
                       << " | Specialty: " << values[2] << " | Address: " << values[3];
            }
            else
            {
                output << "Appointment ID: " << values[0] << " | Date: " << values[1]
                       << " | Doctor ID: " << values[2];
            }
        }
        else
            output << query.selectedField << ": " << values[static_cast<size_t>(selectedIndex)];
        ++matchCount;
    }

    if (matchCount == 0)
        output << "No matching records.";
    output << '\n';
    return true;
}
