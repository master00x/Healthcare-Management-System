
#include "queryEngine.h"

// Remove unnecessary spaces, quotation marks and semicolons
// from the beginning and end of a string.
string trim(string text)
{
    size_t start = text.find_first_not_of(" \t\n\r'\";");

    if (start == string::npos)
        return "";

    size_t end = text.find_last_not_of(" \t\n\r'\";");

    return text.substr(start, end - start + 1);
}

bool parseQuery(const string &query, ParsedQuery &result)
{
    // Clear previous results in case this function is called again
    // using the same ParsedQuery object.
    result.selectedField.clear();
    result.tableName.clear();
    result.conditions.clear();

    // 1. Extract the selected field.
    size_t selectPos = query.find("Select");

    if (selectPos == string::npos)
        return false;

    size_t startPos = selectPos + 6;
    size_t fromPos = query.find("from");

    if (fromPos == string::npos || fromPos <= startPos)
        return false;

    string selectedField = query.substr(startPos, fromPos - startPos);
    result.selectedField = trim(selectedField);

    if (result.selectedField.empty())
        return false;

    // 2. Extract the table name.
    size_t tableStart = fromPos + 4;
    size_t wherePos = query.find("where");

    if (wherePos == string::npos || wherePos <= tableStart)
        return false;

    string tableName = query.substr(tableStart, wherePos - tableStart);
    result.tableName = trim(tableName);

    if (result.tableName.empty())
        return false;

    // 3. Extract the WHERE conditions.
    string conditions = query.substr(wherePos + 5);

    if (trim(conditions).empty())
        return false;

    // 4. Parse each condition separately.
    size_t conditionStart = 0;

    while (conditionStart < conditions.length())
    {
        // Find " and " to separate multiple conditions.
        // This avoids splitting words like "Cassandra".
        size_t andPos = conditions.find(" and ", conditionStart);

        string currentCondition;

        if (andPos == string::npos)
        {
            // Last condition.
            currentCondition = conditions.substr(conditionStart);
        }
        else
        {
            // Extract the current condition before AND.
            currentCondition = conditions.substr(
                conditionStart,
                andPos - conditionStart);
        }

        // Find the equal sign separating field and value.
        size_t equalPos = currentCondition.find('=');

        if (equalPos == string::npos)
            return false;

        // Extract the field and its corresponding value.
        string field = currentCondition.substr(0, equalPos);
        string value = currentCondition.substr(equalPos + 1);

        field = trim(field);
        value = trim(value);

        // Reject incomplete conditions.
        if (field.empty() || value.empty())
            return false;

        // Store the parsed condition.
        Condition condition;
        condition.field = field;
        condition.value = value;

        result.conditions.push_back(condition);

        // Move to the next condition.
        if (andPos == string::npos)
            break;

        conditionStart = andPos + 5;
    }

    return true;
}
