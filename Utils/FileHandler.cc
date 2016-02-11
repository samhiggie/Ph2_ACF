#include "FileHandler.h"

//Constructor
FileHandler::FileHandler( const std::string& pBinaryFileName, char pOption ):
	fBinaryFileName( pBinaryFileName ),
	fOption( pOption ),
	fFileIsOpened( false ) ,
	is_set( false )
{
	openFile();
	//if ( fOption == 'w' ) fThread = std::thread( &FileHandler::writeFile, this );
}

//destructor
FileHandler::~FileHandler()
{
	//fThread.join();
	closeFile();
}
void FileHandler::set( std::vector<uint32_t> pVector )
{
	fMutex.lock();
	fData.clear();
	fData = pVector;
	is_set = true;
	if ( is_set )
		fMutex.unlock();
}

bool FileHandler::openFile( )
{
	if ( !file_open() )
	{
		fMutex.lock();
		if ( fOption == 'w' ) fBinaryFile.open( ( getFilename() ).c_str(), std::fstream::trunc | std::fstream::out | std::fstream::binary );
		else if ( fOption == 'r' ) fBinaryFile.open( getFilename().c_str(),  std::fstream::in |  std::fstream::binary );
		fMutex.unlock();
		fFileIsOpened = true;
	}
	return file_open();
}

void FileHandler::closeFile()
{
	fMutex.lock();
	fBinaryFile.close();
	fMutex.unlock();
}

//read from raw file to vector
std::vector<uint32_t> FileHandler::readFile( )
{
	std::vector<uint32_t> cVector;
	//open file for reading
	while ( !fBinaryFile.eof() )
	{
		char buffer[4];
		fBinaryFile.read( buffer, 4 );
		uint32_t word;
		std::memcpy( &word, buffer, 4 );
		cVector.push_back( word );
	}
	fBinaryFile.close();
	return cVector;
}

void FileHandler::writeFile()
{
	//while ( true ) {
		if ( is_set )
		{
			fMutex.lock();
			uint32_t cBuffer[fData.size()];
			std::copy( fData.begin(), fData.end(), cBuffer );
			fBinaryFile.write( ( char* )&cBuffer, sizeof( cBuffer ) );
			fData.clear();
			is_set = false;
			fMutex.unlock();
			//continue;
		}

		else
		{
			fMutex.lock();
			fMutex.unlock();
			//continue;
		}

	//}
}