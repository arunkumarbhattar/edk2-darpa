# Demo Access Key Driver
## Driver Specifications
1. The driver MUST have access to the Random Number Generator Protocol.
2. The driver MUST create a keychain for the internal storage of keys.
3. The driver MUST create an event to indicate that the keychain is locked.
4. The generation of new keys MUST be halted, when the keychain lock event is triggered.
5. Once the keychain is locked it MUST remain locked for the remainder of system uptime.
6. Signaling keychain lock event is considered driver dependent implementation. This is up to the drivers utilizing the Access Key Protocol.
7. The Access Key Protocol MUST be loaded during the startup of the UEFI phase.
8. The driver MUST provide a protocol for Generation and Validation of keys.

## Generate Access Keys
### General
1. The driver MUST provide a function for the generation of new keys to other drivers.
2. If the keychain lock event has occurred then new keys MUST NOT be generated and EFI_WRITE_PROTECTED error MUST be returned.
3. Keys with WRITE access MUST have the upper 64 bits set to KEY MAGIC and WRITE MAGIC.
4. Keys with READ-ONLY access MUST have the upper 64 bits set to KEY MAGIC and READ MAGIC.
5. The result of random number generation MUST be returned when a key is produced.
6. Upon successful key creation, the key MUST be stored in an internal list for validation.
### Validation
1. The MUST provide a function for the validation of existing keys to other drivers.
2. The result of the function MUST be in the form of a boolean value declaring if the key exists in the internal keychain.
### Types of keys - Write and Read Access
1. If a key has READ MAGIC then it MUST NOT allow write access - only read access.
2. If a key has WRITE MAGIC then it MUST allow read and write access.

## Prose Overview
The Access Key protocol provides a basic method for authentication when reading and writing to NVRAM variables. It allows for drivers to generate keys with read only or read/write access to NVRAM variables.

Keys are only able to be generated prior to a ready-to-lock event. The Access Key protocol creates this event - when triggered indicates that the keychain is locked. Any further attempt to generate keys will result in an error condition.

Keys are stored in an internal linked list. Access to this list is abstracted from other drivers and should not be done directly.

Keys are 128 bits long. The lower 64 bits are randomly generated while the upper 64 bits are a magic that indicate the type of access allowed for this key.
</br>
</br>

# Demo Access Variable Driver
Behaves much like the standard SetVariable and GetVariable, but requires keys to access the variable's data.
## Driver Specifications
1. The driver MUST have access to the Access Key Protocol.
2. The driver MUST allocate and initialize the storage for NVRAM variable storage.
3. The driver MUST ONLY provide access to its variable set and get functionality via the system table.
4. The driver MUST NOT provide a protocol.

## Set and Get Functions
### SetAccessVariable
1. The driver MUST provide a function for the SetAccessVariable function in the RuntimeServices system table.
2. The SetAccessVariable function function prototype MUST be:
```
gST->RuntimeServices->SetAccessVariable ( 
    IN CHAR16    *VariableName,
    IN EFI_GUID  *VendorGuid,  
    IN UINT32    Attributes, 
    IN DEMO1_ACCESS_KEY *AccessKey, 
    IN UINTN     DataSize,  
    IN VOID      *Data 
    );
```
3. To edit/create a variable, the access key provided must have WRITE MAGIC.
4. If a new variable is created, then the access key MUST be stored with the variable.

### GetAccessVariable
1. The driver MUST provide a function for the GetAccessVariable function in the RuntimeServices system table.
2. The GetAccessVariable function prototype MUST be:
```
gST->RuntimeServices->GetAccessVariable (
    IN      CHAR16    *VariableName,
    IN      EFI_GUID  *VendorGuid,
    OUT     UINT32    *Attributes OPTIONAL,
    IN DEMO1_ACCESS_KEY *AccessKey, 
    IN OUT  UINTN     *DataSize,
    OUT     VOID      *Data OPTIONAL
    );
```
</br>
</br>

## Prose Overview

The Variable Service provides drivers with a secure location where they can store shared information. It allows for protecting read and write access based upon keys generated using the Access protocol.

# Alice Driver
## Driver Specifications
1. The driver MUST have access to the Random Number Generator Protocol.
2. The driver MUST have access to the Access Key Protocol.
3. The driver MUST create an event to indicate that the driver is ready to run - completed the INIT phase.
4. The driver MUST create an NVRAM variable to store the status of the current mode.
5. The mode MUST only be INIT or RUN.
6. The driver MUST provide a protocol for supplying data dependent on current mode.
## Function Protocol
1. If the mode is INIT, then the function MUST perform an internal function.
2. If the mode is RUN, then the function MUST provide a random number.
</br>
</br>

# Bob Driver
## Driver Specifications
1. The driver MUST have access to the Access Key Protocol.
2. The driver MUST have access to the Alice Protocol.
3. The driver MUST validate Alice is in INIT mode before completing initialization.
4. The driver MUST create a timer event for 5 seconds, to perform its actions.
5. The driver MUST provide a protocol for supplying data dependent on current mode.
## Event Timer
1. Every 5 seconds, the function MUST check the mode of Alice and perform the associated action.
2. If the mode is INIT, then the function MUST call the function provided by Alice.
3. If the mode is RUN, then the function MUST store the value provided by Alice.
4. If the mode is NOT RUN and NOT INIT, the function MUST perform no actions.
## Function Protocol
1. The function MUST check the mode of Alice.
2. If the mode is RUN, then the driver MUST supply data.
3. If the mode is NOT RUN, then the driver MUST return EFI_NOT_READY and no data.

# Alice and Bob overview

Alice is a data provider that saves specific values based upon the system state. Bob is a data consumer and interprets the data provided by Alice based upon the current state of the system. These can be imagined as background services performing some system task at regular intervals.