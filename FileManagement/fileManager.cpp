#include "fileManager.h"

using namespace std;

void initializeFile(string filename)
{
    fstream file(filename, ios::in | ios::out | ios::binary);

    if (!file.is_open())
    {
        // Create the file
        ofstream createfile(filename, ios::out | ios::binary);
        createfile.close();

        // Reopen the file for reading and writing
        file.clear();
        file.open(filename, ios::in | ios::out | ios::binary);

        // Initialize the header
        int header = -1;
        file.write((char *)&header, sizeof(header));
    }

    file.close();
}

int readHeader(string filename)
{
    fstream file(filename, ios::in | ios::binary);

    if (!file.is_open()) // if there's a probelm opening the file
        return -1;

    file.seekg(0, ios::beg);
    int headr;

    file.read((char *)&headr, sizeof(int));
    file.close();

    return headr;
}

void writeHeader(string filename, int offset)
{

    fstream file(filename, ios::out | ios::in | ios::binary);
    if (!file.is_open())
        return;

    file.seekp(0, ios::beg);

    // Check whether seeking succeeded
    if (!file)
    {
        file.close();
        return;
    }

    file.write((char *)&offset, sizeof(int));

    // Check whether writing succeeded
    if (!file)
    {
        cout << "Error: Failed to write header!" << endl;
        file.close();
        return;
    }

    file.close();
}

int getFirstAvailable(string filename)
{
    return readHeader(filename);
}


int insertRecord(string filename, char record[], int recordSize){
    if (record == nullptr || recordSize < 5)
        return -1;

    fstream file(filename, ios::in | ios::out | ios::binary);

    if (!file)
        return -1;

    int avail;
    file.seekg(0, ios::beg);
    if (!file.read((char*)&avail, sizeof(int)))
        return -1;

    int offset;

    if (avail == -1)
    {
        // No deleted slots: append the new record
        file.seekp(0, ios::end);
        offset = static_cast<int>(file.tellp());

        file.write(record, recordSize);
        if (!file)
            return -1;
    }
    else
    {
        // Reuse a deleted record
        offset = avail;

        int nextAvail;
        file.seekg(avail + 1, ios::beg);

        if (!file.read((char*)&nextAvail, sizeof(int)))
            return -1;

        // Replace the deleted record with new data
        file.seekp(avail, ios::beg);
        file.write(record, recordSize);

        if (!file)
            return -1;

        // Update AVAIL LIST header
        file.seekp(0, ios::beg);
        file.write((char*)&nextAvail, sizeof(int));

        if (!file)
            return -1;
    }

    file.flush();
    if (!file)
        return -1;

    return offset;
}


bool deleteRecord(string filename, int recordOffset,
                  int recordSize, int markerOffset)
{
    fstream file(filename, ios::in | ios::out | ios::binary);

    if (!file || recordSize < 5 ||
        markerOffset < 0 || markerOffset >= recordSize)
        return false;

    // Read current AVAIL LIST head
    int avail;
    file.read((char*)&avail, sizeof(int));

    if (!file)
        return false;

    file.seekg(0, ios::end);
    streamoff fileSize = file.tellg();

    if (recordOffset < static_cast<int>(sizeof(int)) ||
        static_cast<streamoff>(recordOffset) + recordSize > fileSize)
        return false;

    // Reject records that are already deleted
    char marker;
    file.seekg(recordOffset + markerOffset, ios::beg);
    file.read(&marker, 1);

    if (!file || marker == '*')
        return false;

    // Mark the record as deleted
    file.seekp(recordOffset + markerOffset, ios::beg);
    file.write("*", 1);

    // Store the previous AVAIL LIST head
    file.seekp(recordOffset + 1, ios::beg);
    file.write((char*)&avail, sizeof(int));

    if (!file)
        return false;

    // Point the header to the newly deleted record
    file.seekp(0, ios::beg);
    file.write((char*)&recordOffset, sizeof(int));

    file.flush();
    return file.good();
}

bool readRecord(string filename, int recordOffset,
                char record[], int recordSize)
{
    fstream file(filename, ios::in | ios::binary);

    if (!file.is_open())
    {
        cout << "Error opening file!" << endl;
        return false;
    }

    if (recordOffset < sizeof(int) || recordSize <= 0 || record == nullptr)
    {
        cout << "Invalid record parameters!" << endl;
        file.close();
        return false;
    }

    file.seekg(recordOffset, ios::beg);

    if (!file)
    {
        cout << "Error seeking to record!" << endl;
        file.close();
        return false;
    }

    file.read(record, recordSize);

    if (!file)
    {
        cout << "Error reading record!" << endl;
        file.close();
        return false;
    }

    file.close();
    return true;
}