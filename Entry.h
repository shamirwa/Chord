#include <iostream>

using namespace std;


class Entry{
    private:
        string fileKey; // hashcode for the filename
        string value;   // Content of the file

    public:
        Entry();
        ~Entry();
        const string getFileKey();
        const string getFileValue();
};
