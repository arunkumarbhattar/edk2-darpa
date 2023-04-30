/** @file

Module Name:

  Demo1_Alice.h

Abstract:

  Alice driver works with Bob driver

Revision History: 0.1
**/

#ifndef _Demo1_Alice_H_
#define _Demo1_Alice_H_

#include <Uefi.h>
#include <Library/DebugLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PrintLib.h>
#include <Library/UefiLib.h>

#include "../Demo1_Access_Key/Demo1_Access_Key.h"
//#include "../Demo1_Variable/Demo1_Variable.h"

///
/// Global GUIDs - for reference: values are stored in EmulatorPkg.dec
///
#define Demo1_Alice_PROTOCOL_GUID           { 0x1cf631f3, 0xbe03, 0x4435,{ 0xa6, 0x32, 0x79, 0x4f, 0x85, 0x82, 0x0b, 0xab } }
#define Demo1_Alice_Ready_To_Run_GUID       { 0xc77ed9a1, 0x2785, 0x4a62,{ 0xa5, 0x12, 0xf5, 0x75, 0xfd, 0x64, 0x70, 0x73 } }

typedef struct _Demo1_Alice_PROTOCOL Demo1_Alice_PROTOCOL;

#define INIT_MODE           1
#define RUN_MODE            2
#define ALICEMODE_VARNAME   L"Alice_Mode"

///
/// Callback function Types
///
typedef
EFI_STATUS
(EFIAPI *ALICE_CB_TYPE)(
  IN Demo1_Alice_PROTOCOL   *This,
  IN EFI_HANDLE             Controller,
  IN OUT UINTN              *Data
);

///
/// This protocol is used to provide a callback function and event others can use
///
struct _Demo1_Alice_PROTOCOL {
  ALICE_CB_TYPE                           Demo1AliceProvideData;
  EFI_EVENT                               Demo1_Ready_To_Run_Event;
};

///
/// Prototype for the implementation of the callback function in the custom protocol
///
EFI_STATUS
EFIAPI
Demo1AliceProvideData(
  IN Demo1_Alice_PROTOCOL   *This,
  IN EFI_HANDLE             Controller,
  IN OUT UINTN              *Data
);

#endif
