// NOTE: This file doesn't compile with the supplied Makefile. It was extracted
// from a Visual Studio project and, because it was an exploratory exercise, I
// never took the time to make it compile separately. I welcome any patches to
// add Makefile support for this file.

#include "stdafx.h"

#include <iostream>
#include <windows.h>
#include <setupapi.h>
#include <devguid.h>
#include <regstr.h>

using std::cin;
using std::wstring;

bool GetProperty(HDEVINFO hDevInfo, SP_DEVINFO_DATA spDevInfoData, int property, wstring* returnValue) {
  // Call the function initially with a NULL buffer to determine how much 
  // space we need to allocate for its return value. Then, allocate a 
  // sufficiently large buffer and make the call again to actually retrieve
  // the property's value.
  wchar_t* buffer = NULL;
  DWORD bufferSize = 0;
  while (!SetupDiGetDeviceRegistryProperty(hDevInfo, &spDevInfoData, property, NULL, (PBYTE)buffer, bufferSize, &bufferSize))
  {
    if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
    {
      if (buffer) 
        delete buffer;

      // Allocate twice the size that we're told we need in order to avoid 
      // problems on W2k MBCS systems per KB 888609. 
      buffer = new wchar_t[bufferSize * 2];
    }
    else
    {
      delete buffer;
      return false;
    }
  }

  *returnValue = wstring(buffer);
  if (buffer) 
    delete buffer;

  return true;
}

// Borrows heavily from https://support.microsoft.com/en-us/kb/259695.
int main(int argc, char *argv[], char *envp[])
{
  // Find and iterate through all serial devices.
  HDEVINFO hDevInfo;
  hDevInfo = SetupDiGetClassDevs(&GUID_DEVINTERFACE_SERENUM_BUS_ENUMERATOR, 0, 0, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
  if (hDevInfo == INVALID_HANDLE_VALUE)
    return 1;

  SP_DEVINFO_DATA spDevInfoData;
  spDevInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
  for (DWORD i = 0; SetupDiEnumDeviceInfo(hDevInfo, i, &spDevInfoData); i++)
  {
    wstring deviceDesc;
    if (GetProperty(hDevInfo, spDevInfoData, SPDRP_DEVICEDESC, &deviceDesc))
      wprintf(L"Device desc: %s\n", deviceDesc.c_str());

    wstring friendlyName;
    if (GetProperty(hDevInfo, spDevInfoData, SPDRP_FRIENDLYNAME, &friendlyName))
      wprintf(L"Device desc: %s\n", friendlyName.c_str());

    wstring hardwareID;
    if (GetProperty(hDevInfo, spDevInfoData, SPDRP_HARDWAREID, &hardwareID))
      wprintf(L"Device desc: %s\n", hardwareID.c_str());

    wprintf(L"\n");
  }

  if (GetLastError() != NO_ERROR && GetLastError() != ERROR_NO_MORE_ITEMS)
    return 1;

  SetupDiDestroyDeviceInfoList(hDevInfo);

  int input;
  printf("Enter any input to exit: ");
  cin >> input;

  return 0;
}
