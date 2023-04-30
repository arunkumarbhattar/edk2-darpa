#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/LoadedImage.h>
#include <Protocol/ShellParameters.h>

#include <Protocol/Shell.h>
#include <Library/ShellLib.h>

#include "../Demo1_Access_Key/Demo1_Access_Key.h"
Demo1_Access_Key_PROTOCOL *AccessKeyProtocol;

#define EXAMPLEAPP_VARNAME   L"ExampleVar"

EFI_STATUS
EFIAPI
Demo1_Example_App_Entry (
    IN EFI_HANDLE imgHandle,
    IN EFI_SYSTEM_TABLE* sysTable
)
{
    EFI_STATUS retval;
    BOOLEAN retbool;

    gBS = sysTable->BootServices;
    gBS->SetWatchdogTimer(0, 0, 0, NULL); // UEFI apps automatically exit after 5 minutes. Stop that here.

    Print(L"Example App Started\r\n");

    /* Create Access Key Storage */
    DEMO1_ACCESS_KEY *my_access_key=AllocatePool(sizeof(DEMO1_ACCESS_KEY));

    /* Locate Access Key Protocol */
    gBS->LocateProtocol(&gDemo1AccessKeyProtocolGuid, NULL, (VOID *)&AccessKeyProtocol);

    /* Generate Access Key */
    Print(L"Call Generate Access Key\r\n");
    retval = AccessKeyProtocol->Demo1GenerateAccessKey(AccessKeyProtocol, NULL, TRUE, my_access_key);
    if (retval != 0) {
        Print(L"Failed to generate access key\r\n");
        return EFI_ABORTED;
    }
    Print(L"I have an access key: (0x%016llx..%016llx) \r\n",
        my_access_key->access_key_store[0], my_access_key->access_key_store[1]);
    
    /* Validate Access Key */
    Print(L"Call Validate Access Key \r\n");
    AccessKeyProtocol->Demo1ValidateAccessKey(AccessKeyProtocol, NULL, my_access_key, TRUE, &retbool);
    if (retbool == FALSE) {
        Print(L"Could not validate key\r\n");
        return EFI_ABORTED;
    }
    Print(L"I have a valid key\r\n");

    /* Set Variable with Access Key */
    Print(L"Call Set Access Variable \r\n");
    UINTN ExampleVar_Value = 0xdeadbeef;
    EFI_STATUS Status  = sysTable->RuntimeServices->SetAccessVariable (
        EXAMPLEAPP_VARNAME,
        &gExampleVariableGuid,
        EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE,
        my_access_key,
        sizeof(UINTN),
        &ExampleVar_Value
        );
    if (EFI_ERROR (Status)) {
        DEBUG ((DEBUG_ERROR, "%a: variable '%s' could not be written - bailing!\n", __FUNCTION__, EXAMPLEAPP_VARNAME));
        return Status;
    }
    Print(L"Set Access Variable Success\r\n");

    /* Get Variable with Access Key */
     Print(L"Call Get Access Variable \r\n");
    UINTN   getExampleVar_Value = 0;
    UINTN   BufferSize = sizeof(getExampleVar_Value);
    Status = gST->RuntimeServices->GetAccessVariable (
        EXAMPLEAPP_VARNAME,
        &gExampleVariableGuid,
        NULL,
        my_access_key,
        &BufferSize,
        &getExampleVar_Value
        );
    if (EFI_ERROR (Status)) {
        DEBUG ((DEBUG_ERROR, "%a: variable '%s' could not be read - bailing!\n", __FUNCTION__, EXAMPLEAPP_VARNAME));
        return Status;
    }
    Print(L"Get Access Variable Success - 0x%08llx\r\n", getExampleVar_Value);

    Print(L"Example Complete\r\n");
    return EFI_SUCCESS;
}
