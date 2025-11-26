AJSM Features
The AJSM has four features: a TIprogrammed visible unlock code, a customer defined 128-bit key
protected by ECC, a scan path for unlocking the JTAG scan chain, and a key for permanently locking a device. The devices shipped from TI are programmed with the visible unlock code; any other value will result in locking a device. You can lock the device by programming any number of bits and your ECC. If all the bits are programmed (0), the device will be permanently locked.

1.1 AJSM Key Generation
The AJSM key is stored in memory that only allows changing 1s to 0s. TI ships the devices programmed with the 128-bit visible unlock code at address 0xF00000000 and your ECC.
TI Visible Unlock Code (TMS570 [Big Endian] ECC) 0xF00000000: 0xEFFDFFFF 0xFFFFFFFF 0xFFFDFFFE
0xFFEFFFFF ECC: 0xFFFF
This code is mainly 1s to allow for a custom key effective to 123 bits. The ECC is calculated based on the device's endianness. Hercules TMS570 devices are big endian, their visible unlock code ECC has been designed to be all 1s so any key can be selected. Hercules RM devices are little endian, their visible unlock code ECC will be 0xEDCO. Because the ECC is not all 1s, some custom unlock code values will not be possible. Read the value at 0xF000000 for your devices specific AJSM unlock code and 0xF0040000 for the ECC.

1.1.1 RM [Little Endian] Devices
Because the ECC is calculated for the device endianness, the ECC for little endian devices will be
0xEDC0, limiting the custom key effective to 115 bits.

1.1.2 RM57 and TMS570LC4x Devices
There is an erratum for silicon versions before Rev. B that limits the custom key effective to 54 bits. Rev.B silicon was fixed so that the visible unlock code was the same as the other devices in the family and the ECC is 0xEDED for both big and little endian devices. The effective number of bits for Rev. B and later RM57 and TMS570LC4x devices is 119 bits.

1.1.3 HALCoGen Key Generation
HALCoGen supports AJSM Key generation from HALCoGen version 4.06.00