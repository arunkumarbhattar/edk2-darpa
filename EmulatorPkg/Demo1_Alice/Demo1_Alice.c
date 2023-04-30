/** @file

Module Name:

  Demo1_Alice.c

Abstract:

  Alice driver works with Bob driver

Revision History: 0.1
**/

#include "Demo1_Alice.h"

// PRODUCED
Demo1_Alice_PROTOCOL
gDemo1_Alice_Protocol = {
  Demo1AliceProvideData,
};

// CONSUMED
EFI_RNG_PROTOCOL *RngProtocol = NULL;
Demo1_Access_Key_PROTOCOL  *AccessKeyProtocol = NULL;

// GLOBALS
DEMO1_ACCESS_KEY *aliceKey = NULL;
UINTN Mode = INIT_MODE;

/**
  Callback function for ReadyToRun Event.

  @param[in] Event                Event structure
  @param[in] Context              Context pointer

  @retval VOID
**/
STATIC
VOID
EFIAPI
ReadyToRun(
  IN EFI_EVENT Event,
  IN VOID *Context)
{
  //
  // Notify ReadyToLock
  //
  EFI_STATUS Status = gBS->SignalEvent(AccessKeyProtocol->Demo1_Ready_To_Lock_Event);
  if (Status != 0) {
      DEBUG((DEBUG_ERROR, "Alice: Failed to lock access key\n"));
      ASSERT_EFI_ERROR (EFI_ABORTED);
  }

  //
  // Set Alice_Mode Variable
  //
  Mode=RUN_MODE;
  Status  = gST->RuntimeServices->SetAccessVariable (
    ALICEMODE_VARNAME,
    &gAliceVariableGuid,
    EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE,
    aliceKey,
    sizeof(UINTN),
    &Mode
  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a: variable '%s' could not be written - bailing!\n", 
      __FUNCTION__, ALICEMODE_VARNAME));
    ASSERT_EFI_ERROR (EFI_ABORTED);
  }

  gBS->CloseEvent (Event);
}

/**
  Function Alice provides during the Init phase

  @retval                         VOID

**/
STATIC
VOID
EFIAPI
AliceInitFunction(
  )
{
  // Does something
  DEBUG((DEBUG_INFO, "Alice: For now I just say this message\n"));
}

/**
  Main entry for this driver.

  @param[in] ImageHandle          Image handle this driver.
  @param[in] SystemTable          Pointer to SystemTable.

  @retval EFI_SUCCESS             Completed successfully.
  @retval Status                  Returns from other functions may fail, causing this function to fail
**/
EFI_STATUS
EFIAPI
Demo1AliceInit (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS        Status;

  //
  // Get Random Number Generator protocol
  //
  Status = gBS->LocateProtocol (&gEfiRngProtocolGuid, NULL, (VOID **)&RngProtocol);
  if (EFI_ERROR (Status) || (RngProtocol == NULL)) {
    DEBUG ((DEBUG_ERROR, "%a: Could not locate RNG prototocol, Status = %r\n", 
      __FUNCTION__, Status));
    return Status;
  }

  //
  // Get Access Key protocol 
  //
  Status = gBS->LocateProtocol (&gDemo1AccessKeyProtocolGuid, NULL, (VOID **)&AccessKeyProtocol);
  if (EFI_ERROR (Status) || (AccessKeyProtocol == NULL)) {
    DEBUG ((DEBUG_ERROR, "%a: Could not locate RNG prototocol, Status = %r\n", 
      __FUNCTION__, Status));
    return Status;
  }

  //
  // Create Alice Key
  //
  aliceKey = AllocatePool(sizeof(DEMO1_ACCESS_KEY));
  Status = AccessKeyProtocol->Demo1GenerateAccessKey(NULL, NULL, TRUE, aliceKey);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a: Could not generate the key, Status = %r\n", 
      __FUNCTION__, Status));
    return Status;
  }

  //
  // Create an event using event group gDemo1AliceReadyToRunGuid
  //
  Status = gBS->CreateEventEx(
    EVT_NOTIFY_SIGNAL,                                // Type
    TPL_NOTIFY,                                       // NotifyTpl
    ReadyToRun,                                       // NotifyFunction
    NULL,                                             // NotifyContext
    &gDemo1AliceReadyToRunGuid,                       // EventGroup
    &(gDemo1_Alice_Protocol.Demo1_Ready_To_Run_Event) // Event
  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a: Could not create event, Status = %r\n", 
      __FUNCTION__, Status));
    return Status;
  }

  //
  // Set Alice_Mode Variable
  //
  Status  = SystemTable->RuntimeServices->SetAccessVariable (
    ALICEMODE_VARNAME,
    &gAliceVariableGuid,
    EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE,
    aliceKey,
    sizeof(UINTN),
    &Mode
  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a: variable '%s' could not be written - bailing!\n", 
      __FUNCTION__, ALICEMODE_VARNAME));
    return Status;
  }

  //
  // Install Alice Protocol
  //
  Status = gBS->InstallProtocolInterface (
      &ImageHandle,
      &gDemo1AliceProtocolGuid,
      EFI_NATIVE_INTERFACE,
      &gDemo1_Alice_Protocol
    );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  return EFI_SUCCESS;
}

/**
  Unloads the application and its installed protocol.

  @param[in]  ImageHandle         Handle that identifies the image to be unloaded.

  @retval EFI_SUCCESS             The image has been unloaded.
  @retval Status                  Return from UninstallProtocolInterface
**/
EFI_STATUS
EFIAPI
Demo1AliceUnload (
  IN EFI_HANDLE  ImageHandle
  )
{
  EFI_STATUS Status = EFI_SUCCESS;
  Status = gBS->UninstallProtocolInterface (
    &ImageHandle,
    &gDemo1AliceProtocolGuid,
    EFI_NATIVE_INTERFACE
  );
  FreePool(aliceKey);
  return Status;
}

/**
  API call for Alice to give data based upon the current mode

  @param[in]  This                Alice Protocol structure
  @param[in]  Controller          Handle for controller
  @param[in,out]  Data            Pointer to hold the data output

  @retval EFI_SUCCESS             The Access Key has been generated successfully.
  @retval Status                  Return from GetRNG function

**/
EFI_STATUS
EFIAPI
Demo1AliceProvideData(
  IN Demo1_Alice_PROTOCOL   *This,
  IN EFI_HANDLE             Controller,
  IN OUT UINTN              *Data
)
{
  EFI_STATUS Status = EFI_SUCCESS;

  if (Data == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  if (Mode == INIT_MODE) {
    *Data = (UINTN)AliceInitFunction;
  }
  if (Mode == RUN_MODE) {
    Status = RngProtocol->GetRNG (RngProtocol, NULL, sizeof(UINTN), (UINT8 *)Data);;
  }
  return Status;
}
