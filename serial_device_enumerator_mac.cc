#include <stdio.h>
#include <string>

#include <CoreFoundation/CFNumber.h>
#include <IOKit/serial/IOSerialKeys.h>
#include <IOKit/usb/IOUSBLib.h>

using std::string;

template<typename T> T GetRegistryProperty(
    io_service_t device, CFStringRef keyName) {
  // Make sure that we search ancestor entries as well. The registry entry that
  // contains the USB properties is higher up the registry tree than the entry
  // that contains the serial properties.
  return (T) IORegistryEntrySearchCFProperty(
      device, kIOServicePlane, keyName, NULL,
      kIORegistryIterateRecursively | kIORegistryIterateParents);
}

bool GetStringProperty(
    io_service_t device, CFStringRef keyName, string* returnValue) {
  CFStringRef propertyValue = GetRegistryProperty<CFStringRef>(device, keyName);
  if (propertyValue) {
    CFStringEncoding encodingMethod = CFStringGetSystemEncoding();
    const char* str = CFStringGetCStringPtr(propertyValue, encodingMethod);

    if (str) {
      *returnValue = string(str);
      return true;
    }
  }

  return false;
}

bool GetIntProperty(
    io_service_t device, CFStringRef keyName, int* returnValue) {
  CFNumberRef propertyValue = GetRegistryProperty<CFNumberRef>(device, keyName);
  if (propertyValue)
    return CFNumberGetValue(propertyValue, kCFNumberIntType, returnValue);

  return false;
}

int main() {
  // Find and iterate through all serial devices.
  CFMutableDictionaryRef matchingDict;
  matchingDict = IOServiceMatching(kIOSerialBSDServiceValue);
  if (matchingDict == NULL)
    return 1;

  io_iterator_t iter;
  if (IOServiceGetMatchingServices(
          kIOMasterPortDefault, matchingDict, &iter) != KERN_SUCCESS)
    return 1;

  io_service_t device;
  while ((device = IOIteratorNext(iter))) {
    string calloutDevice;
    if (GetStringProperty(device, CFSTR(kIOCalloutDeviceKey), &calloutDevice)) {
      printf("Callout device: %s\n", calloutDevice.c_str());
    }

    string dialinDevice;
    if (GetStringProperty(device, CFSTR(kIODialinDeviceKey), &dialinDevice)) {
      printf("Dialin device: %s\n", dialinDevice.c_str());
    }

    string productString;
    if (GetStringProperty(device, CFSTR(kUSBProductString), &productString)) {
      printf("Product string: %s\n", productString.c_str());
    }

    int vendorId;
    if (GetIntProperty(device, CFSTR(kUSBVendorID), &vendorId)) {
      printf("Vendor ID: 0x%04x\n", vendorId);
    }

    int productId;
    if (GetIntProperty(device, CFSTR(kUSBProductID), &productId)) {
      printf("Product ID: 0x%04x\n", productId);
    }

    IOObjectRelease(device);
    printf("\n");
  }

  IOObjectRelease(iter);
  return 0;
}
