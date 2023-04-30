/** @file
  Basic header file for the heap manipulation protocol information

**/
#ifndef __EMU_HEAP_MANIP_H__
#define __EMU_HEAP_MANIP_H__

#define EFI_HEAP_MANIP_PROTOCOL_GUID \
    { 0xAABBCCDD, 0x1234, 0x3458, { 0x8A, 0x0F, 0x7A, 0xC8, 0xA5, 0xFD, 0x05, 0x21 } };

typedef struct _EFI_HEAP_MANIP_PROTOCOL EFI_HEAP_MANIP_PROTOCOL;

typedef   
 EFI_STATUS
 (EFIAPI *EFI_HEAP_MANIP_ALLOC)(                                                                                                     
    IN EFI_HEAP_MANIP_PROTOCOL    *This,
    IN unsigned int length
    );

typedef   
 EFI_STATUS
 (EFIAPI *EFI_HEAP_MANIP_FREE)(                                                                                                     
    IN EFI_HEAP_MANIP_PROTOCOL    *This,
    IN unsigned int index
    );

typedef   
 EFI_STATUS
 (EFIAPI *EFI_HEAP_MANIP_WRITE)(                                                                                                     
    IN EFI_HEAP_MANIP_PROTOCOL    *This,
    IN unsigned int index,
    IN char *buffer,
    IN unsigned int length
    );

typedef   
 EFI_STATUS
 (EFIAPI *EFI_HEAP_MANIP_READ)(                                                                                                     
    IN EFI_HEAP_MANIP_PROTOCOL    *This,
    IN unsigned int index,
    OUT void **buffAddr
    );

struct _EFI_HEAP_MANIP_PROTOCOL {
    EFI_HEAP_MANIP_ALLOC     alloc;
    EFI_HEAP_MANIP_FREE      free;
    EFI_HEAP_MANIP_WRITE     write;
    EFI_HEAP_MANIP_READ      read;
};


#endif // __EMU_HEAP_MANIP_H__