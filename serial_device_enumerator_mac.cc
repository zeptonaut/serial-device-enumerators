#include <stdio.h>

#include <CoreFoundation/CFNumber.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/usb/IOUSBLib.h>
#include <IOKit/usb/USBSpec.h>

int main() {
  CFMutableDictionaryRef matchingDict;
  matchingDict = IOServiceMatching(kIOUSBDeviceClassName);
  if (matchingDict == NULL)
    return -1;

  io_iterator_t iter;
  if (IOServiceGetMatchingServices(kIOMasterPortDefault, matchingDict, &iter) != KERN_SUCCESS)
    return -1;

  io_service_t device;
  while ((device = IOIteratorNext(iter))) {
    CFMutableDictionaryRef usbProperties;
    CFNumberRef cfVendorID = NULL;
    CFNumberRef cfProductID = NULL;
    CFStringRef cfProductString = NULL;
    CFStringRef cfVendorString = NULL;
    short intVal = 0;
    const char* strVal = NULL;

    if (IORegistryEntryCreateCFProperties(device,
                                          &usbProperties,
                                          NULL,
                                          kNilOptions) != KERN_SUCCESS) {
      IOObjectRelease(device);
      continue;
    }

    if (CFDictionaryGetValueIfPresent(usbProperties,
                                      CFSTR(kUSBProductString),
                                      (const void **) &cfProductString)) {
      CFStringEncoding encodingMethod = CFStringGetSystemEncoding();
      strVal = CFStringGetCStringPtr(cfProductString, encodingMethod);
      printf("Product string: %s\n", strVal);
    }

    if (CFDictionaryGetValueIfPresent(usbProperties, CFSTR(kUSBVendorID), (const void **) &cfVendorID)) {
      CFNumberGetValue(cfVendorID, kCFNumberShortType, &intVal);
      printf("Vendor ID: 0x%04x\n", intVal);
    }

    if (CFDictionaryGetValueIfPresent(usbProperties, CFSTR(kUSBProductID), (const void **) &cfProductID)) {
      CFNumberGetValue(cfProductID, kCFNumberShortType, &intVal);
      printf("Product ID: 0x%04x\n", intVal);
    }

    printf("\n");
    IOObjectRelease(device);
  }

  IOObjectRelease(iter);
  return 0;
}
