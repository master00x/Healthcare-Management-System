#ifndef QUERY_ENGINE_H
#define QUERY_ENGINE_H

#include <iostream>
#include <string>
#include <vector>

using namespace std;

struct Condition
{
    string field;
    string value;
};

struct ParsedQuery
{
    string selectedField;
    string tableName;
    vector<Condition> conditions;
};

bool parseQuery(const string& query, ParsedQuery& result);

#endif