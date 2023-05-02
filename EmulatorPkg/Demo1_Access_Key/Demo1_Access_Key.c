/** @file

Module Name:

  Demo1_Access_Key.c

Abstract:

  Provides protocol for generating/validating an access key

Revision History: 0.1
**/

#include "Demo1_Access_Key.h"

// PRODUCED
Demo1_Access_Key_PROTOCOL
gDemo1_Access_Key_Protocol = {
  Demo1GenerateAccessKey,
  Demo1ValidateAccessKey,
};

// CONSUMED
EFI_RNG_PROTOCOL  *RngProtocol = NULL;
_Ptr<DEMO1_ACCESS_KEY>  masterKey = NULL;

// GLOBALS
BOOLEAN accessKeyLock = FALSE;

// ACCESS KEY STORAGE
typedef struct _LINK LINK;

struct _LINK { // doubly linked list of keys
  DEMO1_ACCESS_KEY access_key;
  _Ptr<LINK> next;
  _Ptr<LINK> prev;
};

_Ptr<LINK> head = NULL;
_Ptr<LINK> last = NULL;
_Ptr<LINK> keychain=NULL;

///
/// Utility Functions for keychain
///

/**
  Utility function indicating if the list is empty

  @retval TRUE                    The keychain list is empty
  @retval FALSE                   The keychain list is non-empty
**/
BOOLEAN IsKeychainEmpty(
  VOID
  )
{
   return head == NULL;
}

/**
  Utility function indicating the length of the list

  @retval UINTN                   The number of entries in the list
**/
UINTN KeychainLength (
  VOID
  )
{
  UINTN length = 0;
  _Ptr<LINK> current = NULL;
  for(current = head; current != NULL; current = current->next){
    length++;
  }
  return length;
}

/**
  Inserts a link at the head of the list

  @param[in]  access_key          Pointer to a valid DEMO1_ACCESS_KEY structure

  @retval VOID
**/
void InsertFirst (
  _Ptr<DEMO1_ACCESS_KEY>                     access_key
  )
{
  _Ptr<LINK> link = AllocatePool(sizeof(LINK)); // create a link
  ASSERT (link != NULL);
  CopyMem(&link->access_key, access_key, KEYSIZE);

  if (IsKeychainEmpty()) { // make it the last link
    last = link;
  } else {
    head->prev = link; // update first prev link
  }
  link->prev = NULL;
  link->next = head; // point it to old first link
  head = link; // point first to new first link
}

/**
  Inserts a link at the tail of the list

  @param[in]  access_key          Pointer to a valid DEMO1_ACCESS_KEY structure

  @retval VOID
**/
VOID InsertLast (
  _Ptr<DEMO1_ACCESS_KEY>                     access_key
  )
{
  _Ptr<LINK> link = AllocatePool(sizeof(LINK)); // create a link
  ASSERT (link != NULL);
  CopyMem(&link->access_key, access_key, KEYSIZE);

  if (IsKeychainEmpty()) { // make it the last link
    last = link;
  } else {
    last->next = link; // make link a new last link
    link->prev = last; // mark old last LINK as prev of new link
   }
  link->next = NULL;
  last = link; // point last to new last LINK
}

/**
  Test if the provided key exists in the key chain

  @param[in]  access_key          Pointer to a valid DEMO1_ACCESS_KEY structure

  @retval TRUE                    The key is valid and exists in the chain
  @retval FALSE                   The key is invalid or does not exist in the chain
**/
BOOLEAN DoesKeyExist (
  _Ptr<DEMO1_ACCESS_KEY>                     access_key
  )
{
  _Ptr<LINK> current = NULL;
  if (access_key == NULL) {
    return FALSE;
  }

  // loop over keychain
  for(current = head; current != NULL; current = current->next) {
    if (access_key->access_key_store[0] == current->access_key.access_key_store[0]) {
      return TRUE;
    }
  }
  return FALSE;
}

/**
  Event handle called when the INIT phase is complete and the Access Key Protocol 
  must cease generating new keys.

  @param[in]  Event               Event structure
  @param[in]  Context             Pointer to a context. (unused)

  @retval                         VOID
**/
STATIC
VOID
EFIAPI
ReadyToLock(
  IN EFI_EVENT                        Event,
  IN VOID                             *Context
  )
{
  accessKeyLock = TRUE;
  gBS->CloseEvent (Event);
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
Demo1AccessKeyInit (
  IN EFI_HANDLE                       ImageHandle,
  IN EFI_SYSTEM_TABLE                 *SystemTable
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
  // Create Master Key
  //
  masterKey = AllocatePool(sizeof(DEMO1_ACCESS_KEY));
  Status = Demo1GenerateAccessKey(&gDemo1_Access_Key_Protocol, NULL, TRUE, masterKey);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a: Could not generate master key, Status = %r\n",
      __FUNCTION__, Status));
    return Status;
  }

  //
  // Create an event using event group gDemo1AccessKeyReadyToLockGuid.
  //
  Status = gBS->CreateEventEx(
    EVT_NOTIFY_SIGNAL,                                      // Type
    TPL_NOTIFY,                                             // NotifyTpl
    ReadyToLock,                                            // NotifyFunction
    NULL,                                                   // NotifyContext
    &gDemo1AccessKeyReadyToLockGuid,                        // EventGroup
    &(gDemo1_Access_Key_Protocol.Demo1_Ready_To_Lock_Event) // Event
  );

  //
  // Install Access Key Protocol
  //
  Status = gBS->InstallProtocolInterface (
    &ImageHandle,
    &gDemo1AccessKeyProtocolGuid,
    EFI_NATIVE_INTERFACE,
    &gDemo1_Access_Key_Protocol
    );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a: Could not install Access Key Protocol, Status = %r\n",
      __FUNCTION__, Status));
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
Demo1AccessKeyUnload (
  IN EFI_HANDLE                       ImageHandle
  )
{
  EFI_STATUS Status = EFI_SUCCESS;
  Status = gBS->UninstallProtocolInterface (
    &ImageHandle,
    &gDemo1AccessKeyProtocolGuid,
    EFI_NATIVE_INTERFACE
  );
  FreePool(masterKey);
  return Status;
}


/**
  Generate Access Key Function.
  Format of Access Key bits 0-64 Random Unique Value, 65-127 Key Magic and Write or Read value

  @param[in] This                 Protocol
  @param[in] Controller           Handle for controller
  @param[in] WriteAccess          Boolean to declare key will be used to edit variable
                                  TRUE indicates that the key needs write access
                                  FALSE indicates that read only access is requested
  @param[in,out] AccessKeyPtr     Pointer to storage for access key, caller provided

  @retval EFI_SUCESS              The Access Key has been generated successfully.
  @retval EFI_INVALID_PARAMETER   No storage for key provided
  @retval EFI_WRITE_PROTECTED     The INIT stage is passed and no additional keys can be generated
**/
EFI_STATUS
EFIAPI
Demo1GenerateAccessKey(
  IN Demo1_Access_Key_PROTOCOL        *This,
  IN EFI_HANDLE                       Controller,
  IN BOOLEAN                          WriteAccess,
  IN OUT _Ptr<DEMO1_ACCESS_KEY>       AccessKeyPtr // caller provided storage
  )
{
  EFI_STATUS Status = EFI_SUCCESS;
  UINTN header=0;

  // Verify user has provided storage
  if (AccessKeyPtr == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  // Verify ReadyToLock event has not occurred
  if (accessKeyLock == TRUE) {
    return EFI_WRITE_PROTECTED;
  }

  // Generate random number
  Status = RngProtocol->GetRNG (RngProtocol, NULL, KEYSIZE, (UINT8 *)&AccessKeyPtr->access_key_store);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  // Define magic for key
  if (WriteAccess) {
    header = (ACCESS_KEY_MAGIC << MAGIC_SIZE) + WRITE_ACCESS;
  } else {
    header = (ACCESS_KEY_MAGIC << MAGIC_SIZE) + READ_ACCESS;
  }
  AccessKeyPtr->access_key_store[1] = header;

  // Store key in keychain
  if (AccessKeyPtr != masterKey) {
    InsertLast(AccessKeyPtr);
  } else {
    InsertFirst(AccessKeyPtr);
  }

  return Status;
}

/**
  Validate Access Key Function.

  @param[in] This                 Protocol
  @param[in] Controller           Handle for controller
  @param[in] AccessKeyPtr         Pointer to access key
  @param[in] WriteAccess          Type of access requested.
                                  TRUE indicates that the key needs write access
                                  FALSE indicates that read only access is requested
  @param[in,out] Result           Result of validation, return to caller. TRUE If valid, FALSE otherwise

  @retval EFI_SUCCESS             Checked key, result of validation in Result boolean
  @retval EFI_INVALID_PARAMETER   An argument from the caller was NULL
**/
EFI_STATUS
EFIAPI
Demo1ValidateAccessKey (
  IN Demo1_Access_Key_PROTOCOL        *This,
  IN EFI_HANDLE                       Controller,
  IN DEMO1_ACCESS_KEY*                AccessKeyPtr : itype(_Ptr<DEMO1_ACCESS_KEY>),
  IN BOOLEAN                          WriteAccess,
  IN OUT BOOLEAN                      *Result
  )
{
  if ( Result == NULL ) {
    return EFI_INVALID_PARAMETER;
  }
  *Result = FALSE;

  if (AccessKeyPtr == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  // Check key permissions.
  if ( WriteAccess && (AccessKeyPtr->access_key_store[1] == ((ACCESS_KEY_MAGIC << MAGIC_SIZE) | READ_ACCESS ) ) ) {
    return EFI_INVALID_PARAMETER;
  }

  *Result = DoesKeyExist(AccessKeyPtr);
  return EFI_SUCCESS;
}