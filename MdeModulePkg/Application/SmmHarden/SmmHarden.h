EFI_STATUS
EFIAPI
SmmHardenVariableManager (
  IN EFI_HANDLE  DispatchHandle,
  IN CONST _Ptr<VOID>  Context         OPTIONAL,
  IN OUT _Array_ptr<VOID>    CommBuffer      OPTIONAL,
  IN OUT _Ptr<UINTN>   CommBufferSize  OPTIONAL
);

EFI_STATUS
EFIAPI
SmmHardenBootService (
  IN EFI_HANDLE  DispatchHandle,
  IN CONST _Ptr<VOID>  Context         OPTIONAL,
  IN OUT _Array_ptr<VOID>    CommBuffer      OPTIONAL,
  IN OUT _Ptr<UINTN>   CommBufferSize OPTIONAL
);

EFI_STATUS _Checked
        SmmHardenGetVariable(
        IN _Nt_array_ptr<CHAR16>   VariableName,
        OUT _Array_ptr<UINT32>    VariableValue
);

EFI_STATUS _Checked
        SmmHardenSetVariable(
        IN _Nt_array_ptr<CHAR16> VariableName,
        IN UINT32     VariableValue
);

VOID
    SmmHardenCommunicateSMM(
    IN EFI_GUID  Guid,
    IN _Array_ptr<VOID>  Data : byte_count(DataSize),
    IN UINTN    DataSize
);
