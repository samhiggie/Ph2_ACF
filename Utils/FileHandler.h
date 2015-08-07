
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
	char fOption;
	std::thread fThread;
	std::mutex fMutex;
	bool fFileIsOpened ;
	bool is_set;


  public:

	std::fstream fBinaryFile;
	std::vector<uint32_t> fData;

	//Constructor
	FileHandler( std::string pBinaryFileName, char pOption ):

		fBinaryFileName( pBinaryFileName ),
		fOption( pOption ),
		fFileIsOpened( false ) ,
		is_set( false ) {
		openFile();
		if ( fOption == 'w' )
			fThread = std::thread( &FileHandler::writeFile, this );
	}

	//destructor
	~FileHandler() {
		fThread.join();
		closeFile();
	}
	void set( std::vector<uint32_t> pVector ) {
		fMutex.lock();
		fData.clear();
		fData = pVector;
		is_set = true;
		if ( is_set )
			fMutex.unlock();
	}

	std::string getFilename() {
		return fBinaryFileName;
	}
	bool openFile( ) {
		if ( !file_open() ) {

			fMutex.lock();
			if ( fOption == 'w' ) fBinaryFile.open( ( getFilename() ).c_str(), std::fstream::trunc | std::fstream::out | std::fstream::binary );
			else if ( fOption == 'r' ) fBinaryFile.open( getFilename().c_str(),  std::fstream::in |  std::fstream::binary );
			fMutex.unlock();
			fFileIsOpened = true;
		}
		return file_open();
	}
	void closeFile() {
		fMutex.lock();
		fBinaryFile.close();
		fMutex.unlock();
	}
	bool file_open() {
		return fFileIsOpened;
	}
	//read from raw file to vector
	std::vector<uint32_t> readFile( ) {

		std::vector<uint32_t> cVector;
		//open file for the reading
		while ( !fBinaryFile.eof() ) {
			char buffer[4];
			fBinaryFile.read( buffer, 4 );
			uint32_t word;
			memcpy( &word, buffer, 4 );
			cVector.push_back( word );
		}
		fBinaryFile.close();
		return cVector;
	}

  private:
	void writeFile() {
		while ( true ) {

			if ( is_set ) {
				fMutex.lock();
				fMutex.unlock();
				fMutex.lock();
				uint32_t cBuffer[fData.size()];
				std::copy( fData.begin(), fData.end(), cBuffer );
				fBinaryFile.write( ( char* )&cBuffer, sizeof( cBuffer ) );
				fData.clear();
				is_set = false;
				fMutex.unlock();
				continue;
			}
			else {
				fMutex.lock();
				fMutex.unlock();
				continue;
			}
		}
	}
};

#endif

