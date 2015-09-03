#include <stdio.h>

#include <CoreFoundation/CFNumber.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/serial/IOSerialKeys.h>
#include <IOKit/storage/IOMedia.h>
#include <IOKit/usb/IOUSBLib.h>
#include <IOKit/usb/USBSpec.h>

int main() {
  // Find and iterate through all serial devices.
  CFMutableDictionaryRef matchingDict;
  matchingDict = IOServiceMatching(kIOSerialBSDServiceValue);
  if (matchingDict == NULL)
    return -1;

  io_iterator_t iter;
  if (IOServiceGetMatchingServices(kIOMasterPortDefault, matchingDict, &iter) != KERN_SUCCESS)
    return -1;

  io_service_t device;
  while ((device = IOIteratorNext(iter))) {
    CFMutableDictionaryRef registryProperties;
    CFNumberRef cfNumber = NULL;
    CFStringRef cfString = NULL;
    CFStringEncoding encodingMethod = CFStringGetSystemEncoding();

    const char* dialinDevice = NULL;
    const char* calloutDevice = NULL;
    const char* productString = NULL;
    short vendorId = -1;
    short productId = -1;

    // Get the callout and dialin paths for the device.
    if (IORegistryEntryCreateCFProperties(device, &registryProperties, NULL, kNilOptions) != KERN_SUCCESS) {
      IOObjectRelease(device);
      continue;
    }

    if (CFDictionaryGetValueIfPresent(registryProperties, CFSTR(kIOCalloutDeviceKey), (const void **) &cfString)) {
      calloutDevice = CFStringGetCStringPtr(cfString, encodingMethod);
    }

    if (CFDictionaryGetValueIfPresent(registryProperties, CFSTR(kIODialinDeviceKey), (const void **) &cfString)) {
      dialinDevice = CFStringGetCStringPtr(cfString, encodingMethod);
    }

    // On Mac, devices are organized into a tree structure. For an
    // IOSerialBSDClient that is also an IOUSBDevice, we traverse the
    // registry tree upwards from the IOSerialBSDClient entry until we
    // hit the IOUSBDevice entry. Once there, we can access the
    // device properties that we need.
    io_registry_entry_t curr;
    kern_return_t kr = IORegistryEntryGetParentEntry(device, kIOServicePlane, &curr);
    while (kr == KERN_SUCCESS) {
      if (IOObjectConformsTo(curr, kIOUSBDeviceClassName)) {
        if (IORegistryEntryCreateCFProperties(curr, &registryProperties, NULL, kNilOptions) != KERN_SUCCESS) {
          break;
        }

        if (CFDictionaryGetValueIfPresent(registryProperties, CFSTR(kUSBProductString), (const void **) &cfString)) {
          productString = CFStringGetCStringPtr(cfString, encodingMethod);
        }

        if (CFDictionaryGetValueIfPresent(registryProperties, CFSTR(kUSBVendorID), (const void **) &cfNumber)) {
          CFNumberGetValue(cfNumber, kCFNumberIntType, &vendorId);
        }

        if (CFDictionaryGetValueIfPresent(registryProperties, CFSTR(kUSBProductID), (const void **) &cfNumber)) {
          CFNumberGetValue(cfNumber, kCFNumberIntType, &productId);
        }

        break;
      }

      io_registry_entry_t next;
      kr = IORegistryEntryGetParentEntry(curr, kIOServicePlane, &next);
      IOObjectRelease(curr);
      curr = next;
    }

    IOObjectRelease(curr);
    IOObjectRelease(device);

    printf("Dialin device: %s\n", dialinDevice);
    printf("Callout device: %s\n", calloutDevice);
    printf("Product string: %s\n", productString);
    printf("Vendor ID: 0x%04x\n", vendorId);
    printf("Product ID: 0x%04x\n", productId);
    printf("\n");
  }

  IOObjectRelease(iter);
  return 0;
}
