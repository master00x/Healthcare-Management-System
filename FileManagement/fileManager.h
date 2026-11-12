#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include <iostream>
#include <fstream>
#include <string>

using namespace std;

// Initialize data files
bool initializeFile(string filename);


int readHeader(string filename);
void writeHeader(string filename, int offset);
int getFirstAvailable(string filename);

int insertRecord(string filename, char record[], int recordSize);

bool deleteRecord(string filename, int recordOffset, int recordSize, int markerOffset = 0);
bool readRecord(string filename, int recordOffset,char record[], int recordSize);



#endif