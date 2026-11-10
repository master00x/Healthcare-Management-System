#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include <iostream>
#include <fstream>
#include <string>

using namespace std;

// Initialize data files
void initializeFile(string filename);


int readHeader(string filename);
void writeHeader(string filename, int offset);
int getFirstAvailable(string filename);
void insertRecord (string filename ,char record[], int recordSize);



#endif