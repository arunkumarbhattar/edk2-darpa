/** 

**/
#ifndef __EMU_NOTIFY_TEST_H__
#define __EMU_NOTIFY_TEST_H__

#define EFI_NOTIFY_TEST_PROTOCOL_GUID \
    { 0xABCDEF12, 0x8C35, 0xB345, { 0x8A, 0x0F, 0x7A, 0xC8, 0xA5, 0xFD, 0x05, 0x21 } };

typedef struct _EFI_NOTIFY_TEST_PROTOCOL EFI_NOTIFY_TEST_PROTOCOL;

typedef   
 EFI_STATUS
 (EFIAPI *EFI_NOTIFY_TEST_SETUP)(                                                                                                     
    IN EFI_NOTIFY_TEST_PROTOCOL    *This
    );

struct _EFI_NOTIFY_TEST_PROTOCOL {
    EFI_NOTIFY_TEST_SETUP     HWA;
};


#endif
