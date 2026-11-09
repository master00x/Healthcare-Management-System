#include "DoctorStorage.h"

void writeField(char record[], int& position, string value, int size) {

    // Fill the field with spaces
    for (int i = 0; i < size; i++) {
        record[position + i] = ' ';
    }

    // Copy the value into the field
    for (int i = 0; i < value.length(); i++) {
        record[position + i] = value[i];
    }

    position += size;
}

string readField(const char record[], int& position, int size) {

    string value = "";

    for (int i = 0; i < size; i++) {
        value += record[position + i];
    }

    position += size;

    // Remove spaces from the end
    while (!value.empty() && value.back() == ' ') {
        value.pop_back();
    }

    return value;
}

void serializeDoctor(const Doctor& doctor, char record[]) {

    int position = 0;

    writeField(record, position, doctor.doctorID, 15);
    writeField(record, position, doctor.doctorName, 30);
    writeField(record, position, doctor.specialty, 20);
    writeField(record, position, doctor.address, 30);
}

Doctor deserializeDoctor(const char record[]) {

    Doctor doctor;
    int position = 0;

    doctor.doctorID = readField(record, position, 15);
    doctor.doctorName = readField(record, position, 30);
    doctor.specialty = readField(record, position, 20);
    doctor.address = readField(record, position, 30);

    return doctor;
}