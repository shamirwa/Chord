#ifndef _ENTRY_H
#define _ENTRY_H


#include <iostream>

using namespace std;


class Entry{
    private:
        string fileKey; // hashcode for the filename
        string value;   // Content of the file

    public:
        Entry();
        Entry(string key, string value);
        ~Entry();
        void setFileKey(string key);
        void setFileValue(string value);
        const string getFileKey();
        const string getFileValue();
};

#endif
