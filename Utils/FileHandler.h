
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
	bool is_set;


  public:

	std::fstream fBinaryFile;
	std::vector<uint32_t> fData;
	//Constructor
	FileHandler( std::string cBinaryFileName ):

		fBinaryFileName( cBinaryFileName ),
		fFileIsOpened( false ) ,
		is_set( false ) {
		openFile( );
		fThread = std::thread( &FileHandler::writeFile, this );
		// fThread.detach();

	}

	//destructor
	~FileHandler() {
		fThread.join();
		closeFile();
	}




	void set( std::vector<uint32_t> pVector ) {

		// while ( !is_set ) {
		fMutex.lock();
		fData.clear();
		fData = pVector;
		is_set = true;
		if ( is_set )

			std::cout << "is_set is true" << std::endl;
		fMutex.unlock();
		// }

	}



	std::string getFilename() {
		return fBinaryFileName;
	}

	bool openFile( ) {
		if ( !file_open() ) {
			// if ( !boost::filesystem::exists( getFilename() + ".raw" ) )
			// 	remove( ( getFilename() + ".raw" ).c_str() );
			fMutex.lock();
			fBinaryFile.open( ( getFilename() + ".raw" ).c_str(), std::fstream::trunc | std::fstream::in | std::fstream::out | std::fstream::binary );
			std::cout << "File opened!" << std::endl;
			fMutex.unlock();
			fFileIsOpened = true;

		}
		return file_open();
	}

	void closeFile() {
		fMutex.lock();
		std::cout << "Closing binary file!" << std::endl;
		fBinaryFile.close();
		fMutex.unlock();

	}

	bool file_open() {
		return fFileIsOpened;
	}



  private:
	void writeFile() {

		while ( true ) {

			if ( is_set ) {
				fMutex.lock();
				std::cout << "is_set = true: writing..." << std::endl;
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

