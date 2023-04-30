/** @file

Module Name:

  Demo1_Variable.c

Abstract:

  Provides Set/Get Access Variable Functionality

Revision History: 0.1
**/

#include "Demo1_Variable.h"

// GLOBAL
EFI_GUID gAuthVarGUID       = { 0xaaf32c78, 0x947b, 0x439a, { 0xa1, 0x80, 0x2e, 0x14, 0x4e, 0xc3, 0x77, 0x92 } };
EFI_GUID gEfiVarGuid        = { 0xddcf3616, 0x3275, 0x4164, { 0x98, 0xb6, 0xfe, 0x85, 0x70, 0x7f, 0xfe, 0x7d }};
EFI_GUID gEfiGlobalVarGuid  = { 0x8BE4DF61, 0x93CA, 0x11D2, { 0xAA, 0x0D, 0x00, 0xE0, 0x98, 0x03, 0x2B, 0x8C }};

// Global variable for calling Access Key Protocol API functions
Demo1_Access_Key_PROTOCOL  *AccessKeyProtocol = NULL;

VARIABLE_MODULE_GLOBAL  *mineVariableModuleGlobal;

///
/// Define a memory cache that improves the search performance for a variable.
/// For EmuNvMode == TRUE, it will be equal to NonVolatileVariableBase.
///
VARIABLE_STORE_HEADER  *mineNvVariableCache = NULL;
///
/// The memory entry used for variable statistics data.
///
VARIABLE_INFO_ENTRY  *gVarInfo = NULL;

/**
  This code checks if variable header is valid or not.

  @param[in] Variable             Pointer to the Variable Header.
  @param[in] VariableStoreEnd     Pointer to the Variable Store End.

  @retval TRUE                    Variable header is valid.
  @retval FALSE                   Variable header is not valid.

**/
BOOLEAN
IsValidAccessVariableHeader (
  IN  ACCESS_VARIABLE_HEADER          *Variable,
  IN  ACCESS_VARIABLE_HEADER          *VariableStoreEnd
  )
{
  if ((Variable == NULL) || (Variable >= VariableStoreEnd) || (Variable->StartId != VARIABLE_DATA)) {
    //
    // Variable is NULL or has reached the end of variable store,
    // or the StartId is not correct.
    //
    return FALSE;
  }
  return TRUE;
}

/**
  This code gets the size of variable data.

  @param[in]  Variable            Pointer to the Variable Header.
  @param[in]  AuthFormat          TRUE indicates authenticated variables are used.
                                  FALSE indicates authenticated variables are not used.

  @return                         Size of variable in bytes.

**/
UINTN
DataSizeOfAccessVariable (
  IN  ACCESS_VARIABLE_HEADER          *Variable,
  IN  BOOLEAN                         AuthFormat
  )
{
  AUTHENTICATED_VARIABLE_HEADER  *AuthVariable;

  AuthVariable = (AUTHENTICATED_VARIABLE_HEADER *)Variable;
  if (AuthFormat) {
    if ((AuthVariable->State == (UINT8)(-1)) ||
        (AuthVariable->DataSize == (UINT32)(-1)) ||
        (AuthVariable->NameSize == (UINT32)(-1)) ||
        (AuthVariable->Attributes == (UINT32)(-1)))
    {
      return 0;
    }

    return (UINTN)AuthVariable->DataSize;
  } else {
    if ((Variable->State == (UINT8)(-1)) ||
        (Variable->DataSize == (UINT32)(-1)) ||
        (Variable->NameSize == (UINT32)(-1)) ||
        (Variable->Attributes == (UINT32)(-1)))
    {
      return 0;
    }

    return (UINTN)Variable->DataSize;
  }
}

/**
  This code gets the size of name of variable.

  @param[in]  Variable            Pointer to the variable header.
  @param[in]  AuthFormat          TRUE indicates authenticated variables are used.
                                  FALSE indicates authenticated variables are not used.

  @return UINTN                   Size of variable in bytes.
**/
UINTN
NameSizeOfAccessVariable (
  IN  ACCESS_VARIABLE_HEADER          *Variable,
  IN  BOOLEAN                         AuthFormat
  )
{
  AUTHENTICATED_VARIABLE_HEADER  *AuthVariable;

  AuthVariable = (AUTHENTICATED_VARIABLE_HEADER *)Variable;
  if (AuthFormat) {
    if ((AuthVariable->State == (UINT8)(-1)) ||
        (AuthVariable->DataSize == (UINT32)(-1)) ||
        (AuthVariable->NameSize == (UINT32)(-1)) ||
        (AuthVariable->Attributes == (UINT32)(-1)))
    {
      return 0;
    }

    return (UINTN)AuthVariable->NameSize;
  } else {
    if ((Variable->State == (UINT8)(-1)) ||
        (Variable->DataSize == (UINT32)(-1)) ||
        (Variable->NameSize == (UINT32)(-1)) ||
        (Variable->Attributes == (UINT32)(-1)))
    {
      return 0;
    }
    return (UINTN)Variable->NameSize;
  }
}

/**
  This code gets the size of variable header.

  @param[in]  AuthFormat          TRUE indicates authenticated variables are used.
                                  FALSE indicates authenticated variables are not used.

  @return                         Size of variable header in bytes in type UINTN.

**/
UINTN
GetAccessVariableHeaderSize (
  IN  BOOLEAN                         AuthFormat
  )
{
  UINTN  Value;

  if (AuthFormat) {
    Value = sizeof (AUTHENTICATED_VARIABLE_HEADER);
  } else {
    Value = sizeof (ACCESS_VARIABLE_HEADER);
  }
  return Value;
}

/**
  This code gets the pointer to the variable name.

  @param[in] Variable             Pointer to the Variable Header.
  @param[in] AuthFormat           TRUE indicates authenticated variables are used.
                                  FALSE indicates authenticated variables are not used.

  @return                         Pointer to Variable Name which is Unicode encoding.

**/
CHAR16 *
GetAccessVariableNamePtr (
  IN  ACCESS_VARIABLE_HEADER          *Variable,
  IN  BOOLEAN                         AuthFormat
  )
{
  return (CHAR16 *)((UINTN)Variable + GetAccessVariableHeaderSize (AuthFormat));
}

/**
  This code gets the pointer to the variable data.

  @param[in] Variable             Pointer to the Variable Header.
  @param[in] AuthFormat           TRUE indicates authenticated variables are used.
                                  FALSE indicates authenticated variables are not used.

  @return                         Pointer to Variable Data.

**/
UINT8 *
GetAccessVariableDataPtr (
  IN  ACCESS_VARIABLE_HEADER          *Variable,
  IN  BOOLEAN                         AuthFormat
  )
{
  UINTN  Value;

  //
  // Be careful about pad size for alignment.
  //
  Value  =  (UINTN)GetAccessVariableNamePtr (Variable, AuthFormat);
  Value += NameSizeOfAccessVariable (Variable, AuthFormat);
  Value += GET_PAD_SIZE (NameSizeOfAccessVariable (Variable, AuthFormat));

  return (UINT8 *)Value;
}

/**
  This code gets the pointer to the next variable header.

  @param[in] Variable             Pointer to the Variable Header.
  @param[in] AuthFormat           TRUE indicates authenticated variables are used.
                                  FALSE indicates authenticated variables are not used.

  @return                         Pointer to next variable header.

**/
ACCESS_VARIABLE_HEADER *
GetNextAccessVariablePtr (
  IN  ACCESS_VARIABLE_HEADER          *Variable,
  IN  BOOLEAN                         AuthFormat
  )
{
  UINTN  Value;

  Value  =  (UINTN)GetAccessVariableDataPtr (Variable, AuthFormat);
  Value += DataSizeOfAccessVariable (Variable, AuthFormat);
  Value += GET_PAD_SIZE (DataSizeOfAccessVariable (Variable, AuthFormat));

  //
  // Be careful about pad size for alignment.
  //
  return (ACCESS_VARIABLE_HEADER *)HEADER_ALIGN (Value);
}

/**
  Gets the pointer to the first variable header in given variable store area.

  @param[in] VarStoreHeader       Pointer to the Variable Store Header.

  @return                         Pointer to the first variable header.

**/
ACCESS_VARIABLE_HEADER *
GetAccessStartPointer (
  IN VARIABLE_STORE_HEADER            *VarStoreHeader
  )
{
  //
  // The start of variable store.
  //
  return (ACCESS_VARIABLE_HEADER *)HEADER_ALIGN (VarStoreHeader + 1);
}

/**
  Gets the pointer to the end of the variable storage area.

  This function gets pointer to the end of the variable storage
  area, according to the input variable store header.

  @param[in] VarStoreHeader       Pointer to the Variable Store Header.

  @return                         Pointer to the end of the variable storage area.
**/
ACCESS_VARIABLE_HEADER *
GetAccessEndPointer (
  IN VARIABLE_STORE_HEADER            *VarStoreHeader
  )
{
  //
  // The end of variable store
  //
  return (ACCESS_VARIABLE_HEADER *)HEADER_ALIGN ((UINTN)VarStoreHeader + VarStoreHeader->Size);
}

/**
  Init emulated non-volatile variable store.

  @param[out] VariableStoreBase   Output pointer to emulated non-volatile variable store base.

  @retval EFI_SUCCESS             Function successfully executed.
  @retval EFI_OUT_OF_RESOURCES    Fail to allocate enough memory resource.

**/
EFI_STATUS
mineInitEmuNonVolatileVariableStore (
  OUT EFI_PHYSICAL_ADDRESS            *VariableStoreBase
  )
{
  VARIABLE_STORE_HEADER  *VariableStore;
  UINT32                 VariableStoreLength=0x8000;
  ASSERT (sizeof (VARIABLE_STORE_HEADER) <= VariableStoreLength);

  //
  // Allocate memory for variable store.
  //
    VariableStore = (VARIABLE_STORE_HEADER *)AllocateRuntimePool (VariableStoreLength); // 32KB
    if (VariableStore == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }

  SetMem (VariableStore, VariableStoreLength, 0xff);

  //
  // Use gAuthVarGUID for potential auth variable support.
  //
  CopyGuid (&VariableStore->Signature, &gAuthVarGUID);
  VariableStore->Size      = VariableStoreLength;
  VariableStore->Format    = VARIABLE_STORE_FORMATTED;
  VariableStore->State     = VARIABLE_STORE_HEALTHY;
  VariableStore->Reserved  = 0;
  VariableStore->Reserved1 = 0;
  
  *VariableStoreBase = (EFI_PHYSICAL_ADDRESS)(UINTN)VariableStore;

  return EFI_SUCCESS;
}

/**
  Init non-volatile variable store.

  @retval EFI_SUCCESS             Function successfully executed.
  @retval EFI_OUT_OF_RESOURCES    Fail to allocate enough memory resource.
  @retval EFI_VOLUME_CORRUPTED    Variable Store or Firmware Volume for Variable Store is corrupted.

**/
EFI_STATUS
mineInitNonVolatileVariableStore (
  VOID
  )
{
  ACCESS_VARIABLE_HEADER       *Variable;
  ACCESS_VARIABLE_HEADER       *NextVariable;
  EFI_PHYSICAL_ADDRESS  VariableStoreBase;
  UINTN                 VariableSize;
  EFI_STATUS            Status;

  Status = mineInitEmuNonVolatileVariableStore (&VariableStoreBase);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  mineVariableModuleGlobal->VariableGlobal.EmuNvMode = TRUE;

  mineVariableModuleGlobal->VariableGlobal.NonVolatileVariableBase = VariableStoreBase;
  mineNvVariableCache                                              = (VARIABLE_STORE_HEADER *)(UINTN)VariableStoreBase;
  mineVariableModuleGlobal->VariableGlobal.AuthFormat              = FALSE; 
  mineVariableModuleGlobal->MaxVariableSize     = 0x2000;
  mineVariableModuleGlobal->MaxAuthVariableSize = 0x2800;

  //
  // Parse non-volatile variable data and get last variable offset.
  //
  Variable = GetAccessStartPointer (mineNvVariableCache);
  while (IsValidAccessVariableHeader (Variable, GetAccessEndPointer (mineNvVariableCache))) {
    NextVariable = GetNextAccessVariablePtr (Variable, mineVariableModuleGlobal->VariableGlobal.AuthFormat);
    VariableSize = (UINTN)NextVariable - (UINTN)Variable;
    if ((Variable->Attributes & (EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_HARDWARE_ERROR_RECORD)) == (EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_HARDWARE_ERROR_RECORD)) {
      mineVariableModuleGlobal->HwErrVariableTotalSize += VariableSize;
    } else {
      mineVariableModuleGlobal->CommonVariableTotalSize += VariableSize;
    }

    Variable = NextVariable;
  }

  mineVariableModuleGlobal->NonVolatileLastVariableOffset = (UINTN)Variable - (UINTN)mineNvVariableCache;
  return EFI_SUCCESS;
}

/**
  This code gets the pointer to the variable guid.

  @param[in] Variable             Pointer to the Variable Header.
  @param[in] AuthFormat           TRUE indicates authenticated variables are used.
                                  FALSE indicates authenticated variables are not used.

  @return                         A EFI_GUID* pointer to Vendor Guid.

**/
EFI_GUID *
GetAccessVendorGuidPtr (
  IN  ACCESS_VARIABLE_HEADER          *Variable,
  IN  BOOLEAN                         AuthFormat
  )
{
  AUTHENTICATED_VARIABLE_HEADER  *AuthVariable;

  AuthVariable = (AUTHENTICATED_VARIABLE_HEADER *)Variable;
  if (AuthFormat) {
    return &AuthVariable->VendorGuid;
  } else {
    return &Variable->VendorGuid;
  }
}

/**
  Return TRUE if ExitBootServices () has been called.

  @retval                         TRUE If ExitBootServices () has been called. 
                                  FALSE if ExitBootServices () has not been called.
**/
BOOLEAN
AtRuntime (
  VOID
  )
{
  return FALSE;
}

/**
  Routine used to track statistical information about variable usage.
  The data is stored in the EFI system table so it can be accessed later.
  VariableInfo.efi can dump out the table. Only Boot Services variable
  accesses are tracked by this code. The PcdVariableCollectStatistics
  build flag controls if this feature is enabled.

  A read that hits in the cache will have Read and Cache true for
  the transaction. Data is allocated by this routine, but never
  freed.

  @param[in]      VariableName    Name of the Variable to track.
  @param[in]      VendorGuid      Guid of the Variable to track.
  @param[in]      Volatile        TRUE if volatile FALSE if non-volatile.
  @param[in]      Read            TRUE if GetVariable() was called.
  @param[in]      Write           TRUE if SetVariable() was called.
  @param[in]      Delete          TRUE if deleted via SetVariable().
  @param[in]      Cache           TRUE for a cache hit.
  @param[in,out]  VariableInfo    Pointer to a pointer of VARIABLE_INFO_ENTRY structures.

**/
VOID
UpdateAccessVariableInfo (
  IN  CHAR16                          *VariableName,
  IN  EFI_GUID                        *VendorGuid,
  IN  BOOLEAN                         Volatile,
  IN  BOOLEAN                         Read,
  IN  BOOLEAN                         Write,
  IN  BOOLEAN                         Delete,
  IN  BOOLEAN                         Cache,
  IN OUT VARIABLE_INFO_ENTRY          **VariableInfo
  )
{
  VARIABLE_INFO_ENTRY  *Entry;

  if (TRUE) {
    if ((VariableName == NULL) || (VendorGuid == NULL) || (VariableInfo == NULL)) {
      return;
    }

    if (AtRuntime ()) {
      // Don't collect statistics at runtime.
      return;
    }

    if (*VariableInfo == NULL) {
      //
      // On the first call allocate a entry and place a pointer to it in
      // the EFI System Table.
      //
      *VariableInfo = AllocateZeroPool (sizeof (VARIABLE_INFO_ENTRY));
      ASSERT (*VariableInfo != NULL);

      CopyGuid (&(*VariableInfo)->VendorGuid, VendorGuid);
      (*VariableInfo)->Name = AllocateZeroPool (StrSize (VariableName));
      ASSERT ((*VariableInfo)->Name != NULL);
      StrCpyS ((*VariableInfo)->Name, StrSize (VariableName)/sizeof (CHAR16), VariableName);
      (*VariableInfo)->Volatile = Volatile;
    }

    for (Entry = (*VariableInfo); Entry != NULL; Entry = Entry->Next) {
      if (CompareGuid (VendorGuid, &Entry->VendorGuid)) {
        if (StrCmp (VariableName, Entry->Name) == 0) {
          if (Read) {
            Entry->ReadCount++;
          }

          if (Write) {
            Entry->WriteCount++;
          }

          if (Delete) {
            Entry->DeleteCount++;
          }

          if (Cache) {
            Entry->CacheCount++;
          }

          return;
        }
      }

      if (Entry->Next == NULL) {
        //
        // If the entry is not in the table add it.
        // Next iteration of the loop will fill in the data.
        //
        Entry->Next = AllocateZeroPool (sizeof (VARIABLE_INFO_ENTRY));
        ASSERT (Entry->Next != NULL);

        CopyGuid (&Entry->Next->VendorGuid, VendorGuid);
        Entry->Next->Name = AllocateZeroPool (StrSize (VariableName));
        ASSERT (Entry->Next->Name != NULL);
        StrCpyS (Entry->Next->Name, StrSize (VariableName)/sizeof (CHAR16), VariableName);
        Entry->Next->Volatile = Volatile;
      }
    }
  }
}

/**
  Compare two EFI_TIME data.

  @param[in] FirstTime            A pointer to the first EFI_TIME data.
  @param[in] SecondTime           A pointer to the second EFI_TIME data.

  @retval  TRUE                   The FirstTime is not later than the SecondTime.
  @retval  FALSE                  The FirstTime is later than the SecondTime.

**/
BOOLEAN
AccessVariableCompareTimeStampInternal (
  IN EFI_TIME  *FirstTime,
  IN EFI_TIME  *SecondTime
  )
{
  if (FirstTime->Year != SecondTime->Year) {
    return (BOOLEAN)(FirstTime->Year < SecondTime->Year);
  } else if (FirstTime->Month != SecondTime->Month) {
    return (BOOLEAN)(FirstTime->Month < SecondTime->Month);
  } else if (FirstTime->Day != SecondTime->Day) {
    return (BOOLEAN)(FirstTime->Day < SecondTime->Day);
  } else if (FirstTime->Hour != SecondTime->Hour) {
    return (BOOLEAN)(FirstTime->Hour < SecondTime->Hour);
  } else if (FirstTime->Minute != SecondTime->Minute) {
    return (BOOLEAN)(FirstTime->Minute < SecondTime->Minute);
  }

  return (BOOLEAN)(FirstTime->Second <= SecondTime->Second);
}

/**
  This code sets the size of name of variable.

  @param[in]  Variable            Pointer to the Variable Header.
  @param[in]  NameSize            Name size to set.
  @param[in]  AuthFormat          TRUE indicates authenticated variables are used.
                                  FALSE indicates authenticated variables are not used.

**/
VOID
SetNameSizeOfAccessVariable (
  IN ACCESS_VARIABLE_HEADER           *Variable,
  IN UINTN                            NameSize,
  IN BOOLEAN                          AuthFormat
  )
{
  AUTHENTICATED_VARIABLE_HEADER  *AuthVariable;

  AuthVariable = (AUTHENTICATED_VARIABLE_HEADER *)Variable;
  if (AuthFormat) {
    AuthVariable->NameSize = (UINT32)NameSize;
  } else {
    Variable->NameSize = (UINT32)NameSize;
  }
}

/**
  This code sets the size of variable data.

  @param[in] Variable             Pointer to the Variable Header.
  @param[in] DataSize             Data size to set.
  @param[in] AuthFormat           TRUE indicates authenticated variables are used.
                                  FALSE indicates authenticated variables are not used.

**/
VOID
SetDataSizeOfAccessVariable (
  IN  ACCESS_VARIABLE_HEADER          *Variable,
  IN  UINTN                           DataSize,
  IN  BOOLEAN                         AuthFormat
  )
{
  AUTHENTICATED_VARIABLE_HEADER  *AuthVariable;

  AuthVariable = (AUTHENTICATED_VARIABLE_HEADER *)Variable;
  if (AuthFormat) {
    AuthVariable->DataSize = (UINT32)DataSize;
  } else {
    Variable->DataSize = (UINT32)DataSize;
  }
}

/**

  This function writes data to the FWH at the correct LBA even if the LBAs
  are fragmented.

  @param Global                   Pointer to VARAIBLE_GLOBAL structure.
  @param Volatile                 Point out the Variable is Volatile or Non-Volatile.
  @param SetByIndex               TRUE if target pointer is given as index.
                                  FALSE if target pointer is absolute.
  @param Fvb                      Pointer to the writable FVB protocol.
  @param DataPtrIndex             Pointer to the Data from the end of VARIABLE_STORE_HEADER
                                  structure.
  @param DataSize                 Size of data to be written.
  @param Buffer                   Pointer to the buffer from which data is written.

  @retval EFI_INVALID_PARAMETER   Parameters not valid.
  @retval EFI_UNSUPPORTED         Fvb is a NULL for Non-Volatile variable update.
  @retval EFI_OUT_OF_RESOURCES    The remaining size is not enough.
  @retval EFI_SUCCESS             Variable store successfully updated.

**/
EFI_STATUS
UpdateAccessVariableStore (
  IN  VARIABLE_GLOBAL                     *Global,
  IN  BOOLEAN                             Volatile,
  IN  BOOLEAN                             SetByIndex,
  IN  EFI_FIRMWARE_VOLUME_BLOCK_PROTOCOL  *Fvb,
  IN  UINTN                               DataPtrIndex,
  IN  UINT32                              DataSize,
  IN  UINT8                               *Buffer
  )
{
  EFI_PHYSICAL_ADDRESS    DataPtr = DataPtrIndex;

  //
  // Emulated non-volatile variable mode.
  //
  if (SetByIndex) {
    DataPtr += (UINTN)mineNvVariableCache;
  }

  if ((DataPtr + DataSize) > ((UINTN)mineNvVariableCache + mineNvVariableCache->Size)) {
    return EFI_OUT_OF_RESOURCES;
  }
  //
  // If Emulated Non-volatile Variable just do a simple mem copy.
  //
  CopyMem ((UINT8 *)(UINTN)DataPtr, Buffer, DataSize);
  return EFI_SUCCESS;
}

/**
  This code gets the variable data offset related to variable header.

    @param[in] Variable             Pointer to the Variable Header.
    @param[in] AuthFormat           TRUE indicates authenticated variables are used.
                                    FALSE indicates authenticated variables are not used.

  @return Variable Data offset.

**/
UINTN
GetAccessVariableDataOffset (
  IN  ACCESS_VARIABLE_HEADER          *Variable,
  IN  BOOLEAN                         AuthFormat
  )
{
  UINTN  Value;

  //
  // Be careful about pad size for alignment
  //
  Value  = GetAccessVariableHeaderSize (AuthFormat);
  Value += NameSizeOfAccessVariable (Variable, AuthFormat);
  Value += GET_PAD_SIZE (NameSizeOfAccessVariable (Variable, AuthFormat));

  return Value;
}


/**
  Update the variable region with Variable information. If EFI_VARIABLE_AUTHENTICATED_WRITE_ACCESS is set,
  index of associated public key is needed.

  @param[in] VariableName         ame of variable.
  @param[in] VendorGuid           Guid of variable.
  @param AccessKey                Pointer to Access Key to use to edit/create variable
  @param[in] Data                 Variable data.
  @param[in] DataSize             Size of data. 0 means delete.
  @param[in] Attributes           Attributes of the variable.
  @param[in] KeyIndex             Index of associated public key.
  @param[in] MonotonicCount       Value of associated monotonic count.
  @param[in, out] CacheVariable   The variable information which is used to keep track of variable usage.
  @param[in] TimeStamp            Value of associated TimeStamp.

  @retval EFI_SUCCESS             The update operation is success.
  @retval EFI_OUT_OF_RESOURCES    Variable region is full, can not write other data into this region.

**/
EFI_STATUS
UpdateAccessVariable (
  IN      CHAR16                          *VariableName,
  IN      EFI_GUID                        *VendorGuid,
  IN      DEMO1_ACCESS_KEY                *AccessKey,
  IN      VOID                            *Data,
  IN      UINTN                           DataSize,
  IN      UINT32                          Attributes      OPTIONAL,
  IN      UINT32                          KeyIndex        OPTIONAL,
  IN      UINT64                          MonotonicCount  OPTIONAL,
  IN OUT  ACCESS_VARIABLE_POINTER_TRACK   *CacheVariable,
  IN      EFI_TIME                        *TimeStamp      OPTIONAL
  )
{
  EFI_STATUS                          Status;
  ACCESS_VARIABLE_HEADER              *NextVariable;
  UINTN                               ScratchSize;
  UINTN                               MaxDataSize;
  UINTN                               VarNameOffset;
  UINTN                               VarDataOffset;
  UINTN                               VarNameSize;
  UINTN                               VarSize;
  EFI_FIRMWARE_VOLUME_BLOCK_PROTOCOL  *Fvb;
  UINT8                               State;
  ACCESS_VARIABLE_POINTER_TRACK       *Variable;
  ACCESS_VARIABLE_POINTER_TRACK       NvVariable;
  VARIABLE_STORE_HEADER               *VariableStoreHeader;
  UINT8                               *BufferForMerge;
  UINTN                               MergedBufSize;
  BOOLEAN                             DataReady;
  UINTN                               DataOffset;
  AUTHENTICATED_VARIABLE_HEADER       *AuthVariable;
  BOOLEAN                             AuthFormat;

  if (AccessKey == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if ((AccessKey->access_key_store[1] & WRITE_ACCESS) != WRITE_ACCESS){
    return EFI_WRITE_PROTECTED;
  }

  AuthFormat = mineVariableModuleGlobal->VariableGlobal.AuthFormat;
  if ((CacheVariable->CurrPtr == NULL))
  {
    Variable = CacheVariable;
  }
  else
  {
    //
    // Update/Delete existing NV variable.
    // CacheVariable points to the variable in the memory copy of Flash area
    // Now let Variable points to the same variable in Flash area.
    //
    VariableStoreHeader = (VARIABLE_STORE_HEADER *)((UINTN)mineVariableModuleGlobal->VariableGlobal.NonVolatileVariableBase);
    Variable            = &NvVariable;
    Variable->StartPtr  = GetAccessStartPointer (VariableStoreHeader);
    Variable->EndPtr    = (ACCESS_VARIABLE_HEADER *)((UINTN)Variable->StartPtr + ((UINTN)CacheVariable->EndPtr - (UINTN)CacheVariable->StartPtr));

    Variable->CurrPtr = (ACCESS_VARIABLE_HEADER *)((UINTN)Variable->StartPtr + ((UINTN)CacheVariable->CurrPtr - (UINTN)CacheVariable->StartPtr));
    if (CacheVariable->InDeletedTransitionPtr != NULL) {
      Variable->InDeletedTransitionPtr = (ACCESS_VARIABLE_HEADER *)((UINTN)Variable->StartPtr + ((UINTN)CacheVariable->InDeletedTransitionPtr - (UINTN)CacheVariable->StartPtr));
    } else {
      Variable->InDeletedTransitionPtr = NULL;
    }

    Variable->Volatile = FALSE;
  }
  Fvb = mineVariableModuleGlobal->FvbInstance;

  //
  // Tricky part: Use scratch data area at the end of volatile variable store
  // as a temporary storage.
  //
  NextVariable = GetAccessEndPointer ((VARIABLE_STORE_HEADER *)((UINTN)mineVariableModuleGlobal->VariableGlobal.NonVolatileVariableBase));
  ScratchSize  = mineVariableModuleGlobal->ScratchBufferSize;
  SetMem (NextVariable, ScratchSize, 0xff);
  DataReady = FALSE;

  if (Variable->CurrPtr != NULL)
  {
    //
    // Update/Delete existing variable.
    //
    //
    // If the variable is marked valid, and the same data has been passed in,
    // then return to the caller immediately.
    //
    if ((DataSizeOfAccessVariable (CacheVariable->CurrPtr, AuthFormat) == DataSize) &&
        (CompareMem (Data, GetAccessVariableDataPtr (CacheVariable->CurrPtr, AuthFormat), DataSize) == 0) &&
        ((Attributes & EFI_VARIABLE_APPEND_WRITE) == 0) &&
        (TimeStamp == NULL))
    {
      //
      // Variable content unchanged and no need to update timestamp, just return.
      //
      UpdateAccessVariableInfo (VariableName, VendorGuid, Variable->Volatile, FALSE, TRUE, FALSE, FALSE, &gVarInfo);
      Status = EFI_SUCCESS;
      goto Done;
    }
    else if ((CacheVariable->CurrPtr->State == VAR_ADDED) ||
               (CacheVariable->CurrPtr->State == (VAR_ADDED & VAR_IN_DELETED_TRANSITION)))
    {
      //
      // EFI_VARIABLE_APPEND_WRITE attribute only effects for existing variable.
      //
      if ((Attributes & EFI_VARIABLE_APPEND_WRITE) != 0) {
        //
        // NOTE: From 0 to DataOffset of NextVariable is reserved for Variable Header and Name.
        // From DataOffset of NextVariable is to save the existing variable data.
        //
        DataOffset     = GetAccessVariableDataOffset (CacheVariable->CurrPtr, AuthFormat);
        BufferForMerge = (UINT8 *)((UINTN)NextVariable + DataOffset);
        CopyMem (
          BufferForMerge,
          (UINT8 *)((UINTN)CacheVariable->CurrPtr + DataOffset),
          DataSizeOfAccessVariable (CacheVariable->CurrPtr, AuthFormat)
          );

        //
        // Set Max Auth/Non-Volatile/Volatile Variable Data Size as default MaxDataSize.
        //
        if ((Attributes & VARIABLE_ATTRIBUTE_AT_AW) != 0) {
          MaxDataSize = mineVariableModuleGlobal->MaxAuthVariableSize - DataOffset;
        } else if ((Attributes & EFI_VARIABLE_NON_VOLATILE) != 0) {
          MaxDataSize = mineVariableModuleGlobal->MaxVariableSize - DataOffset;
        } else {
          MaxDataSize = mineVariableModuleGlobal->MaxVolatileVariableSize - DataOffset;
        }

        //
        // Append the new data to the end of existing data.
        // Max Harware error record variable data size is different from common/auth variable.
        //
        if ((Attributes & EFI_VARIABLE_HARDWARE_ERROR_RECORD) == EFI_VARIABLE_HARDWARE_ERROR_RECORD) {
          MaxDataSize = 4 - DataOffset;
        }

        if (DataSizeOfAccessVariable (CacheVariable->CurrPtr, AuthFormat) + DataSize > MaxDataSize) {
          //
          // Existing data size + new data size exceed maximum variable size limitation.
          //
          Status = EFI_INVALID_PARAMETER;
          goto Done;
        }
        CopyMem ((UINT8 *)((UINTN)BufferForMerge + DataSizeOfAccessVariable (CacheVariable->CurrPtr, AuthFormat)), Data, DataSize);
        MergedBufSize = DataSizeOfAccessVariable (CacheVariable->CurrPtr, AuthFormat) + DataSize;

        //
        // BufferForMerge(from DataOffset of NextVariable) has included the merged existing and new data.
        //
        Data      = BufferForMerge;
        DataSize  = MergedBufSize;
        DataReady = TRUE;
      }

      //
      // Mark the old variable as in delete transition.
      //
      State  = CacheVariable->CurrPtr->State;
      State &= VAR_IN_DELETED_TRANSITION;

      Status = UpdateAccessVariableStore (&mineVariableModuleGlobal->VariableGlobal, Variable->Volatile, FALSE, Fvb, (UINTN)&Variable->CurrPtr->State, sizeof (UINT8), &State );
      if (EFI_ERROR (Status)) {
        goto Done;
      }

      if (!Variable->Volatile) {
        CacheVariable->CurrPtr->State = State;
      }
    }
  }
  else
  {
    //
    // Not found existing variable. Create a new variable.
    //
    if ((DataSize == 0) && ((Attributes & EFI_VARIABLE_APPEND_WRITE) != 0)) {
      Status = EFI_SUCCESS;
      goto Done;
    }
    //
    // Make sure we are trying to create a new variable.
    // Setting a data variable with zero DataSize or no access attributes means to delete it.
    //
    if ((DataSize == 0) || ((Attributes & (EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_BOOTSERVICE_ACCESS)) == 0)) {
      Status = EFI_NOT_FOUND;
      goto Done;
    }
    CopyMem(&(NextVariable->accesskeylist), AccessKey, KEYSIZE); // Store the access key with the new variable
  }
  //
  // Function part - create a new variable and copy the data.
  // Both update a variable and create a variable will come here.
  //
  NextVariable->StartId = VARIABLE_DATA;
  NextVariable->Reserved = 0;

  if (mineVariableModuleGlobal->VariableGlobal.AuthFormat)
  {
    AuthVariable                 = (AUTHENTICATED_VARIABLE_HEADER *)NextVariable;
    AuthVariable->PubKeyIndex    = KeyIndex;
    AuthVariable->MonotonicCount = MonotonicCount;
    ZeroMem (&AuthVariable->TimeStamp, sizeof (EFI_TIME));

    if (((Attributes & EFI_VARIABLE_TIME_BASED_AUTHENTICATED_WRITE_ACCESS) != 0) &&
        (TimeStamp != NULL))
    {
      if ((Attributes & EFI_VARIABLE_APPEND_WRITE) == 0) {
        CopyMem (&AuthVariable->TimeStamp, TimeStamp, sizeof (EFI_TIME));
      } else {
        //
        // In the case when the EFI_VARIABLE_APPEND_WRITE attribute is set, only
        // when the new TimeStamp value is later than the current timestamp associated
        // with the variable, we need associate the new timestamp with the updated value.
        //
        if (Variable->CurrPtr != NULL) {
          if (AccessVariableCompareTimeStampInternal (&(((AUTHENTICATED_VARIABLE_HEADER *)CacheVariable->CurrPtr)->TimeStamp), TimeStamp)) {
            CopyMem (&AuthVariable->TimeStamp, TimeStamp, sizeof (EFI_TIME));
          } else {
            CopyMem (&AuthVariable->TimeStamp, &(((AUTHENTICATED_VARIABLE_HEADER *)CacheVariable->CurrPtr)->TimeStamp), sizeof (EFI_TIME));
          }
        }
      }
    }
  }

  //
  // The EFI_VARIABLE_APPEND_WRITE attribute will never be set in the returned
  // Attributes bitmask parameter of a GetVariable() call.
  //
  NextVariable->Attributes = Attributes & (~EFI_VARIABLE_APPEND_WRITE);

  VarNameOffset = GetAccessVariableHeaderSize (AuthFormat);
  VarNameSize   = StrSize (VariableName);
  CopyMem ((UINT8 *)((UINTN)NextVariable + VarNameOffset), VariableName, VarNameSize );
  VarDataOffset = VarNameOffset + VarNameSize + GET_PAD_SIZE (VarNameSize);

  //
  // If DataReady is TRUE, it means the variable data has been saved into
  // NextVariable during EFI_VARIABLE_APPEND_WRITE operation preparation.
  //
  if (!DataReady)
  {
    CopyMem ((UINT8 *)((UINTN)NextVariable + VarDataOffset), Data, DataSize);
  }

  CopyMem (GetAccessVendorGuidPtr (NextVariable, AuthFormat), VendorGuid, sizeof (EFI_GUID));
  //
  // There will be pad bytes after Data, the NextVariable->NameSize and
  // NextVariable->DataSize should not include pad size so that variable
  // service can get actual size in GetVariable.
  //

  SetNameSizeOfAccessVariable (NextVariable, VarNameSize, AuthFormat);
  SetDataSizeOfAccessVariable (NextVariable, DataSize, AuthFormat);

  //
  // The actual size of the variable that stores in storage should
  // include pad size.
  //
  VarSize = VarDataOffset + DataSize + GET_PAD_SIZE (DataSize);
  if ((Attributes & EFI_VARIABLE_NON_VOLATILE) != 0)
  {
    //
    // Emulated non-volatile variable mode.
    //
    NextVariable->State = VAR_ADDED;
    Status              = UpdateAccessVariableStore (
                            &mineVariableModuleGlobal->VariableGlobal,
                            FALSE,
                            TRUE,
                            Fvb,
                            mineVariableModuleGlobal->NonVolatileLastVariableOffset,
                            (UINT32)VarSize,
                            (UINT8 *)NextVariable
                            );
    if (EFI_ERROR (Status)) {
      goto Done;
    }

    mineVariableModuleGlobal->NonVolatileLastVariableOffset += HEADER_ALIGN (VarSize);

    if ((Attributes & EFI_VARIABLE_HARDWARE_ERROR_RECORD) != 0) {
      mineVariableModuleGlobal->HwErrVariableTotalSize += HEADER_ALIGN (VarSize);
    } else {
      mineVariableModuleGlobal->CommonVariableTotalSize += HEADER_ALIGN (VarSize);
    }
  }
  else
  {
    return EFI_INVALID_PARAMETER;
  }

Done:
  return Status;
}

/**
  Find the variable in the specified variable store.

  @param[in]       VariableName   Name of the variable to be found
  @param[in]       VendorGuid     Vendor GUID to be found.
  @param[in]       IgnoreRtCheck  Ignore EFI_VARIABLE_RUNTIME_ACCESS attribute
                                  check at runtime when searching variable.
  @param[in, out]  PtrTrack       Variable Track Pointer structure that contains Variable Information.
  @param[in]       AuthFormat     TRUE indicates authenticated variables are used.
                                  FALSE indicates authenticated variables are not used.

  @retval                         EFI_SUCCESS   Variable found successfully
  @retval                         EFI_NOT_FOUND Variable not found

**/
EFI_STATUS
FindAccessVariableEx (
  IN     CHAR16                           *VariableName,
  IN     EFI_GUID                         *VendorGuid,
  IN     BOOLEAN                          IgnoreRtCheck,
  IN OUT ACCESS_VARIABLE_POINTER_TRACK    *PtrTrack,
  IN     BOOLEAN                          AuthFormat
  )
{
  ACCESS_VARIABLE_HEADER  *InDeletedVariable;
  VOID             *Point;

  PtrTrack->InDeletedTransitionPtr = NULL;

  //
  // Find the variable by walk through HOB, volatile and non-volatile variable store.
  //
  InDeletedVariable = NULL;

  for ( PtrTrack->CurrPtr = PtrTrack->StartPtr
        ; IsValidAccessVariableHeader (PtrTrack->CurrPtr, PtrTrack->EndPtr)
        ; PtrTrack->CurrPtr = GetNextAccessVariablePtr (PtrTrack->CurrPtr, AuthFormat)
        )
  {
    if ((PtrTrack->CurrPtr->State == VAR_ADDED) ||
        (PtrTrack->CurrPtr->State == (VAR_IN_DELETED_TRANSITION & VAR_ADDED))
        )
    {
      if (IgnoreRtCheck || !AtRuntime () || ((PtrTrack->CurrPtr->Attributes & EFI_VARIABLE_RUNTIME_ACCESS) != 0)) {
        if (VariableName[0] == 0) {
          if (PtrTrack->CurrPtr->State == (VAR_IN_DELETED_TRANSITION & VAR_ADDED)) {
            InDeletedVariable = PtrTrack->CurrPtr;
          } else {
            PtrTrack->InDeletedTransitionPtr = InDeletedVariable;
            return EFI_SUCCESS;
          }
        } else {
          if (CompareGuid (VendorGuid, GetAccessVendorGuidPtr (PtrTrack->CurrPtr, AuthFormat))) {
            Point = (VOID *)GetAccessVariableNamePtr (PtrTrack->CurrPtr, AuthFormat);

            ASSERT (NameSizeOfAccessVariable (PtrTrack->CurrPtr, AuthFormat) != 0);
            if (CompareMem (VariableName, Point, NameSizeOfAccessVariable (PtrTrack->CurrPtr, AuthFormat)) == 0) {
              if (PtrTrack->CurrPtr->State == (VAR_IN_DELETED_TRANSITION & VAR_ADDED)) {
                InDeletedVariable = PtrTrack->CurrPtr;
              } else {
                PtrTrack->InDeletedTransitionPtr = InDeletedVariable;
                return EFI_SUCCESS;
              }
            }
          }
        }
      }
    }
  }

  PtrTrack->CurrPtr = InDeletedVariable;
  return (PtrTrack->CurrPtr  == NULL) ? EFI_NOT_FOUND : EFI_SUCCESS;
}

/**
  Finds variable in storage blocks of volatile and non-volatile storage areas.

  This code finds variable in storage blocks of volatile and non-volatile storage areas.
  If VariableName is an empty string, then we just return the first
  qualified variable without comparing VariableName and VendorGuid.
  If IgnoreRtCheck is TRUE, then we ignore the EFI_VARIABLE_RUNTIME_ACCESS attribute check
  at runtime when searching existing variable, only VariableName and VendorGuid are compared.
  Otherwise, variables without EFI_VARIABLE_RUNTIME_ACCESS are not visible at runtime.

  @param[in]   VariableName       Name of the variable to be found.
  @param[in]   VendorGuid         Vendor GUID to be found.
  @param[out]  PtrTrack           ACCESS_VARIABLE_POINTER_TRACK structure for output,
                                  including the range searched and the target position.
  @param[in]   Global             Pointer to VARIABLE_GLOBAL structure, including
                                  base of volatile variable storage area, base of
                                  NV variable storage area, and a lock.
  @param[in]   IgnoreRtCheck      Ignore EFI_VARIABLE_RUNTIME_ACCESS attribute
                                  check at runtime when searching variable.

  @retval                         EFI_INVALID_PARAMETER  If VariableName is not an empty string, while
                                                         VendorGuid is NULL.
  @retval                         EFI_SUCCESS            Variable successfully found.
  @retval                         EFI_NOT_FOUND          Variable not found

**/
EFI_STATUS
FindAccessVariable (
  IN  CHAR16                          *VariableName,
  IN  EFI_GUID                        *VendorGuid,
  OUT ACCESS_VARIABLE_POINTER_TRACK   *PtrTrack,
  IN  VARIABLE_GLOBAL                 *Global,
  IN  BOOLEAN                         IgnoreRtCheck
  )
{
  EFI_STATUS             Status;

  if ((VariableName[0] != 0) && (VendorGuid == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  PtrTrack->StartPtr = GetAccessStartPointer (mineNvVariableCache);
  PtrTrack->EndPtr   = GetAccessEndPointer (mineNvVariableCache);
  PtrTrack->Volatile = FALSE;

  Status =  FindAccessVariableEx (VariableName, VendorGuid, IgnoreRtCheck, PtrTrack, mineVariableModuleGlobal->VariableGlobal.AuthFormat);
  if (!EFI_ERROR (Status)) {
    return Status;
  }
  return EFI_NOT_FOUND;
}

/**
  This code sets variable in storage blocks (Volatile or Non-Volatile).

  Caution: This function may receive untrusted input.
  This function may be invoked in SMM mode, and datasize and data are external input.
  This function will do basic validation, before parse the data.
  This function will parse the authentication carefully to avoid security issues, like
  buffer overflow, integer overflow.
  This function will check attribute carefully to avoid authentication bypass.

  @param VariableName             Name of Variable to be found.
  @param VendorGuid               Variable vendor GUID.
  @param Attributes               Attribute value of the variable found
  @param AccessKey                Pointer to Access Key to use to edit/create variable
  @param DataSize                 Size of Data found. If size is less than the
                                  data, this value contains the required size.
  @param Data                     Data pointer.

  @return                         EFI_INVALID_PARAMETER   Invalid parameter.
  @return                         EFI_SUCCESS             Set successfully.
  @return                         EFI_OUT_OF_RESOURCES    Resource not enough to set variable.
  @return                         EFI_NOT_FOUND           Not found.
  @return                         EFI_WRITE_PROTECTED     Variable is read-only.

**/
EFI_STATUS
EFIAPI
mineVariableServiceSetVariable (
  IN CHAR16                           *VariableName,
  IN EFI_GUID                         *VendorGuid,
  IN UINT32                           Attributes,
  IN DEMO1_ACCESS_KEY                 *AccessKey,
  IN UINTN                            DataSize,
  IN VOID                             *Data
  )
{
  ACCESS_VARIABLE_POINTER_TRACK   Variable;
  EFI_STATUS                      Status;
  ACCESS_VARIABLE_HEADER          *NextVariable;
  EFI_PHYSICAL_ADDRESS            Point;
  UINTN                           PayloadSize;
  BOOLEAN                         AuthFormat;
  BOOLEAN                         ValidKey;

  AuthFormat = mineVariableModuleGlobal->VariableGlobal.AuthFormat;

  // Check input parameters.
  if ((VariableName == NULL) || (VariableName[0] == 0) || (VendorGuid == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  if ((DataSize != 0) && (Data == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  PayloadSize = DataSize;
  if ((UINTN)(~0) - PayloadSize < StrSize (VariableName)) {
    // Prevent whole variable size overflow
    return EFI_INVALID_PARAMETER;
  }

  if (StrSize (VariableName) + PayloadSize > mineVariableModuleGlobal->MaxVariableSize - GetAccessVariableHeaderSize (AuthFormat)) {
    DEBUG ((DEBUG_ERROR, "%a: Failed to set variable '%s' with Guid %g\n", __FUNCTION__, VariableName, VendorGuid));
    DEBUG ((DEBUG_ERROR,"NameSize(0x%x) + PayloadSize(0x%x) > " "MaxVariableSize(0x%x) - HeaderSize(0x%x)\n", StrSize (VariableName), PayloadSize, GetAccessVariableHeaderSize (AuthFormat)));
    return EFI_INVALID_PARAMETER;
  }

  //
  // Parse non-volatile variable data and get last variable offset.
  //
  Point = mineVariableModuleGlobal->VariableGlobal.NonVolatileVariableBase;
  NextVariable = GetAccessStartPointer ((VARIABLE_STORE_HEADER *)(UINTN)Point);
  while (IsValidAccessVariableHeader (NextVariable, GetAccessEndPointer ((VARIABLE_STORE_HEADER *)(UINTN)Point))) {
    NextVariable = GetNextAccessVariablePtr (NextVariable, AuthFormat);
  }
  mineVariableModuleGlobal->NonVolatileLastVariableOffset = (UINTN)NextVariable - (UINTN)Point;

  if (AccessKeyProtocol->Demo1ValidateAccessKey(AccessKeyProtocol, NULL, AccessKey, TRUE, &ValidKey) != EFI_SUCCESS){
    return EFI_INVALID_PARAMETER;
  }
  if (!ValidKey){
    return EFI_INVALID_PARAMETER;
  }

  FindAccessVariable (VariableName, VendorGuid, &Variable, &mineVariableModuleGlobal->VariableGlobal, TRUE);
  Status = UpdateAccessVariable (VariableName, VendorGuid, AccessKey, Data, DataSize, Attributes, 0, 0, &Variable, NULL);
  return Status;
}


/**
  This code finds variable in storage blocks (Volatile or Non-Volatile).
  Caution: This function may receive untrusted input.
  This function may be invoked in SMM mode, and datasize is external input.
  This function will do basic validation, before parse the data.
  @param VariableName             Name of Variable to be found.
  @param VendorGuid               Variable vendor GUID.
  @param Attributes               Attribute value of the variable found..
  @param AccessKey                Pointer to Access Key to use to edit/create variable
  @param DataSize                 Size of Data found. If size is less than the
                                  data, this value contains the required size.
  @param Data                     The buffer to return the contents of the variable. May be NULL
                                  with a zero DataSize in order to determine the size buffer needed.
  @return                         EFI_INVALID_PARAMETER  Invalid parameter.
  @return                         EFI_SUCCESS            Find the specified variable.
  @return                         EFI_NOT_FOUND          Not found.
  @return                         EFI_BUFFER_TO_SMALL    DataSize is too small for the result.

**/
EFI_STATUS
EFIAPI
mineVariableServiceGetVariable (
  IN      CHAR16                      *VariableName,
  IN      EFI_GUID                    *VendorGuid,
  OUT     UINT32                      *Attributes OPTIONAL,
  IN      DEMO1_ACCESS_KEY            *AccessKey,
  IN OUT  UINTN                       *DataSize,
  OUT     VOID                        *Data OPTIONAL
  )
{
  EFI_STATUS                      Status;
  ACCESS_VARIABLE_POINTER_TRACK   Variable;
  UINTN                           VarDataSize;
  BOOLEAN                         ValidKey;

  if ((VariableName == NULL) || (VendorGuid == NULL) || (DataSize == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  if (VariableName[0] == 0) {
    return EFI_NOT_FOUND;
  }

  if (AccessKeyProtocol->Demo1ValidateAccessKey(AccessKeyProtocol, NULL, AccessKey, FALSE, &ValidKey) != EFI_SUCCESS){
    return EFI_INVALID_PARAMETER;
  }
  if (!ValidKey){
    return EFI_INVALID_PARAMETER;
  }

  Status = FindAccessVariable (VariableName, VendorGuid, &Variable, &mineVariableModuleGlobal->VariableGlobal, FALSE);
  if ((Variable.CurrPtr == NULL) || EFI_ERROR (Status)) {
    goto Done;
  }

  //
  // Get data size
  //
  VarDataSize = DataSizeOfAccessVariable (Variable.CurrPtr, mineVariableModuleGlobal->VariableGlobal.AuthFormat);
  ASSERT (VarDataSize != 0);

  if (*DataSize >= VarDataSize) {
    if (Data == NULL) {
      Status = EFI_INVALID_PARAMETER;
      goto Done;
    }

    CopyMem (Data, GetAccessVariableDataPtr (Variable.CurrPtr, mineVariableModuleGlobal->VariableGlobal.AuthFormat), VarDataSize);

    *DataSize = VarDataSize;
    UpdateAccessVariableInfo (VariableName, VendorGuid, Variable.Volatile, TRUE, FALSE, FALSE, FALSE, &gVarInfo);

    Status = EFI_SUCCESS;
    goto Done;
  } else {
    *DataSize = VarDataSize;
    Status    = EFI_BUFFER_TOO_SMALL;
    goto Done;
  }

Done:
  if ((Status == EFI_SUCCESS) || (Status == EFI_BUFFER_TOO_SMALL)) {
    if ((Attributes != NULL) && (Variable.CurrPtr != NULL)) {
      *Attributes = Variable.CurrPtr->Attributes;
    }
  }

  return Status;
}

/**
  Main entry for Demo1 Variable driver.

  @param ImageHandle              Image handle this driver.
  @param SystemTable              Pointer to SystemTable.

  @retval                         EFI_SUCCESS           Completed successfully.
  @retval                         EFI_OUT_OF_RESOURCES  Unable to allocate a block of memory for variable storage
  @retval                         Status                Returns from other functions may fail, causing this function to fail
**/
EFI_STATUS
EFIAPI
Demo1VariableInit (
  IN EFI_HANDLE                       ImageHandle,
  IN EFI_SYSTEM_TABLE                 *SystemTable
  )
{
  EFI_STATUS Status;

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
  // Allocate Variable Storage
  //
  mineVariableModuleGlobal = AllocateRuntimeZeroPool (sizeof (VARIABLE_MODULE_GLOBAL));
  if (mineVariableModuleGlobal == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // Initialize Non-volatile Variabl Store
  //
  Status = mineInitNonVolatileVariableStore ();
  if (EFI_ERROR (Status)) {
    FreePool (mineVariableModuleGlobal);
    return Status;
  } 

  SystemTable->RuntimeServices->GetAccessVariable = mineVariableServiceGetVariable;
  SystemTable->RuntimeServices->SetAccessVariable = mineVariableServiceSetVariable;
  return EFI_SUCCESS;
}

/**
  Unloads the application and its installed protocol.

  @param[in]  ImageHandle         Handle that identifies the image to be unloaded.

  @retval EFI_SUCCESS             The image has been unloaded.
**/
EFI_STATUS
EFIAPI
Demo1_Variable_Unload (
  IN EFI_HANDLE                       ImageHandle
  )
{
  gST->RuntimeServices->GetAccessVariable = NULL;
  gST->RuntimeServices->SetAccessVariable = NULL;
  FreePool(mineVariableModuleGlobal);
  return EFI_SUCCESS;
}
