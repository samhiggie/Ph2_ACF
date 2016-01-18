#ifndef __PARAMSET_H__
#define __PARAMSET_H__

#include <map> 
#include <string> 

/** Set of parameters whose values will be read from and written into a text file with format key=value 
*/ 
class ParamSet{
public:
	ParamSet();
	/// get a parameter value
	uint32_t getValue(const std::string& strName);
	std::string getStrValue(const std::string& strName);
	bool containsIntValue(const std::string& strName);
	bool containsStrValue(const std::string& strName);
	void removeIntValue(const std::string& strName);
	///Set a parameter value
	uint32_t setValue(const std::string& strName, uint32_t uVal);
	void setValue(const std::string& strName, const std::string& strVal);
	/// Return parameter names and their value in a string with format <parameter 1>=<value 1>\n<parameter 2>=<value 2>\n...
	std::string nameAndValuePairs();
	/// Read parameter and value pairs from a text file and write them into the board
	bool loadParamValuePairsFromFile(const std::string& strFile);
	void clearValues();
private:
	///Internal values map
	std::map <std::string, uint32_t> mapIntValues;
	std::map <std::string, std::string> mapStrValues;
};

#endif

