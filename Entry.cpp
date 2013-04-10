#include "Entry.h"

Entry::Entry(){}

Entry::~Entry(){}

const string Entry::getFileKey(){

    return this->fileKey;
}

const string Entry::getFileValue(){
    return this->value;
}
