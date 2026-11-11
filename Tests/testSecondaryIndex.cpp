#include "../SecondaryIndex/secondaryIndexManager.h"
#include <iostream>
#include <filesystem>
#include <fstream>
#include <algorithm>

using namespace std;

static int passed = 0, failed = 0;

#define CHECK(cond)                                                      \
    do                                                                   \
    {                                                                    \
        if (cond)                                                        \
            passed++;                                                    \
        else                                                             \
        {                                                                \
            failed++;                                                    \
            cout << "FAILED line " << __LINE__ << ": " #cond << endl;    \
        }                                                                \
    } while (0)

static bool eq(const vector<string> &a, const vector<string> &b) { return a == b; }

static long fileSize(const string &p)
{
    ifstream f(p, ios::binary | ios::ate);
    return (long)f.tellg();
}

int main()
{
    const string dir = "TestData";
    filesystem::remove_all(dir);

    cout << "[1] basic insert / duplicates / sorted order\n";
    {
        SecondaryIndex idx(dir + "/t.idx", dir + "/t.lst", 30, 15);
        CHECK(idx.open(true));

        CHECK(idx.insert("Ali", "D3"));
        CHECK(idx.insert("Omar", "D2"));
        CHECK(idx.insert("Ahmed", "D1"));
        CHECK(idx.insert("Ali", "D1")); 
        CHECK(idx.insert("Ali", "D2"));
        CHECK(!idx.insert("Ali", "D2"));
        CHECK(!idx.insert(string(31, 'x'), "D9")); 
        CHECK(!idx.insert("Zed", string(16, 'x')));

        CHECK(idx.keyCount() == 3);
        CHECK(eq(idx.search("Ali"), {"D1", "D2", "D3"})); 
        CHECK(eq(idx.search("Omar"), {"D2"}));
        CHECK(idx.search("Nobody").empty());
        
        CHECK(idx.findKey("Ahmed") == 0);
        CHECK(idx.findKey("Ali") == 1);
        CHECK(idx.findKey("Omar") == 2);
        CHECK(idx.findKey("Ahmad") == -1);
    }

    cout << "[2] delete: unlink, AVAIL list reuse, file does not grow\n";
    {
        SecondaryIndex idx(dir + "/t.idx", dir + "/t.lst", 30, 15);
        CHECK(idx.open(true));
        for (string pk : {"D1", "D2", "D3", "D4"})
            CHECK(idx.insert("Ali", pk));

        long size0 = fileSize(dir + "/t.lst");
        CHECK(size0 == 4 + 4 * (15 + 4)); 

        CHECK(idx.remove("Ali", "D2")); 
        CHECK(idx.remove("Ali", "D1")); 
        CHECK(idx.remove("Ali", "D4"));
        CHECK(eq(idx.search("Ali"), {"D3"}));
        CHECK(!idx.remove("Ali", "D2")); 
        CHECK(!idx.remove("Nobody", "D1"));

        {
            ifstream f(dir + "/t.lst", ios::binary);
            int avail;
            f.read((char *)&avail, 4);
            CHECK(avail != -1);
            char mark;
            f.seekg(avail);
            f.read(&mark, 1);
            CHECK(mark == '*');
        }

        CHECK(idx.insert("Neuro", "D5"));
        CHECK(idx.insert("Neuro", "D6"));
        CHECK(idx.insert("Neuro", "D7"));
        CHECK(fileSize(dir + "/t.lst") == size0);
        {
            ifstream f(dir + "/t.lst", ios::binary);
            int avail;
            f.read((char *)&avail, 4);
            CHECK(avail == -1);
        }
        // fourth one has to append
        CHECK(idx.insert("Neuro", "D8"));
        CHECK(fileSize(dir + "/t.lst") == size0 + 19);
        CHECK(eq(idx.search("Neuro"), {"D5", "D6", "D7", "D8"}));
        CHECK(eq(idx.search("Ali"), {"D3"}));

        
        CHECK(idx.remove("Ali", "D3"));
        CHECK(idx.findKey("Ali") == -1);
        CHECK(idx.keyCount() == 1);
    }

    
    cout << "[3] persistence: reopen from files\n";
    {
        {
            SecondaryIndex idx(dir + "/p.idx", dir + "/p.lst", 20, 15);
            CHECK(idx.open(true));
            idx.insert("Cardio", "D1");
            idx.insert("Cardio", "D4");
            idx.insert("Neuro", "D2");
            idx.insert("Derma", "D3");
            idx.remove("Cardio", "D1");
        }
        SecondaryIndex again(dir + "/p.idx", dir + "/p.lst", 20, 15);
        CHECK(again.open());
        CHECK(again.keyCount() == 3);
        CHECK(eq(again.search("Cardio"), {"D4"}));
        CHECK(eq(again.search("Derma"), {"D3"}));
        CHECK(again.insert("Cardio", "D9")); 
        CHECK(eq(again.search("Cardio"), {"D4", "D9"}));
    }

    cout << "[4] updateKey / updatePrimaryKey / renameKey\n";
    {
        SecondaryIndex idx(dir + "/u.idx", dir + "/u.lst", 30, 15);
        CHECK(idx.open(true));
        idx.insert("Ali", "D1");
        idx.insert("Ali", "D2");
        idx.insert("Omar", "D3");

        CHECK(idx.updateKey("D1", "Ali", "Zaki")); 
        CHECK(eq(idx.search("Ali"), {"D2"}));
        CHECK(eq(idx.search("Zaki"), {"D1"}));
        CHECK(!idx.updateKey("D9", "Ali", "Foo")); 
        CHECK(idx.search("Foo").empty());

        
        idx.insert("Ali", "D5");
        CHECK(idx.updatePrimaryKey("Ali", "D2", "D9"));
        CHECK(eq(idx.search("Ali"), {"D5", "D9"}));
        CHECK(!idx.updatePrimaryKey("Ali", "D2", "D7")); 
        CHECK(!idx.updatePrimaryKey("Ali", "D5", "D9")); 
        CHECK(idx.renameKey("Omar", "Omer"));
        CHECK(idx.search("Omar").empty());
        CHECK(eq(idx.search("Omer"), {"D3"}));
        CHECK(idx.renameKey("Ali", "Zaki")); 
        CHECK(eq(idx.search("Zaki"), {"D1", "D5", "D9"}));
        CHECK(idx.findKey("Ali") == -1);
    }

    cout << "[5] manager: the three indexes + multi-index query\n";
    {
        SecondaryIndexManager m(dir + "/mgr");
        CHECK(m.open(true));

        CHECK(m.onDoctorAdded("D1", "Ali", "Cardio"));
        CHECK(m.onDoctorAdded("D2", "Omar", "Neuro"));
        CHECK(m.onDoctorAdded("D3", "Ali", "Neuro"));
        CHECK(m.onDoctorAdded("D4", "Ali", "Cardio"));

        CHECK(eq(m.doctorIDsByName("Ali"), {"D1", "D3", "D4"}));
        CHECK(eq(m.doctorIDsBySpecialty("Neuro"), {"D2", "D3"}));
        CHECK(eq(m.doctorIDsByNameAndSpecialty("Ali", "Cardio"), {"D1", "D4"}));
        CHECK(eq(m.doctorIDsByNameAndSpecialty("Ali", "Neuro"), {"D3"}));
        CHECK(m.doctorIDsByNameAndSpecialty("Omar", "Cardio").empty());
        CHECK(m.doctorIDsByNameAndSpecialty("Nobody", "Neuro").empty());

        CHECK(m.onAppointmentAdded("A1", "D1"));
        CHECK(m.onAppointmentAdded("A2", "D1"));
        CHECK(m.onAppointmentAdded("A3", "D2"));
        CHECK(eq(m.appointmentIDsByDoctor("D1"), {"A1", "A2"}));
        CHECK(eq(m.appointmentIDsByDoctor("D2"), {"A3"}));
        CHECK(m.appointmentIDsByDoctor("D3").empty());

        CHECK(m.onAppointmentDeleted("A1", "D1"));
        CHECK(eq(m.appointmentIDsByDoctor("D1"), {"A2"}));
        CHECK(!m.onAppointmentDeleted("A1", "D1"));

        CHECK(m.onDoctorNameUpdated("D4", "Ali", "Sara"));
        CHECK(eq(m.doctorIDsByName("Ali"), {"D1", "D3"}));
        CHECK(eq(m.doctorIDsByName("Sara"), {"D4"}));
        CHECK(m.onDoctorSpecialtyUpdated("D4", "Cardio", "Derma"));
        CHECK(eq(m.doctorIDsByNameAndSpecialty("Sara", "Derma"), {"D4"}));

        CHECK(m.onDoctorIDUpdated("D1", "D7", "Ali", "Cardio"));
        CHECK(eq(m.doctorIDsByName("Ali"), {"D3", "D7"}));
        CHECK(eq(m.doctorIDsBySpecialty("Cardio"), {"D7"}));
        CHECK(m.appointmentIDsByDoctor("D1").empty());
        CHECK(eq(m.appointmentIDsByDoctor("D7"), {"A2"}));

        CHECK(m.onDoctorDeleted("D7", "Ali", "Cardio"));
        CHECK(eq(m.doctorIDsByName("Ali"), {"D3"}));
        CHECK(m.doctorIDsBySpecialty("Cardio").empty());
        CHECK(!m.onDoctorDeleted("D7", "Ali", "Cardio"));

        m.close();
        SecondaryIndexManager m2(dir + "/mgr");
        CHECK(m2.open());
        CHECK(eq(m2.doctorIDsByName("Ali"), {"D3"})); 
        CHECK(eq(m2.appointmentIDsByDoctor("D7"), {"A2"}));
    }

    cout << "[6] rebuild from data + stress against a naive model\n";
    {
        SecondaryIndexManager m(dir + "/rb");
        vector<DoctorKeys> docs = {{"D2", "Omar", "Neuro"}, {"D1", "Ali", "Cardio"},
                                    {"D3", "Ali", "Neuro"}};
        vector<AppointmentKeys> apps = {{"A2", "D1"}, {"A1", "D1"}, {"A3", "D3"}};
        CHECK(m.rebuild(docs, apps));
        CHECK(eq(m.doctorIDsByName("Ali"), {"D1", "D3"}));
        CHECK(eq(m.appointmentIDsByDoctor("D1"), {"A1", "A2"}));

        SecondaryIndex idx(dir + "/s.idx", dir + "/s.lst", 30, 15);
        CHECK(idx.open(true));
        vector<pair<string, string>> model;
        srand(7);
        bool allOk = true;
        for (int i = 0; i < 3000; i++)
        {
            string key = "K" + to_string(rand() % 12);
            string pk = "P" + to_string(rand() % 40);
            if (rand() % 3)
            {
                bool had = false;
                for (auto &p : model)
                    if (p.first == key && p.second == pk)
                        had = true;
                bool r = idx.insert(key, pk);
                if (r == had)
                    allOk = false;
                if (!had)
                    model.push_back({key, pk});
            }
            else
            {
                size_t at = model.size();
                for (size_t j = 0; j < model.size(); j++)
                    if (model[j].first == key && model[j].second == pk)
                        at = j;
                bool r = idx.remove(key, pk);
                if (r != (at != model.size()))
                    allOk = false;
                if (at != model.size())
                    model.erase(model.begin() + at);
            }
        }
        CHECK(allOk);
        for (int k = 0; k < 12; k++)
        {
            string key = "K" + to_string(k);
            vector<string> expect;
            for (auto &p : model)
                if (p.first == key)
                    expect.push_back(p.second);
            sort(expect.begin(), expect.end());
            CHECK(eq(idx.search(key), expect));
        }
    }

    filesystem::remove_all(dir);
    cout << "\nPassed: " << passed << "   Failed: " << failed << endl;
    return failed == 0 ? 0 : 1;
}
