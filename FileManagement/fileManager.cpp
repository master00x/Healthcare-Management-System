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

void insertRecord(string filename, char record[], int recordSize)
{
    fstream file(filename, ios::in | ios::out | ios::binary);

    if (!file.is_open())
    {
        cout << "Error opening file!" << endl;
        return;
    }

    // Read header
    int avail;
    file.seekg(0, ios::beg);
    file.read((char *)&avail, sizeof(int));

    if (!file)
    {
        cout << "Error reading header!" << endl;
        file.close();
        return;
    }

    if (avail == -1)
    {
        // Case 1: AVAIL LIST is empty -> append at the end
        file.seekp(0, ios::end);
        file.write(record, recordSize);
    }
    else
    {
        // Case 2: Reuse the first deleted record
        int nextAvail;

        // Read the next deleted record's offset
        file.seekg(avail + 1, ios::beg);
        file.read((char *)&nextAvail, sizeof(int));

        if (!file)
        {
            cout << "Error reading next available offset!" << endl;
            file.close();
            return;
        }

        // Update header to point to the next deleted record
        file.seekp(0, ios::beg);
        file.write((char *)&nextAvail, sizeof(int));

        if (!file)
        {
            cout << "Error updating header!" << endl;
            file.close();
            return;
        }

        // Overwrite the deleted record
        file.seekp(avail, ios::beg);
        file.write(record, recordSize);
    }

    
    if (!file)
        cout << "Error writing record!" << endl;

    file.close();
}
