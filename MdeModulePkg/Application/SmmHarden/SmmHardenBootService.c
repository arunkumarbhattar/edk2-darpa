/*
This vulnerability is inspired by:
   https://github.com/binarly-io/Vulnerability-REsearch/blob/main/Insyde/BRLY-2022-022.md
*/

#include <Uefi/UefiBaseType.h>
#include "Library/VarCheckLib.h"
#include <Library/DebugLib.h>
#include <Library/BaseLib.h>
#include <Guid/VariableFormat.h>
#include <Protocol/SmmVariable.h>
#include "SmmHarden.h"
#include <Library/UefiBootServicesTableLib.h>

#define VARNAME L"SYS-FirstTime"


EFI_STATUS
EFIAPI
SmmHardenBootService (
        IN EFI_HANDLE  DispatchHandle,
        IN CONST _Ptr<VOID>  Context         OPTIONAL,
        IN OUT _Array_ptr<VOID>    CommBuffer      OPTIONAL,
        IN OUT _Ptr<UINTN>   CommBufferSize OPTIONAL
)
{
  EFI_STATUS Status;
  UINT32 SysFirstTime = 0;

  Status = SmmHardenGetVariable(VARNAME, &SysFirstTime);

  if(Status == EFI_NOT_FOUND || SysFirstTime == 0){
      DEBUG ((DEBUG_INFO, "[SmmHardenBootService] This is the first time we execute.\n"));
      SmmHardenSetVariable(VARNAME, 1);
      /* ROP-able vulnerability here */
  }
  else{
      DEBUG ((DEBUG_INFO, "[SmmHardenBootService] We already executed, aborting.\n"));
  }

  return EFI_SUCCESS;
}
