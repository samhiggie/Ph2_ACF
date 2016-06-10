/*!

        \file                   FileHeader.h
        \brief                  class for binary Raw data file header with infor
        \author                 Georg AUZINGER
        \version                1.0
        \date                   09/06/16
        Support :               mail to : georg.auzinger@cern.ch

 */
#ifndef __FILEHEADER_H__
#define __FILEHEADER_H__

#include <iostream>
#include <vector>

/*!
 * \class FileHandler
 * \brief Class to write Data objects in binary file using multithreading
*/

class FileHeader
{
  public:
    bool fValid;
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
    FileHeader() :
        fValid (false),
        fVersionMajor (0),
        fVersionMinor (0),
        fNBoard (0),
        fNCbc (0),
        fEventSize32 (1)
    {
    }


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
        uint32_t cMask = 0xAAAAAAAA;

        if (pVec.at (0) == cMask && pVec.at (3) == cMask && pVec.at (6) == cMask && pVec.at (9) == cMask && pVec.at (11) == cMask )
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
            fValid = true;
            std::cout << "Sucess, this is a valid header!" << std::endl;
        }
        else
        {
            std::cout << "Error, this is not a valid header!" << std::endl;
            fValid = false;
        }
    }
};

#endif
