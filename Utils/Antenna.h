#ifndef Antenna_h__
#define Antenna_h__

#include <usb.h>
#include <iostream>
#include <sstream>

#define VERSION "0.1.0"
#define VENDOR_ID 0x10C4
#define PRODUCT_ID 0x87A0
#define INTERF 0

namespace patch
{
	template < typename T > std::string to_string( const T& n )
	{
		std::ostringstream stm ;
		stm << n ;
		return stm.str() ;
	}
}

class Antenna
{
  public:
	Antenna() {}

	~Antenna() {}

	int initializeAntenna();

	void close();

	/*!
	* \brief private method that configures SPI interface between CP2130 and slave analog switch
	*/
	void ConfigureSpiSlave( uint8_t pSlaveChipSelectId );

	/*!
	* \brief private method that switches on given channel of last analog switch for which SPI interface was configured
	*/
	void TurnOnAnalogSwitchChannel( uint8_t pSwichChannelId );

	usb_dev_handle* find_antenna_usb_handle();

	usb_dev_handle* setup_libusb_access();

  private:
	usb_dev_handle* fUsbHandle;
	const static int fUsbEndpointBulkIn = 0x82;  // usb endpoint 0x82 address for USB IN bulk transfers
	const static int fUsbEndpointBulkOut = 0x01;  // usb endpoint 0x01 address for USB OUT bulk transfers
	const static int fUsbTimeout = 5000;  // usb operation timeout in ms
};
#endif
