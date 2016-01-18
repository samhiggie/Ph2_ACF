#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <fstream>
//#include <boost/algorithm/string.hpp>
//#include "toolbox/string.h"
#include "ParamSet.h"

using namespace std;

ParamSet::ParamSet(){
}

uint32_t ParamSet::setValue(const std::string& strName, uint32_t uVal){
	mapIntValues[strName]=uVal;
	return uVal;
}

void ParamSet::setValue(const std::string& strName, const std::string& strVal){
	mapStrValues[strName]=strVal;
}

uint32_t ParamSet::getValue(const std::string& strName){
	return mapIntValues[strName];
}
	
std::string ParamSet::getStrValue(const std::string& strName){
	return mapStrValues[strName];
}

bool ParamSet::containsIntValue(const std::string& strName){
	return mapIntValues.find(strName)!=mapIntValues.end();
}

bool ParamSet::containsStrValue(const std::string& strName){
	return mapStrValues.find(strName)!=mapStrValues.end();
}

void ParamSet::removeIntValue(const std::string& strName){
	mapIntValues.erase(strName);
}
	
/// Return parameter names and their value in a string with format <parameter 1>=<value 1>\n<parameter 2>=<value 2>\n...
std::string ParamSet::nameAndValuePairs(){
	std::ostringstream ostream;
	for (map<string, uint32_t>::const_iterator it=mapIntValues.begin(); it!=mapIntValues.end(); it++)
		ostream<<it->first<<"="<<it->second<<endl;
		
	for (map<string, string>::const_iterator it=mapStrValues.begin(); it!=mapStrValues.end(); it++)
		ostream<<it->first<<"='"<<it->second<<"'"<<endl;
		
	return ostream.str();
}
/// Read parameter and value pairs from a text file and write them into the board
bool ParamSet::loadParamValuePairsFromFile(const std::string& strFile){
    ifstream fValues(strFile.c_str());
    string strLig;
    if (fValues.good()){
    	mapIntValues.clear(); 
    	mapStrValues.clear();
    	while (fValues.good()){
        	getline(fValues, strLig);
        	size_t iEqual=strLig.find('=');
        	if (iEqual!=string::npos && iEqual>0){
        		if (strLig[iEqual+1]=='\'')//string value
        			setValue(strLig.substr(0,iEqual), strLig.substr(iEqual+2, strLig.length()-iEqual-3).c_str());
        		else //Numeric value
        			setValue(strLig.substr(0,iEqual), (uint32_t)atol(strLig.substr(iEqual+1).c_str()));
        	}
        }
        fValues.close();
        return true;
    } else
    	return false;
}	
///Delete all integer and string values
void ParamSet::clearValues(){
	mapIntValues.clear();
	mapStrValues.clear();
}
	

