/** @file
  Basic header file for the hello world protocol information

**/
#ifndef __EMU_HELLO_WORLD_H__
#define __EMU_HELLO_WORLD_H__

#define EFI_HELLO_WORLD_PROTOCOL_GUID \
    { 0xFFFFBE54, 0x8C35, 0xB345, { 0x8A, 0x0F, 0x7A, 0xC8, 0xA5, 0xFD, 0x05, 0x21 } };

typedef struct _EFI_HELLO_WORLD_PROTOCOL EFI_HELLO_WORLD_PROTOCOL;

typedef   
 EFI_STATUS
 (EFIAPI *EFI_HELLO_WORLD_A)(                                                                                                     
    IN EFI_HELLO_WORLD_PROTOCOL    *This
    );

typedef   
 EFI_STATUS
 (EFIAPI *EFI_HELLO_WORLD_B)(                                                                                                     
    IN EFI_HELLO_WORLD_PROTOCOL    *This
    );

typedef   
 EFI_STATUS
 (EFIAPI *EFI_HELLO_WORLD_C)(                                                                                                     
    IN EFI_HELLO_WORLD_PROTOCOL    *This
    );

typedef   
 EFI_STATUS
 (EFIAPI *EFI_HELLO_WORLD_D)(                                                                                                     
    IN EFI_HELLO_WORLD_PROTOCOL    *This
    );

struct _EFI_HELLO_WORLD_PROTOCOL {
    EFI_HELLO_WORLD_A     HWA;
    EFI_HELLO_WORLD_B     HWB;
    EFI_HELLO_WORLD_C     HWC;
    EFI_HELLO_WORLD_D     HWD;
};


#endif