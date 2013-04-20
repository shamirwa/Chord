#include "Entry.h"

Entry::Entry(){}

Entry::Entry(string name, string value){
    this->fileName = name;
    this->value = value;
}

Entry::~Entry(){}

const string Entry::getFileName(){

    return this->fileName;
}

const string Entry::getFileValue(){
    return this->value;
}

void Entry::setFileValue(string value){
    this->value = value;
}

void Entry::setFileName(string name){
    this->fileName = name;
}
