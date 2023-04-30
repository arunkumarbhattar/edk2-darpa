/** @file

Module Name:

  Demo1_Access_Key.h

Abstract:

  Provides protocol for generating/validating an access key

Revision History: 0.1
**/

#ifndef _DEMO1_ACCESS_Key_H_
#define _DEMO1_ACCESS_Key_H_

#include <Uefi.h>
#include <Library/DebugLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PrintLib.h>
#include <Library/UefiLib.h>

#include <Library/RngLib.h>
#include <Protocol/Rng.h>

///
/// Global GUIDs - for reference: values are stored in EmulatorPkg.dec
///
#define Demo1_Access_Key_PROTOCOL_GUID { 0xa084c36d, 0x630a, 0x4265,{ 0xbd, 0x44, 0x62, 0x21, 0x72, 0x41, 0xbc, 0x37 } }
#define Demo1_Access_Key_Ready_To_Lock_GUID { 0xe795d54e, 0xd24f, 0x430e,{ 0xa6, 0x33, 0xec, 0xe5, 0xd8, 0x72, 0xd8, 0xc2 } }

typedef struct _Demo1_Access_Key_PROTOCOL Demo1_Access_Key_PROTOCOL;

#define KEYSIZE   16

#define ACCESS_KEY_MAGIC      0xDEC0DEBABB1E
#define MAGIC_SIZE            0x10
#define WRITE_ACCESS          0x7E11
#define READ_ACCESS           0x10AD
#define NO_ACCESS             0x0000

///
/// Callback function Types
///
typedef
EFI_STATUS
(EFIAPI *DRVR_FUNC_CB_TYPE1)(
  IN Demo1_Access_Key_PROTOCOL            *This,
  IN EFI_HANDLE                           Controller,
  IN BOOLEAN                              WriteAccess,
  IN OUT DEMO1_ACCESS_KEY                 *AccessKeyPtr
);

typedef
EFI_STATUS
EFIAPI
(EFIAPI *DRVR_FUNC_CB_TYPE2) (
  IN Demo1_Access_Key_PROTOCOL            *This,
  IN EFI_HANDLE                           Controller,
  IN DEMO1_ACCESS_KEY                     *AccessKeyPtr,
  IN BOOLEAN                              WriteAccess,
  IN OUT BOOLEAN                          *Result
);

///
/// This protocol is used to provide a callback functions and event others can use
///
struct _Demo1_Access_Key_PROTOCOL {
  DRVR_FUNC_CB_TYPE1                      Demo1GenerateAccessKey;
  DRVR_FUNC_CB_TYPE2                      Demo1ValidateAccessKey;
  EFI_EVENT                               Demo1_Ready_To_Lock_Event;
};

///
/// Prototypes for the implementation of the callback functions in the custom protocol
///
EFI_STATUS
EFIAPI
Demo1GenerateAccessKey(
  IN Demo1_Access_Key_PROTOCOL            *This,
  IN EFI_HANDLE                           Controller,
  IN BOOLEAN                              WriteAccess,
  IN OUT DEMO1_ACCESS_KEY                 *AccessKeyPtr
);

EFI_STATUS
EFIAPI
Demo1ValidateAccessKey (
  IN Demo1_Access_Key_PROTOCOL            *This,
  IN EFI_HANDLE                           Controller,
  IN DEMO1_ACCESS_KEY                     *AccessKeyPtr,
  IN BOOLEAN                              WriteAccess,
  IN OUT BOOLEAN                          *Result
);

#endif
