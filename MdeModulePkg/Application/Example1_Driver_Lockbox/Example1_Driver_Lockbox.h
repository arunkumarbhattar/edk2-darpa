/** @file

Copyright (c) 2007 - 2017, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

Module Name:

  Example1_Driver_Lockbox.h

Abstract:


Revision History


**/

#ifndef _Example1_Driver_Lockbox_H_
#define _Example1_Driver_Lockbox_H_

#include <Uefi.h>
#include <Library/DebugLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PrintLib.h>
#include <Library/UefiLib.h>


#define LOCKED    1
#define UNLOCKED  0

/// A custom protocol that has a callback service
///
/// Global ID for the Component Name Protocol
///
#define Example1_Driver_Lockbox_PROTOCOL_GUID { 0xf3aae06f, 0x7716, 0x4722,{ 0xb6, 0x26, 0x02, 0x64, 0x79, 0xb9, 0xfa, 0x57 } }

typedef struct _Example1_Driver_Lockbox_PROTOCOL Example1_Driver_Lockbox_PROTOCOL;


///
/// This callback functions is specific to the Protocol calling it
///
typedef
EFI_STATUS
(EFIAPI *MY_DRIVER_FUNCTION_CB)(
  IN Example1_Driver_Lockbox_PROTOCOL       *This,
  IN EFI_HANDLE                       Controller,
  IN UINTN                            value
);

typedef
EFI_STATUS
(EFIAPI *MY_DRIVER_FUNCTION_CB2)(
  IN Example1_Driver_Lockbox_PROTOCOL       *This,
  IN EFI_HANDLE                       Controller,
  IN UINTN                            dest,  
  IN VOID                             *src, 
  IN UINTN                            length
);

typedef
EFI_STATUS
(EFIAPI *MY_DRIVER_FUNCTION_CB3)(
  IN Example1_Driver_Lockbox_PROTOCOL       *This,
  IN EFI_HANDLE                       Controller,
  IN OUT VOID                         **dest, 
  IN UINTN                            offset, 
  IN UINTN                            length
);

typedef
EFI_STATUS
(EFIAPI *MY_DRIVER_FUNCTION_CB4)(
  IN Example1_Driver_Lockbox_PROTOCOL       *This,
  IN EFI_HANDLE                       Controller,
  IN VOID                             *dest,  
  IN VOID                             *src, 
  IN UINTN                            length
);
///
/// This protocol is used to provide a callback function others can use
///
struct _Example1_Driver_Lockbox_PROTOCOL {
  MY_DRIVER_FUNCTION_CB           Example1_Driver_Lockbox_SetLockPin;
  MY_DRIVER_FUNCTION_CB2          Example1_Driver_Lockbox_WriteData_Wrapper;
  MY_DRIVER_FUNCTION_CB3          Example1_Driver_Lockbox_ReadData;
  MY_DRIVER_FUNCTION_CB4          Example1_Driver_Lockbox_WriteData;
};


///
/// A prototype for the implementation of the callback functions in the custom protocol
///

EFI_STATUS
EFIAPI 
Example1_Driver_Lockbox_SetLockPin(
  IN Example1_Driver_Lockbox_PROTOCOL        *This,
  IN EFI_HANDLE             Controller,
  IN UINTN value
  );


EFI_STATUS
EFIAPI
Example1_Driver_Lockbox_WriteData_Wrapper (
  IN Example1_Driver_Lockbox_PROTOCOL       *This,
  IN EFI_HANDLE                       Controller,
  IN UINTN offset, 
  IN VOID *src, 
  IN UINTN length
  );

EFI_STATUS
EFIAPI
Example1_Driver_Lockbox_WriteData (
  IN Example1_Driver_Lockbox_PROTOCOL       *This,
  IN EFI_HANDLE                       Controller,
  IN VOID                             *dest,
  IN VOID                             *src, 
  IN UINTN                            length
  );

EFI_STATUS
EFIAPI
Example1_Driver_Lockbox_ReadData (
  IN Example1_Driver_Lockbox_PROTOCOL       *This,
  IN EFI_HANDLE                       Controller,
  IN OUT VOID                         **dest, 
  IN UINTN                            offset,
  IN UINTN                            length
  );
#endif
