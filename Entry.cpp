#include "Entry.h"

Entry::Entry(){}

Entry::Entry(string key, string value){
    this->fileKey = key;
    this->value = value;
}

Entry::~Entry(){}

const string Entry::getFileKey(){

    return this->fileKey;
}

const string Entry::getFileValue(){
    return this->value;
}

void Entry::setFileValue(string value){
    this->value = value;
}

void Entry::setFileKey(string key){
    this->fileKey = key;
}
