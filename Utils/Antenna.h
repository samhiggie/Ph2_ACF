#ifndef Antenna_h__
#define Antenna_h__

#include <usb.h>
#include <iostream>

#define VERSION "0.1.0"
#define VENDOR_ID 0x10C4
#define PRODUCT_ID 0x87A0
#define INTERF 0
	

class Antenna
{
	public:
		Antenna(){}
		~Antenna(){}
		//int initializeAntenna();
		usb_dev_handle* fUsbHandle;	
		usb_dev_handle* find_antenna_usb_handle();
		usb_dev_handle* setup_libusb_access();
	private:		

};
#endif
