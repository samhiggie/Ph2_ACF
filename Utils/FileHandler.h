
#ifndef __FILEHANDLER_H__
#define __FILEHANDLER_H__

#include <istream>
#include <iostream>
#include <boost/filesystem.hpp>
#include <fstream>
#include <vector>

#include <thread>

class FileHandler
{
  private:
	// for mini DAQ file IO
	std::string fBinaryFileName;
	std::string fOption;
	std::thread fThread;
	bool fFileIsOpened ;
  public:
	FILE* fBinaryFile;
	//Constructor
	FileHandler( std::string cBinaryFileName /*, std::string cOption */ ):

		fBinaryFileName( cBinaryFileName ),
		fFileIsOpened( false ) /*,
		fOption(cOption)*/
	{
		//REMEMBER TO UPDATE THIS PART!!!!!!!!!!!!!!!!!!!!!!!
		openFile( "w+" );

	}


	//destructor
	~FileHandler() {

		closeFile();
	}

	// std::ofstream getFile() {
	// 	return fBinaryFile;
	// }

	// void setFile( std::ofstream pBinaryFile ) {
	// 	fBinaryFile = pBinaryFile;
	// }

	inline std::string getFilename() {
		return fBinaryFileName;
	}

	inline bool openFile( std::string pOption ) {


		if ( !file_open() ) {
			// if ( !boost::filesystem::exists( getFilename() + ".raw" ) )
			fBinaryFile = fopen( ( getFilename() + ".raw" ).c_str(), pOption.c_str() );
			fFileIsOpened = true;
			if ( file_open() )
				std::cout << "FILE OPENED!!!!!!!!!!!!!!!!!!!!!!!  " << std::endl;

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
		fThread.join();
	}


	inline void writeFile( std::vector<uint32_t>  pData ) {

		std::vector<uint32_t> cVectorCopy;
		cVectorCopy = pData;

		uint32_t pDataBuffer[cVectorCopy.size()];
		std::copy( cVectorCopy.begin(), cVectorCopy.end(), pDataBuffer );
		std::cout << "pDataBuffer dimension:  " << sizeof( pDataBuffer ) / sizeof( pDataBuffer[0] ) << std::endl;
		std::cout << "cVectorCopy dimension:  " << cVectorCopy.size() << std::endl;
		if ( file_open() )
			std::cout << "FILE STILL OPENED " << std::endl;
		else
			std::cout << "THE FILE IS CLOSED!!!!!!!!!  " << std::endl;
		fwrite( /*( char* )&cVectorCopy[0]*/ ( char* )&pDataBuffer , cVectorCopy.size()*sizeof( uint32_t ) , 1, fBinaryFile );
		closeFile();
	}
};


#endif

