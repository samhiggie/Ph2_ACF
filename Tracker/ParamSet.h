#ifndef __PARAMSET_H__
#define __PARAMSET_H__

#include <map> 
#include <string> 

/** Set of parameters whose values will be read from and written into a text file with format key=value 
*/ 
class ParamSet{
public:
	ParamSet(){}
	ParamSet(const std::string& strFile);
	/// get a parameter value
	uint32_t getValue(const std::string& strName);
	/// Get a parameter integer value or the default value if there is no such parameter name
	uint32_t getValueDef(const std::string& strName, uint32_t uDefault);
	/// Get a parameter string value
	std::string getStrValue(const std::string& strName);
	/// Tell if an integer value exists
	bool containsIntValue(const std::string& strName);
	/// Tell if a string value exists
	bool containsStrValue(const std::string& strName);
	/// Remove an integer value
	void removeIntValue(const std::string& strName);
	///Set a parameter value (integer)
	virtual uint32_t setValue(const std::string& strName, uint32_t uVal);
	///Set a parameter value (string)
	virtual void setValue(const std::string& strName, const std::string& strVal);
	/// Return parameter names and their value in a string with format <parameter 1>=<value 1>\n<parameter 2>=<value 2>\n...
	std::string nameAndValuePairs();
	/// Read parameter and value pairs from a text file and write them into the board
	virtual bool loadParamValuePairsFromFile(const std::string& strFile);
	/// Remove all values
	void clearValues();
protected:
	///Internal values map
	std::map <std::string, uint32_t> mapIntValues;
	std::map <std::string, std::string> mapStrValues;
};

#endif

