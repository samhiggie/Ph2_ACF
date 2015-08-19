#include "Antenna.h"

int Antenna::initializeAntenna(){
	if ((fUsbHandle = setup_libusb_access()) == NULL)
	{ 
		std::cout<<"Abandon the ship! Failed to connect with antenna setup, check if it is plugged in the USB port."<<std::endl;
		exit(-1); // ok this is maybe a slight overreaction, but there is no point to continue testing if antenna test board is not connected
	}
	usb_claim_interface(fUsbHandle, 0); // claim the interface for antenna connection, so kernel cannot do it when we need to use the device
	usb_reset(fUsbHandle); // chip needs to wake up from whatever state it was in before its interface was reclaimed
	return 0;
}

void Antenna::close(){
	/*We release interface so any other software or kernel driver can claim it */
	usb_release_interface(fUsbHandle, 0);
	/*we close the usb connection with the cp2130 chip*/
 	usb_close(fUsbHandle);
}

usb_dev_handle* Antenna::find_antenna_usb_handle()
{
	struct usb_bus *bus;
 	struct usb_device *dev;
 	for (bus = usb_busses; bus; bus = bus->next) {
	     	for (dev = bus->devices; dev; dev = dev->next) {
    			if (dev->descriptor.idVendor == VENDOR_ID &&
 				dev->descriptor.idProduct == PRODUCT_ID ) {
 				usb_dev_handle *handle;
		  		std::cout << "antenna_usb_handle with Vendor Id: " << VENDOR_ID << " and Product Id: " << PRODUCT_ID << " found " << std::endl;
 				if (!(handle = usb_open(dev))) {
 					std::cout << "Could not open USB device" << std::endl;
		
 					return NULL;
 				}

				return handle;
 			}
  		}	
	}

 	return NULL;
}

usb_dev_handle* Antenna::setup_libusb_access() 
{
	usb_dev_handle *antenna_usb_handle;
	usb_set_debug(255);
	usb_init();
	usb_find_busses();
	usb_find_devices();
	if(!(antenna_usb_handle = find_antenna_usb_handle())) {
		std::cout << "Couldn't find the USB device, Exiting" << std::endl;
		std::cin.ignore();
		return NULL;
	}
	if (usb_set_configuration(antenna_usb_handle, 1) < 0) {
		std::cout << "Could not set configuration 1 : " << std::endl;

 		return NULL;
	}
 	if (usb_claim_interface(antenna_usb_handle, INTERF) < 0) {
		std::cout << "Could not claim interface: " << std::endl;

		return NULL;
	}
	
 	return antenna_usb_handle;
}

void Antenna::ConfigureSpiSlave(uint8_t pSlaveChipSelectId)
{		
	int result; // variable for gathering results from USB related transfers, useful only for debugging
	char buf_in[4] = {0}; // buf_in[] is used just for reading back data from chip, via usb, I used it only for debugging purpose to check if correct CS line has been set for SPI transfers
 	char control_msg_set_spi_word[2] = {0, 0x19}; //configuration values from CP2130 datasheet
	char buf_out[2] = {0, 2};  //configuration values from CP2130 datasheet
	char bulk_buffer_out[9] = {0, 0, 1, 0, 1, 0, 0, 0, 0}; //SPI communication values, bits of the last byte are the channels or the analog switch to be turned on
	buf_out[0] = pSlaveChipSelectId;
	control_msg_set_spi_word[0] = pSlaveChipSelectId;
	
	/*First activate chip select line of corresponding analog switch for SPI communication.*/
    	result = usb_control_msg(fUsbHandle, 0x40, 0x25, 0, 0, (char *) buf_out, sizeof(buf_out), fUsbTimeout);
	
	/*Check 'buf_in' if correct number of chip select channel was stored in the cp2130 chip.*/
    	result = usb_control_msg(fUsbHandle, 0xC0, 0x24, 0, 0, (char *) buf_in, sizeof(buf_in), fUsbTimeout);
	
	/*Set SPI transfer parameters.*/
    	result = usb_control_msg(fUsbHandle, 0x40, 0x31, 0, 0, (char *) control_msg_set_spi_word, sizeof(control_msg_set_spi_word), fUsbTimeout);
	
	/*Finally we can write through cp2130 to analog switch. We are writing an array of chars where the last byte is giving the position of channels to be turned on. 
	They are identified by '1' position in binary representation of that byte value.*/
	result = usb_bulk_write(fUsbHandle, fUsbEndpointBulkOut, (char *) bulk_buffer_out, sizeof(bulk_buffer_out), fUsbTimeout); // turning off all channels of analog switch 
	sleep(0.1);

}

void Antenna::TurnOnAnalogSwitchChannel(uint8_t pSwichChannelId)
{	
 	int result; // variable for gathering results from USB related transfers, useful only for debugging 
	char bulk_buffer_out[9] = {0, 0, 1, 0, 1, 0, 0, 0, 0}; //SPI communication values, bits of the last byte are the channels or the analog switch to be turned on
	if (pSwichChannelId == 9)
	{ 
		bulk_buffer_out[8] = 0; // this is just to turn off all the channels of analog switch (if powered it holds last written configuration) at the end of the loop
	}
	else
	{ 
		bulk_buffer_out[8] = (char)((1<<(pSwichChannelId-1))&0xFF);
	}

	result = usb_bulk_write(fUsbHandle, fUsbEndpointBulkOut, (char *) bulk_buffer_out, sizeof(bulk_buffer_out), fUsbTimeout);
	sleep(0.1);
	
}

