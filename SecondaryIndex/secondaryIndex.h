#ifndef SECONDARY_INDEX_H
#define SECONDARY_INDEX_H

#include <string>
#include <vector>
#include <fstream>

using namespace std;

class SecondaryIndex
{
public:
    SecondaryIndex(const string &indexFile, const string &listFile,int keySize, int primaryKeySize);
    ~SecondaryIndex();


    bool open(bool reset = false);
    void close();


    bool insert(const string &key, const string &primaryKey);
    bool remove(const string &key, const string &primaryKey);
    bool updateKey(const string &primaryKey, const string &oldKey, const string &newKey);
    bool updatePrimaryKey(const string &key, const string &oldPK, const string &newPK);
    bool renameKey(const string &oldKey, const string &newKey);

    int findKey(const string &key) const;
    vector<string> search(const string &key) const;

    int keyCount() const { return (int)entries.size(); }
    void print(const string &title) const; 
    string listFileName() const { return listPath; }

private:
    struct Entry
    {
        string key;
        int head;
    };

    string indexPath, listPath;
    int keySize, pkSize, nodeSize;
    vector<Entry> entries; 
    mutable fstream list; 

    int lowerBound(const string &key, bool &found) const;
    int readAvail() const;
    void writeAvail(int value);
    bool readNode(int offset, string &pk, int &next) const;
    bool writeNode(int offset, const string &pk, int next);
    int allocNode(const string &pk, int next); 
    void freeNode(int offset);               
    bool saveIndex() const;
    bool loadIndex();
    bool validKey(const string &s, int size) const;
};

#endif
