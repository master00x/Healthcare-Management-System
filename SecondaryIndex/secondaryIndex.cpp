#include "secondaryIndex.h"

#include <iostream>
#include <cstring>
#include <filesystem>

using namespace std;


static string trimRight(const char *buf, int size)
{
    int n = size;
    while (n > 0 && (buf[n - 1] == ' ' || buf[n - 1] == '\0'))
        n--;
    return string(buf, n);
}

static void ensureParentDir(const string &path)
{
    filesystem::path p(path);
    if (p.has_parent_path())
    {
        error_code ec;
        filesystem::create_directories(p.parent_path(), ec);
    }
}


SecondaryIndex::SecondaryIndex(const string &indexFile, const string &listFile,int keySize_, int primaryKeySize)
    : indexPath(indexFile), listPath(listFile),keySize(keySize_), pkSize(primaryKeySize),nodeSize(primaryKeySize + (int)sizeof(int))
{
}

SecondaryIndex::~SecondaryIndex() { close(); }

bool SecondaryIndex::open(bool reset)
{
    close();
    entries.clear();
    ensureParentDir(indexPath);
    ensureParentDir(listPath);

    bool haveIndex = false;
    if (!reset)
    {
        ifstream probe(indexPath, ios::binary);
        haveIndex = probe.is_open();
    }

    // The key array and the lists only make sense together. If the index
    // file is missing the lists are meaningless, so start both from scratch.
    bool freshList = reset || !haveIndex;
    if (!freshList)
    {
        ifstream probe(listPath, ios::binary);
        freshList = !probe.is_open();
        if (freshList)
            haveIndex = false;
    }

    if (freshList)
    {
        ofstream create(listPath, ios::binary | ios::trunc);
        int avail = -1;
        create.write((char *)&avail, sizeof(int));
    }

    list.open(listPath, ios::in | ios::out | ios::binary);
    if (!list.is_open())
        return false;

    if (haveIndex)
    {
        if (!loadIndex())
            return false;
    }
    else
    {
        if (!saveIndex())
            return false;
    }
    return true;
}

void SecondaryIndex::close()
{
    if (list.is_open())
    {
        list.flush();
        list.close();
    }
    list.clear();
}


bool SecondaryIndex::saveIndex() const
{
    ofstream f(indexPath, ios::binary | ios::trunc);
    if (!f)
        return false;

    int count = (int)entries.size();
    f.write((char *)&count, sizeof(int));

    for (const Entry &e : entries)
    {
        string padded = e.key;
        padded.resize(keySize, ' ');
        f.write(padded.data(), keySize);
        f.write((const char *)&e.head, sizeof(int));
    }
    return (bool)f;
}

bool SecondaryIndex::loadIndex()
{
    ifstream f(indexPath, ios::binary);
    if (!f)
        return false;

    int count = 0;
    f.read((char *)&count, sizeof(int));
    if (!f || count < 0)
        return false;

    entries.clear();
    vector<char> buf(keySize);
    for (int i = 0; i < count; i++)
    {
        Entry e;
        f.read(buf.data(), keySize);
        f.read((char *)&e.head, sizeof(int));
        if (!f)
        {
            entries.clear();
            return false;
        }
        e.key = trimRight(buf.data(), keySize);
        entries.push_back(e);
    }
    return true;
}


int SecondaryIndex::readAvail() const
{
    int v = -1;
    list.clear();
    list.seekg(0, ios::beg);
    list.read((char *)&v, sizeof(int));
    return v;
}

void SecondaryIndex::writeAvail(int value)
{
    list.clear();
    list.seekp(0, ios::beg);
    list.write((char *)&value, sizeof(int));
    list.flush();
}

bool SecondaryIndex::readNode(int offset, string &pk, int &next) const
{
    vector<char> buf(nodeSize);
    list.clear();
    list.seekg(offset, ios::beg);
    list.read(buf.data(), nodeSize);
    if (!list)
    {
        list.clear();
        return false;
    }
    pk = trimRight(buf.data(), pkSize);
    memcpy(&next, buf.data() + pkSize, sizeof(int));
    return true;
}

bool SecondaryIndex::writeNode(int offset, const string &pk, int next)
{
    string padded = pk;
    padded.resize(pkSize, ' ');

    list.clear();
    list.seekp(offset, ios::beg);
    list.write(padded.data(), pkSize);
    list.write((char *)&next, sizeof(int));
    list.flush();
    return (bool)list;
}


int SecondaryIndex::allocNode(const string &pk, int next)
{
    int avail = readAvail();
    int offset;

    if (avail != -1)
    {
        string oldPk;
        int nextAvail;
        if (!readNode(avail, oldPk, nextAvail))
            return -1;
        writeAvail(nextAvail);
        offset = avail;
    }
    else
    {
        list.clear();
        list.seekp(0, ios::end);
        offset = (int)list.tellp();
    }

    if (!writeNode(offset, pk, next))
        return -1;
    return offset;
}

void SecondaryIndex::freeNode(int offset)
{
    int oldHead = readAvail();
    writeNode(offset, "*", oldHead);
    writeAvail(offset);
}

bool SecondaryIndex::validKey(const string &s, int size) const
{
    return (int)s.length() <= size;
}


int SecondaryIndex::lowerBound(const string &key, bool &found) const
{
    int low = 0, high = (int)entries.size() - 1;
    found = false;

    while (low <= high)
    {
        int mid = low + (high - low) / 2;
        int cmp = entries[mid].key.compare(key);

        if (cmp == 0)
        {
            found = true;
            return mid;
        }
        if (cmp < 0)
            low = mid + 1;
        else
            high = mid - 1;
    }
    return low;
}

int SecondaryIndex::findKey(const string &key) const
{
    bool found;
    int pos = lowerBound(key, found);
    return found ? pos : -1;
}

vector<string> SecondaryIndex::search(const string &key) const
{
    vector<string> result;
    int pos = findKey(key);
    if (pos == -1)
        return result;

    int cur = entries[pos].head;
    while (cur != -1)
    {
        string pk;
        int next;
        if (!readNode(cur, pk, next))
            break;
        result.push_back(pk);
        cur = next;
    }
    return result;
}
// Search, then map each primary key to its record byte offset using the primary index.
vector<int> SecondaryIndex::searchOffsets(const string &key,int (*findOffset)(const string &)) const
{
    vector<int> offsets;
    for (const string &pk : search(key))
    {
        int off = findOffset(pk); // primary index: primary key -> offset
        if (off != -1)            // -1 = not found, skip it
            offsets.push_back(off);
    }
    return offsets;
}


bool SecondaryIndex::insert(const string &key, const string &primaryKey)
{
    if (!validKey(key, keySize) || !validKey(primaryKey, pkSize))
        return false;
    if (primaryKey.empty() || primaryKey[0] == '*')
        return false;

    bool found;
    int pos = lowerBound(key, found);

    if (!found)
    {
        int node = allocNode(primaryKey, -1);
        if (node == -1)
            return false;
        entries.insert(entries.begin() + pos, Entry{key, node}); 
        return saveIndex();
    }



    int prev = -1;
    int cur = entries[pos].head;
    while (cur != -1)
    {
        string pk;
        int next;
        if (!readNode(cur, pk, next))
            return false;
        if (pk == primaryKey)
            return false; // already linked
        if (pk > primaryKey)
            break;
        prev = cur;
        cur = next;
    }

    int node = allocNode(primaryKey, cur);
    if (node == -1)
        return false;

    if (prev == -1)
    {
        entries[pos].head = node;
        return saveIndex();
    }

    string prevPk;
    int dummy;
    if (!readNode(prev, prevPk, dummy))
        return false;
    return writeNode(prev, prevPk, node);
}

bool SecondaryIndex::remove(const string &key, const string &primaryKey)
{
    int pos = findKey(key);
    if (pos == -1)
        return false;

    int prev = -1;
    int cur = entries[pos].head;
    while (cur != -1)
    {
        string pk;
        int next;
        if (!readNode(cur, pk, next))
            return false;

        if (pk == primaryKey)
        {
            // unlink
            if (prev == -1)
                entries[pos].head = next;
            else
            {
                string prevPk;
                int dummy;
                if (!readNode(prev, prevPk, dummy))
                    return false;
                writeNode(prev, prevPk, next);
            }
            freeNode(cur);

            if (entries[pos].head == -1)
                entries.erase(entries.begin() + pos);

            return saveIndex();
        }
        prev = cur;
        cur = next;
    }
    return false;
}

bool SecondaryIndex::updateKey(const string &primaryKey, const string &oldKey,const string &newKey)
{
    if (oldKey == newKey)
        return findKey(oldKey) != -1;
    if (!validKey(newKey, keySize))
        return false;
    if (!remove(oldKey, primaryKey))
        return false;
    if (!insert(newKey, primaryKey))
    {
        insert(oldKey, primaryKey); 
        return false;
    }
    return true;
}

bool SecondaryIndex::updatePrimaryKey(const string &key, const string &oldPK, const string &newPK)
{
    
    if (oldPK == newPK)
        return false;
    if (!validKey(newPK, pkSize) || newPK.empty() || newPK[0] == '*')
        return false;
    int pos = findKey(key);
    if (pos == -1)
        return false;
    vector<string> current = search(key);
    bool hasOld = false;
    for (const string &s : current)
    {
        if (s == newPK)
            return false;
        if (s == oldPK)
            hasOld = true;
    }
    if (!hasOld)
        return false;

    if (!remove(key, oldPK))
        return false;
    if (!insert(key, newPK))
    {
        insert(key, oldPK);
        return false;
    }
    return true;
}

bool SecondaryIndex::renameKey(const string &oldKey, const string &newKey)
{
    if (oldKey == newKey || !validKey(newKey, keySize))
        return false;
    vector<string> pks = search(oldKey);
    if (pks.empty())
        return false;

    for (const string &pk : pks)
        remove(oldKey, pk);
    for (const string &pk : pks)
        insert(newKey, pk);
    return true;
}


void SecondaryIndex::print(const string &title) const
{
    cout << "--- " << title << " (" << entries.size() << " keys) ---\n";
    for (const Entry &e : entries)
    {
        cout << "  [" << e.key << "] head=" << e.head << " -> ";
        int cur = e.head;
        while (cur != -1)
        {
            string pk;
            int next;
            if (!readNode(cur, pk, next))
                break;
            cout << pk << " ";
            cur = next;
        }
        cout << "\n";
    }
}
