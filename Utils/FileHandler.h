
#ifndef __FILEHANDLER_H__
#define __FILEHANDLER_H__

#include <istream>
#include <iostream>
#include <boost/filesystem.hpp>
#include <fstream>
#include <vector>
#include <mutex>
#include <thread>

class FileHandler
{
  private:
	// for mini DAQ file IO
	std::string fBinaryFileName;
	std::string fOption;
	std::thread fThread;
	std::mutex fMutex;
	bool fFileIsOpened ;
  public:
	FILE* fBinaryFile;
	//Constructor
	FileHandler( std::string cBinaryFileName , std::string cOption ):
		fBinaryFileName( cBinaryFileName ),
		fFileIsOpened( false ) {
		openFile( cOption );
	}

	//destructor
	~FileHandler() {
		closeFile();
	}

	inline std::string getFilename() {
		return fBinaryFileName;
	}

	inline bool openFile( std::string pOption ) {
		if ( !file_open() ) {
			// if ( !boost::filesystem::exists( getFilename() + ".raw" ) )
			fBinaryFile = fopen( ( getFilename() + ".raw" ).c_str(), pOption.c_str() );
			fFileIsOpened = true;
		}
		return file_open();
	}

	inline int closeFile() {
		return fclose( fBinaryFile );
	}

	inline bool file_open() {
		return fFileIsOpened;
	}

	inline void write( std::vector<uint32_t > pData ) {
		fThread = std::thread( &FileHandler::writeFile, this, pData );
		fThread.detach();
	}

	inline const std::vector<uint32_t> read() {

	}

  private:
	inline void write( std::vector<uint32_t>  pData ) {

		fMutex.lock();
		char cBuffer[pData.size() * 4];

		std::copy( pData.begin(), pData.end(), cBuffer );
		fMutex.unlock();
		fwrite( cBuffer  , sizeof( cBuffer ) , 1, fBinaryFile );
		closeFile();
	}
};


#endif

