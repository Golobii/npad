#if !defined(FILE_MANAGER_H)
#define FILE_MANAGER_H

#include <fstream>
#include <string>

using std::string;

class FileManager
{
protected:
    string fileName;
    string fileContents;

    string buffer;

    string readFile(string fileName);

    void writeToFile(string fileName, string buffer);

    void log(string message);
};

#endif // FILE_MANAGER_H
