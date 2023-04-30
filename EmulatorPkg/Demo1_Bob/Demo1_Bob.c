/** @file

Module Name:

  Demo1_Bob.c

Abstract:

  Bob driver works with Alice driver

Revision History: 0.1
**/

#include "Demo1_Bob.h"
#include <Protocol/LoadedImage.h>

// PRODUCED
Demo1_Bob_PROTOCOL
gDemo1_Bob_Protocol = {
  Demo1BobDataProvider,
};

// CONSUMED
Demo1_Access_Key_PROTOCOL  *AccessKeyProtocol = NULL;
Demo1_Alice_PROTOCOL *AliceProtocol;

// GLOBALS
DEMO1_ACCESS_KEY bobKey;
EFI_EVENT Demo1_Bob_PeriodicTimer = NULL;
UINTN DataToProvide = 0;
EFI_LOADED_IMAGE_PROTOCOL *gLoadImage = NULL;

/**
  Handler for Bob when the data provided by Alice is of type INIT Mode.
  The format is expected to be a function pointer that Bob must call.

  @param[in] Controller           Handle for controller
  @param[in] Data                 Function Pointer to call received from Alice

  @retval VOID
**/
STATIC
VOID
EFIAPI
Demo1BobInitModeAction(
  IN EFI_HANDLE             Controller,
  IN VOID                   *Data()
  )
{
  (*Data)();
}

/**
  Handler for Bob when the data provided by Alice is of type Run Mode.
  The format is expected to be a 64 bit integer.

  @param[in] Controller           Handle for controller
  @param[in] Data                 Points to a 64-bit integer

  @retval VOID
**/
STATIC
VOID
EFIAPI
Demo1BobRunModeAction(
  IN EFI_HANDLE             Controller,
  IN VOID                   *Data
  )
{
  DataToProvide = *(UINTN *)Data;
}

/**
  Handler for Bob's timer event. Each time this event is triggered Bob checks if
  we are in Init or Run mode. Bob then reads data from Alice an interprets the data
  according to the current mode.

  @param[in] Event                Event structure
  @param[in] Context              Context pointer

  @retval VOID
**/
STATIC
VOID
EFIAPI
Demo1BobTimerHandler (
  IN EFI_EVENT              Event,
  IN VOID                   *Context
  )
{
  static UINTN change=0;
  UINTN   Data=0;
  UINTN   Mode = 0;
  UINTN   BufferSize = sizeof(Mode);

  //
  // Get Alice_Mode Variable
  //
  EFI_STATUS Status = gST->RuntimeServices->GetAccessVariable (
    ALICEMODE_VARNAME,
    &gAliceVariableGuid,
    NULL,
    &bobKey,
    &BufferSize,
    &Mode
  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a: variable '%s' could not be read - bailing!\n", 
      __FUNCTION__, ALICEMODE_VARNAME));
    return;
  }

  //
  // Perform Run Action
  //
  if (Mode == RUN_MODE) {
    AliceProtocol->Demo1AliceProvideData(AliceProtocol, NULL, &Data);
    Demo1BobRunModeAction(NULL, (VOID *)&Data);
    return;
  }
  //
  // Perform Init Action
  //
  if (Mode != INIT_MODE) {
    return;
  } else {
    AliceProtocol->Demo1AliceProvideData(AliceProtocol, NULL, &Data);
    Demo1BobInitModeAction(NULL, (VOID*)Data);

    if (change == 0) {
      change = 1;
      //
      // Change to 5 second timer
      //
      Status = gBS->SetTimer (
        Demo1_Bob_PeriodicTimer,        // Event
        TimerPeriodic,                  // Type
        EFI_TIMER_PERIOD_SECONDS(5));   // Period
      if (EFI_ERROR (Status)) {
        DEBUG ((DEBUG_ERROR, "%a:  could not change timer - bailing!\n", 
          __FUNCTION__));
        ASSERT_EFI_ERROR (EFI_ABORTED);
      }
      //
      // Notify ReadyToRun
      //
      gBS->SignalEvent(AliceProtocol->Demo1_Ready_To_Run_Event);
    }
  }
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
Demo1BobInit (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS        Status;
  UINTN             Mode;
  UINTN             BufferSize = sizeof(Mode);

  // Open the LoadImage protocol to get access to the base and size.
  Status = gBS->OpenProtocol (
    ImageHandle,
    &gEfiLoadedImageProtocolGuid,
    (VOID **)&gLoadImage,
    gImageHandle,
    NULL,
    EFI_OPEN_PROTOCOL_GET_PROTOCOL
    );

  //
  // Get Access Key protocol
  //
  Status = gBS->LocateProtocol (&gDemo1AccessKeyProtocolGuid, NULL, (VOID **)&AccessKeyProtocol);
  if (EFI_ERROR (Status) || (AccessKeyProtocol == NULL)) {
    DEBUG ((DEBUG_ERROR, "%a: Could not locate Access Key protocol, Status = %r\n", 
      __FUNCTION__, Status));
    return Status;
  }

  //
  // Create Bob Key
  //
  Status = AccessKeyProtocol->Demo1GenerateAccessKey(NULL, NULL, FALSE, &bobKey);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a: Could not generate key, Status = %r\n", 
      __FUNCTION__, Status));
    return Status;
  }

  //
  // Get Alice Driver Protocol
  //
  Status = gBS->LocateProtocol(&gDemo1AliceProtocolGuid, NULL, (VOID *)&AliceProtocol);
  if (EFI_ERROR (Status) || (AliceProtocol == NULL)) {
    DEBUG ((DEBUG_ERROR, "%a: Could not locate Alice protocol, Status = %r\n", 
      __FUNCTION__, Status));
    return Status;
  }

  //
  // Get Alice Driver Mode
  //
  Status = SystemTable->RuntimeServices->GetAccessVariable (
    ALICEMODE_VARNAME,
    &gAliceVariableGuid,
    NULL,
    &bobKey,
    &BufferSize,
    &Mode
  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a: variable '%s' could not be read - bailing!\n", 
      __FUNCTION__, ALICEMODE_VARNAME));
    return Status;
  }

  //
  // Validate Alice Driver Mode
  //
  if (Mode == RUN_MODE) {
    DEBUG ((DEBUG_ERROR, "%a: Alice is already running, quitting\n",
      __FUNCTION__));
    return EFI_ALREADY_STARTED;
  }
  if (Mode != INIT_MODE) {
    DEBUG ((DEBUG_ERROR, "%a: Alice returned invalid mode, quitting\n",
      __FUNCTION__));
    return EFI_UNSUPPORTED;
  }
  // This must be INIT_MODE - continue

  //
  // Create a timer event 
  //
  Status = gBS->CreateEvent (
    EVT_TIMER | EVT_NOTIFY_SIGNAL,  // Type
    TPL_NOTIFY,                     // NotifyTpl
    Demo1BobTimerHandler,           // NotifyFunction
    NULL,                           // NotifyContext
    &Demo1_Bob_PeriodicTimer        // Event
  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a: Could not create event timer, Status = %r\n", 
      __FUNCTION__, Status));
    return Status;
  }

  //
  // Start timer
  //
  Status = gBS->SetTimer (
    Demo1_Bob_PeriodicTimer,        // Event
    TimerPeriodic,                  // Type
    EFI_TIMER_PERIOD_SECONDS(1));   // Period
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Install Bob Protocol
  //
  Status = gBS->InstallProtocolInterface (
    &ImageHandle,
    &gDemo1BobProtocolGuid,
    EFI_NATIVE_INTERFACE,
    &gDemo1_Bob_Protocol
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
Demo1BobUnload (
  IN EFI_HANDLE  ImageHandle
  )
{
  EFI_STATUS Status = EFI_SUCCESS;
  Status = gBS->UninstallProtocolInterface(
    &ImageHandle,
    &gDemo1BobProtocolGuid,
    EFI_NATIVE_INTERFACE
  );
  return Status;
}

/**
  API call for Bob to retrieve data from Alice.

  @param[in]  This              Bob Protocol structure
  @param[in]  Address           Pointer within Bob from which to read
  @param[in]  Dest              Pointer to the where the allocated destination buffer will be stored.
  @param[in]  Size              Length of the data to read.

  @retval EFI_SUCCESS           Successfully read and stored the data
  @retval EFI_ACCESS_DENIED     Invalid memory range
  @retval EFI_INVALID_PARAMETER Destination pointer is invalid.
**/
EFI_STATUS
EFIAPI
Demo1BobDataProvider(
  IN Demo1_Bob_PROTOCOL     *This,
  IN VOID                   *Address,
  IN VOID                   **Dest,
  IN UINTN                  Size
)
{

  // Used for comparison checks
  UINTN IAddress = (UINTN)Address;
  UINTN IBase = (UINTN)gLoadImage->ImageBase;
  VOID *Storage = NULL;

  if (Dest == NULL ) {
    return EFI_INVALID_PARAMETER;
  }

  if ( IAddress < IBase ) {
    return EFI_ACCESS_DENIED;
  }

  if ( IBase + gLoadImage->ImageSize < IAddress + Size ) {
    return EFI_ACCESS_DENIED;
  }

  Storage = AllocatePool(Size);

  if ( Storage == NULL ) {
    return EFI_INVALID_PARAMETER;
  }

  CopyMem( Storage, Address, Size);

  *Dest = Storage;

  return EFI_SUCCESS;
}
