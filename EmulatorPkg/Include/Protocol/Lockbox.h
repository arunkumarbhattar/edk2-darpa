/**
 * Header for the lockbox protocol information

**/
#ifndef __EMU_LOCKBOX_H__
#define __EMU_LOCKBOX_H__

#define EFI_LOCKBOX_PROTOCOL_GUID \
    { 0xABCDEF13, 0x8C35, 0xB345, { 0x8A, 0x0F, 0x7A, 0xC8, 0xA5, 0xFD, 0x05, 0x21 } };

typedef struct _EFI_LOCKBOX_PROTOCOL EFI_LOCKBOX_PROTOCOL;

typedef   
 EFI_STATUS
 (EFIAPI *EFI_LOCKBOX_SET_BIT)(                                                                                                     
    IN EFI_LOCKBOX_PROTOCOL    *This
    );

typedef   
 EFI_STATUS
 (EFIAPI *EFI_LOCKBOX_UNSET_BIT)(                                                                                                     
    IN EFI_LOCKBOX_PROTOCOL    *This
    );

typedef   
 EFI_STATUS
 (EFIAPI *EFI_LOCKBOX_WRITE)(                                                                                                     
    IN EFI_LOCKBOX_PROTOCOL    *This,
    IN VOID *Data,
    IN INTN len
    );

struct _EFI_LOCKBOX_PROTOCOL {
    EFI_LOCKBOX_SET_BIT     LockboxSetBit;
    EFI_LOCKBOX_WRITE       LockboxWrite;
    EFI_LOCKBOX_UNSET_BIT   LockboxUnsetBit;
};


#endif
