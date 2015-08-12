
#ifndef __FILEHANDLER_H__
#define __FILEHANDLER_H__

#include <istream>
#include <cstring>
#include <iostream>
#include <fstream>
#include <vector>
#include <mutex>
#include <thread>

class FileHandler

{
  private:
	// for mini DAQ file IO
	std::string fBinaryFileName;
	char fOption;
	std::thread fThread;
	std::mutex fMutex;
	bool fFileIsOpened ;
	bool is_set;


  public:

	std::fstream fBinaryFile;
	std::vector<uint32_t> fData;

	//Constructor
	FileHandler( std::string pBinaryFileName, char pOption );

	//destructor
	~FileHandler();
	void set( std::vector<uint32_t> pVector );

	std::string getFilename() {
		return fBinaryFileName;
	}

	bool openFile( );

	void closeFile();

	bool file_open() {
		return fFileIsOpened;
	}

	//read from raw file to vector
	std::vector<uint32_t> readFile( );

  private:
	void writeFile() ;
};

#endif

