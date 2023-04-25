/** @file
This is an example of how a driver might export data to the HII protocol to be
later utilized by the Setup Protocol

Copyright (c) 2004 - 2018, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "Example1_Driver_Lockbox.h"

///
/// SMM Lockbox
///
EFI_PHYSICAL_ADDRESS PhysicalBuffer;
UINTN Pages;
UINTN lockpin=UNLOCKED;
VOID *lockbox_start;
UINTN lockbox_length = SIZE_16KB;

Example1_Driver_Lockbox_PROTOCOL 
gExample1_Driver_LockboxProtocol = {
  Example1_Driver_Lockbox_SetLockPin,
  Example1_Driver_Lockbox_WriteData_Wrapper,
  Example1_Driver_Lockbox_ReadData,
  Example1_Driver_Lockbox_WriteData,
};

/**
  Main entry for this driver.

  @param ImageHandle     Image handle this driver.
  @param SystemTable     Pointer to SystemTable.

  @retval EFI_SUCESS     This function always complete successfully.

**/
EFI_STATUS
EFIAPI
Example1_Driver_LockboxInit (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS                             Status;

  lockpin = LOCKED;

  //CpuBreakpoint();

  //
  // Allocate the number of pages to hold Size bytes and
  // return in PhysicalBuffer
  //
  Pages = EFI_SIZE_TO_PAGES (SIZE_16KB);
  Status = gBS->AllocatePages (
                  AllocateAnyPages,
                  EfiBootServicesData,
                  Pages,
                  &PhysicalBuffer
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Convert the physical address to a pointer.
  // This method works for all support CPU architectures.
  //
  lockbox_start = (VOID *)(UINTN)PhysicalBuffer;

  Status = gBS->InstallProtocolInterface (
                  &ImageHandle,
                  &gExample1_Driver_LockboxProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  &gExample1_Driver_LockboxProtocol
                  );
  ASSERT_EFI_ERROR (Status);

  return EFI_SUCCESS;
}

/**
  Unloads the application and its installed protocol.

  @param[in]  ImageHandle       Handle that identifies the image to be unloaded.

  @retval EFI_SUCCESS           The image has been unloaded.
**/
EFI_STATUS
EFIAPI
Example1_Driver_LockboxUnload (
  IN EFI_HANDLE  ImageHandle
  )
{
  EFI_STATUS Status = EFI_SUCCESS;

  //
  // Free the allocated buffer
  //
  Status = gBS->FreePages (PhysicalBuffer, Pages);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  return Status;
}

EFI_STATUS
EFIAPI 
Example1_Driver_Lockbox_SetLockPin(
  IN Example1_Driver_Lockbox_PROTOCOL        *This,
  IN EFI_HANDLE             Controller,
  IN UINTN value
  ) 
{
	if ( lockpin != 0 && value == 0 ) {
		return EFI_ACCESS_DENIED;
	}
	lockpin = value;
	DEBUG((DEBUG_INFO, "Example1_Driver_Lockbox Set Lockpin(%ld)\n", value));
	return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
Example1_Driver_Lockbox_WriteData_Wrapper (
  IN Example1_Driver_Lockbox_PROTOCOL        *This,
  IN EFI_HANDLE             Controller,
  IN UINTN offset, 
  IN VOID *src,
  IN UINTN length
  )
{
  return Example1_Driver_Lockbox_WriteData(This, NULL, (void *)(lockbox_start+offset), src, length);
}

EFI_STATUS
EFIAPI
Example1_Driver_Lockbox_WriteData (
  IN Example1_Driver_Lockbox_PROTOCOL        *This,
  IN EFI_HANDLE             Controller,
  IN VOID *dest,
  IN VOID *src, 
  IN UINTN length
  )
{
	// Check if in lockbox
	if ( lockpin ) {
		// | lockbox start ----- dest ----- lockbox_start + length |
		if ( lockbox_start <= dest && dest < lockbox_start + lockbox_length ) {
		 	return EFI_WRITE_PROTECTED;
		}
	}
	CopyMem( dest, src, length);

	return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
Example1_Driver_Lockbox_ReadData (
  IN Example1_Driver_Lockbox_PROTOCOL       *This,
  IN EFI_HANDLE                       Controller,
  IN OUT VOID                         **dest, 
  IN UINTN                            offset, 
  IN UINTN                            length
  )
{
  // Check if pointer is provided
  if ((void*)*dest == NULL)
    return EFI_INVALID_PARAMETER;

	// Check if in lockbox
  void *src = (void*)(lockbox_start+offset);
		// | lockbox start ----- src ----- lockbox_start + length |
  if ( lockbox_start > src || src >= lockbox_start + lockbox_length ) {
    return EFI_NO_MAPPING;
  }
  if (src + length >= lockbox_start + lockbox_length ) {
    return EFI_NO_MAPPING;
  }

	CopyMem( (void*)*dest, src, length);

	return EFI_SUCCESS;
}
