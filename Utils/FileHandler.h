
#ifndef __FILEHANDLER_H__
#define __FILEHANDLER_H__

#include <istream>
#include <cstring>
#include <iostream>
#include <fstream>
#include <vector>
#include <mutex>
#include <thread>

/*!
 * \class FileHandler
 * \brief Class to write Data objects in binary file using multithreading
*/

class FileHeader
{
  public:
    // FW type
    char fType[8];
    uint32_t fVersionMajor;
    uint32_t fVersionMinor;
    // Info about HW
    uint32_t fNBoard;
    uint32_t fNCbc;
    //EventSize
    uint32_t fEventSize32;
    //Header Size useful for encoding and decoding
    static const uint32_t fHeaderSize32 = 12;

  public:
    std::vector<uint32_t> encodeHeader()
    {
        std::vector<uint32_t> cVec;

        //surround every block with 10101....
        cVec.push_back (0xAAAAAAAA);
        //insert the header
        //2 words
        cVec.push_back (fType[0] << 24 | fType[1] << 16 | fType[2] << 8 | fType[3]);
        cVec.push_back (fType[4] << 24 | fType[5] << 16 | fType[6] << 8 | fType[7]);

        cVec.push_back (0xAAAAAAAA);
        // 2 words FW version
        cVec.push_back (fVersionMajor);
        cVec.push_back (fVersionMinor);

        cVec.push_back (0xAAAAAAAA);
        // 2 words nObjecs
        cVec.push_back (fNBoard);
        cVec.push_back (fNCbc);

        cVec.push_back (0xAAAAAAAA);
        //1 word event size
        cVec.push_back (fEventSize32);
        cVec.push_back (0xAAAAAAAA);

        return cVec;
    }

    void decodeHeader (const std::vector<uint32_t>& pVec)
    {
        if (pVec.at (0) == pVec.at (3) == pVec.at (6 == pVec.at (9) == pVec.at (11) == 0xAAAAAAAA) )
        {
            fType[0] = (pVec.at (1) && 0xFF000000) >> 24;
            fType[1] = (pVec.at (1) && 0x00FF0000) >> 16;
            fType[2] = (pVec.at (1) && 0x0000FF00) >> 8;
            fType[3] = (pVec.at (1) && 0x000000FF);

            fType[4] = (pVec.at (2) && 0xFF000000) >> 24;
            fType[5] = (pVec.at (2) && 0x00FF0000) >> 16;
            fType[6] = (pVec.at (2) && 0x0000FF00) >> 8;
            fType[7] = (pVec.at (2) && 0x000000FF);

            fVersionMajor = pVec.at (4);
            fVersionMinor = pVec.at (5);

            fNBoard = pVec.at (7);
            fNCbc = pVec.at (8);

            fEventSize32 = pVec.at (10);
        }
        else std::cout << "Error, Vector does not contain a Header" << std::endl;
    }
};

class FileHandler

{
  public:
    FileHeader fHeader;

  private:

    std::string fBinaryFileName;
    char fOption;/*!< option for read or write */
    std::thread fThread;/*!< a thread for the multitrading */
    std::mutex fMutex;/*!< Mutex */
    bool fFileIsOpened ;/*!< to check if the file is opened */
    bool is_set;/*!< check if fdata is set */


  public:

    std::fstream fBinaryFile;/*!< the stream of the binary file */
    std::vector<uint32_t> fData;/*!< the vector of data */

    /*!
    * \brief constructor for the class
    * \param pBinaryFileName: set the fbinaryFileName to pbinaryFileName
    * \param  pOption: set fOption to pOption
    */
    FileHandler ( const std::string& pBinaryFileName, char pOption );
    /*!
    * \brief constructor for the class for write access with header object
    * \param pBinaryFileName: set the fbinaryFileName to pbinaryFileName
    * \param  pHeader: a const reference to a FileHeader object
    */
    FileHandler ( const std::string& pBinaryFileName, const FileHeader& pHeader );

    /*!
    * \brief destructor
    */
    ~FileHandler();

    /*!
     * \brief set the header
     * \param pHeader: reference to a FileHeaderObject
     */
    void setHeader ( const FileHeader& pHeader )
    {
        fHeader = pHeader;
    }
    /*!
    * \brief set fData to pVector
    */
    void set ( std::vector<uint32_t> pVector );


    /*!
    * \brief get the name of the binary file
    * \param return the name of the file
    */
    std::string getFilename() const
    {
        return fBinaryFileName;
    }

    /*!
    * \brief Open file
    * \return 0 if the file cannot be opened
    */
    bool openFile( );
    /*!
    * \brief close the file
    */
    void closeFile();
    /*!
    * \brief check if the file is open
    * \return true if the file is already opened
    */
    bool file_open()
    {
        return fFileIsOpened;
    }
    /*!
     * \brief read from raw file
     * \return a vector with the data read from file
     */
    std::vector<uint32_t> readFile( );
    std::vector<uint32_t> readFileChunks ( uint32_t pNWords32 );
    std::vector<uint32_t> readFileTail ( long pNbytes );

    /*!
    * \brief Write data to file
    */
    void writeFile() ;
};

#endif
