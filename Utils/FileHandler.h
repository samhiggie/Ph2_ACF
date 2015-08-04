
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
	// std::vector<uint32_t>* fData;

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

	std::string getFilename() {
		return fBinaryFileName;
	}

	bool openFile( std::string pOption ) {
		if ( !file_open() ) {
			// if ( !boost::filesystem::exists( getFilename() + ".raw" ) )
			fMutex.lock();
			fBinaryFile = fopen( ( getFilename() + ".raw" ).c_str(), pOption.c_str() );
			fMutex.unlock();
			fFileIsOpened = true;
		}
		return file_open();
	}

	int closeFile() {
		fMutex.lock();
		std::cout << "Closing binary file!" << std::endl;
		int cStatus = fclose( fBinaryFile );
		fMutex.unlock();
		return cStatus;
	}

	bool file_open() {
		return fFileIsOpened;
	}

	void write( std::vector<uint32_t > pData ) {
		// std::vector<uint32_t> temp = pData;
		// char cBuffer[temp.size() * 4];
		// std::copy( temp.begin(), temp.end(), cBuffer );
		// std::cout << "Done with Deep Copy" << std::endl;
		// std::vector<uint32_t> cTmpData = pData;
		fThread = std::thread( &FileHandler::writeFile, this, pData );
		fThread.join();
		std::cout << "threads joined " << std::endl;
	}

	inline const std::vector<uint32_t> read() {

	}

  private:
	void writeFile( std::vector<uint32_t> pData ) {
		// std::cout << "New Thread! " << std::endl;
		// std::mutex pMutex;
		// pMutex.lock();
		// std::vector<uint32_t> temp = pData;
		// pMutex.unlock();

		std::vector<uint32_t> cTmpData = pData;
		char cBuffer[pData.size() * 4];

		std::copy( pData.begin(), pData.end(), cBuffer );
		fMutex.lock();
		fwrite( cBuffer  , sizeof( cBuffer ) , 1, fBinaryFile );
		fMutex.unlock();
	}
};


#endif

