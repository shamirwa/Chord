#ifndef _ENTRY_H
#define _ENTRY_H


#include <iostream>

using namespace std;


class Entry{
    private:
        string fileName; // hashcode for the filename
        string value;   // Content of the file

    public:
        Entry();
        Entry(string name, string value);
        ~Entry();
        void setFileName(string name);
        void setFileValue(string value);
        const string getFileName();
        const string getFileValue();
};

#endif
