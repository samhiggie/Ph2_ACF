
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

	// std::vector<uint32_t>* fData;

  public:
	//FILE* fBinaryFile;
	std::fstream fBinaryFile;
	std::vector<uint32_t> fData;
	//Constructor
	FileHandler( std::string cBinaryFileName  /*std::string cOption*/ ):

		fBinaryFileName( cBinaryFileName ),
		fFileIsOpened( false ) ,
		is_set( false ) {
		openFile( );
		fThread = std::thread( &FileHandler::writeFile, this );
		fThread.detach();

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
			fMutex.lock();
			fBinaryFile.open( ( getFilename() + ".raw" ).c_str(), std::fstream::in | std::fstream::out | std::fstream::binary );
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

	// void write( std::vector<uint32_t > pData ) {
	// 	fThread = std::thread( &FileHandler::writeFile, this, pData );
	// 	fThread.join();
	// }

	// inline  std::vector<uint32_t> read() {




	// 	fseek( fBinaryFile, 0, SEEK_END );

	// 	std::cout << "check1" << std::endl;
	// 	long lsize = ftell( fBinaryFile );
	// 	rewind( fBinaryFile );
	// 	std::cout << "check2" << std::endl;

	// 	lsize /= sizeof( uint32_t );
	// 	uint32_t pDataBuffer[lsize];
	// 	//read file
	// 	fread( pDataBuffer, sizeof( uint32_t ), lsize, fBinaryFile );
	// 	std::vector<uint32_t> cVector( lsize );
	// 	for ( int i = 0; i < lsize; i++ )
	// 		cVector[i] = pDataBuffer[i];

	// 	return cVector;

	// }

  private:
	void writeFile() {

		while ( true ) {
			//std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) );
			if ( is_set ) {
				fMutex.lock();
				std::cout << "is_set = true: writing..." << std::endl;
				fMutex.unlock();

				fMutex.lock();
				//for ( auto& cElements : fData )
				//fBinaryFile.write( ( char* ) &cElements, sizeof( uint32_t ) );
				// std::copy( fData.begin(), fData.end(), std::ostreambuf_iterator<char>( fBinaryFile ) );
				uint32_t cBuffer[fData.size()];
				std::copy( fData.begin(), fData.end(), cBuffer );
				fBinaryFile.write( ( const char* )cBuffer, sizeof( cBuffer ) );
				fData.clear();
				is_set = false;
				fMutex.unlock();
				continue;
			}
			else {
				fMutex.lock();
				// std::cout << "is_set = false: NOT writing..." << std::endl;
				fMutex.unlock();
				continue;
			}
			// if ( !is_set ) {
			// 	fMutex.lock();
			// 	std::cout << "is_set = false: NOT writing..." << std::endl;
			// 	fMutex.unlock();
			// }
			// std::vector<uint32_t> cVectorCopy = pData;
			// uint32_t pDataBuffer[cVectorCopy.size()];
			// std::copy( cVectorCopy.begin(), cVectorCopy.end(), pDataBuffer );
			// fMutex.lock();
			// fwrite( &pDataBuffer , sizeof( pDataBuffer ) , 1, fBinaryFile );
			// fMutex.unlock();
		}
		// while ( true );
		fMutex.lock();
		std::cout << "What is wrong " << std::endl;
		fMutex.unlock();
	}
};

#endif

