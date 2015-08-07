#ifndef USBANTENNA_H__
#define USBANTENNA_H__

#include <usb.h>

#define VERSION "0.1.0"
#define VENDOR_ID 0x10C4
#define PRODUCT_ID 0x87A0
#define INTERF 0
#define CH_DIAGNOSIS_DECISION_TH 80 // decision threshold value for channels diagnosis, expressed in % from 0 to 100	



usb_dev_handle *find_antenna_usb_handle();

usb_dev_handle* setup_libusb_access() {
     usb_dev_handle *antenna_usb_handle;
     usb_set_debug(255);
     usb_init();
     usb_find_busses();
     usb_find_devices();
     if(!(antenna_usb_handle = find_antenna_usb_handle())) {
 		printf("Couldn't find the USB device, Exiting\n");
 		std::cin.ignore();

 		return NULL;
 	}
 	if (usb_set_configuration(antenna_usb_handle, 1) < 0) {
 		printf("Could not set configuration 1 : \n");

 		return NULL;
 	}
 	if (usb_claim_interface(antenna_usb_handle, INTERF) < 0) {
 		printf("Could not claim interface: \n");

 		return NULL;
 	}

 	return antenna_usb_handle;
}

usb_dev_handle *find_antenna_usb_handle()
{
    struct usb_bus *bus;
 	struct usb_device *dev;
 	for (bus = usb_busses; bus; bus = bus->next) {
     	for (dev = bus->devices; dev; dev = dev->next) {
    			if (dev->descriptor.idVendor == VENDOR_ID &&
 		  		dev->descriptor.idProduct == PRODUCT_ID ) {
 				usb_dev_handle *handle;
 		  		printf("antenna_usb_handle with Vendor Id: %x and Product Id: %x found.\n", VENDOR_ID, PRODUCT_ID);
 				if (!(handle = usb_open(dev))) {
 					printf("Could not open USB device\n");
 					std::cin.ignore();

 					return NULL;
 				}

 				return handle;
 			}
  		}
 	}

 	return NULL;
}


#endif
