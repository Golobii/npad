#include "../include/fileManager.h"

string FileManager::readFile(string fileName)
{
    string fileContents;
    std::ifstream file;
    file.open(fileName.c_str());

    if (file.is_open())
    {
        string line;
        while (getline(file, line))
        {
            fileContents += line + "\n";
        }
        file.close();
    }
    else
    {
        fileContents = "Unable to open file";
    }
    return fileContents;
}

void FileManager::writeToFile(string fileName, string buffer)
{
    std::ofstream file;
    file.open(fileName.c_str());

    if (file.is_open())
    {
        file << buffer;
        file.close();
    }
    else
    {
        log("Unable to open file");
    }
}

void FileManager::log(string message)
{
    std::ofstream logFile;
    logFile.open("log.log", std::ios_base::app);
    logFile << message << std::endl;
    logFile.close();
}