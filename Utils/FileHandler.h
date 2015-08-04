
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
		fThread = std::thread( &FileHandler::writeFile, this, pData );
		fThread.join();
	}

	inline const std::vector<uint32_t> read() {



		// openFile( "r" );
		//obtain file size
		fseek( fBinaryFile, 0, SEEK_END );
		long lsize = ftell( fBinaryFile );
		rewind( fBinaryFile );
		uint32_t pDataBuffer[sizeof( uint32_t )*lsize];
		//read file
		fread( pDataBuffer, sizeof( uint32_t ), lsize, fBinaryFile );
		std::vector<uint32_t> cVector( pDataBuffer, pDataBuffer + sizeof( uint32_t )*lsize );
		closeFile();
		return cVector;

	}

  private:
	void writeFile( std::vector<uint32_t> pData ) {
		std::vector<uint32_t> cVectorCopy = pData;
		uint32_t pDataBuffer[cVectorCopy.size()];
		std::copy( cVectorCopy.begin(), cVectorCopy.end(), pDataBuffer );
		fMutex.lock();
		fwrite( &pDataBuffer , sizeof( pDataBuffer ) , 1, fBinaryFile );
		fMutex.unlock();
	}
};


#endif

