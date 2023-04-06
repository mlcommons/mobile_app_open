//==============================================================================
//
//  Copyright (c) 2020 Qualcomm Technologies, Inc.
//  All Rights Reserved.
//  Confidential and Proprietary - Qualcomm Technologies, Inc.
//
//==============================================================================

#ifndef _ACPITABL_H
#define _ACPITABL_H

#pragma once

#ifdef ASL_ASSEMBLER
#undef PHYSICAL_ADDRESS
#define PHYSICAL_ADDRESS ULONGLONG
#endif

#ifndef PHYSICAL_ADDRESS
typedef LARGE_INTEGER PHYSICAL_ADDRESS, *PPHYSICAL_ADDRESS;
#endif

//
// These map to bios provided structures, so turn on 1 byte packing
//

#include <pshpack1.h>

// Generic Register Address Structure

typedef struct _GEN_ADDR {
  UCHAR AddressSpaceID;
  UCHAR BitWidth;
  UCHAR BitOffset;
  UCHAR AccessSize;
  PHYSICAL_ADDRESS Address;
} GEN_ADDR, *PGEN_ADDR;

#define RSDP_SIGNATURE 0x2052545020445352  // "RSD PTR "

typedef struct _RSDP {  // Root System Description Table Pointer Structure
  ULONGLONG Signature;  // 8 UCHAR table signature 'RSD PTR '
  UCHAR
  Checksum;  // checksum for first 20 bytes of table (entire ACPI 1.0 table)
  UCHAR OEMID[6];     //      String that uniquely ID's the OEM
  UCHAR Revision;     // 0 - ACPI 1.0;  2 - ACPI 2.0
  ULONG RsdtAddress;  // physical address of Root System Description Table  (1.0
                      // table ended here)
  ULONG Length;       // Length of the table in bytes
  PHYSICAL_ADDRESS XsdtAddress;  // physical address of XSDT
  UCHAR XChecksum;               // checksum for entire table
  UCHAR Reserved[3];
} RSDP, *PRSDP;

#define RSDP_SEARCH_RANGE_BEGIN \
  0xE0000  // physical address where we begin searching for the RSDP
#define RSDP_SEARCH_RANGE_END 0xFFFFF
#define RSDP_SEARCH_RANGE_LENGTH \
  (RSDP_SEARCH_RANGE_END - RSDP_SEARCH_RANGE_BEGIN + 1)
#define RSDP_SEARCH_INTERVAL 16  // search on 16 byte boundaries

//
// Maximum lengths in bytes
//

#define ACPI_MAX_SIGNATURE 4
#define ACPI_MAX_OEM_ID 6
#define ACPI_MAX_TABLE_ID 8
#define ACPI_MAX_CREATOR_ID 4

typedef struct _DESCRIPTION_HEADER {  // Header structure appears at the
                                      // beginning of each ACPI table
  ULONG Signature;  //      Signature used to identify the type of table
  ULONG Length;  //      Length of entire table including the DESCRIPTION_HEADER
  UCHAR
  Revision;  //      Minor version of ACPI spec to which this table conforms
  UCHAR Checksum;  //      sum of all bytes in the entire TABLE should = 0
  CHAR OEMID[ACPI_MAX_OEM_ID];         //      String that uniquely ID's the OEM
  CHAR OEMTableID[ACPI_MAX_TABLE_ID];  //      String that uniquely ID's this
                                       //      table (used for table patching
                                       //      and replacement).
  ULONG OEMRevision;  //      OEM supplied table revision number.  Bigger number
                      //      = newer table.
  CHAR CreatorID[ACPI_MAX_CREATOR_ID];  //      Vendor ID of utility which
                                        //      created this table.
  ULONG CreatorRev;  //      Revision of utility that created the table.
} DESCRIPTION_HEADER, *PDESCRIPTION_HEADER;

#define FACS_SIGNATURE 0x53434146  // "FACS"

typedef enum _ACPI_GENERIC_ADDRESS_TYPE {
  AcpiGenericSpaceMemory = 0,
  AcpiGenericSpaceIO,
  AcpiGenericSpacePciConfig,
  AcpiGenericSpaceEC,
  AcpiGenericSpaceSMBus,
  AcpiGenericSpacePcc = 0x0a,
  AcpiGenericSpaceFixedFunction = 0x7F
} ACPI_GENERIC_ADDRESS_TYPE,
    *PACPI_GENERIC_ADDRESS_TYPE;

typedef enum _ACPI_GENERIC_ACCESS_SIZE {
  AcpiGenericAccessSizeLegacy = 0,
  AcpiGenericAccessSizeByte,
  AcpiGenericAccessSizeWord,
  AcpiGenericAccessSizeDWord,
  AcpiGenericAccessSizeQWord
} ACPI_GENERIC_ACCESS_SIZE,
    *PACPI_GENERIC_ACCESS_SIZE;

typedef struct _FACS {  // Firmware ACPI Control Structure.  Note that this
                        // table does not have a header, it is pointed to by the
                        // FADT
  ULONG Signature;      //      'FACS'
  ULONG Length;  //      Length of entire firmware ACPI control structure (must
                 //      be 64 bytes or larger)
  ULONG HardwareSignature;
  ULONG pFirmwareWakingVector;  // physical address of location where the OS
                                // needs to put the firmware waking vector
  ULONG GlobalLock;  // 32 bit structure used for sharing Embedded Controller
  ULONG Flags;
  PHYSICAL_ADDRESS x_FirmwareWakingVector;  // 64-bit capable firmware vector
  UCHAR version;
  UCHAR Reserved[31];
} FACS, *PFACS;

// FACS.GlobalLock bit field definitions

#define GL_PENDING_BIT 0x00
#define GL_PENDING (1 << GL_PENDING_BIT)

#define GL_OWNER_BIT 0x01
#define GL_OWNER (1 << GL_OWNER_BIT)

#define GL_NON_RESERVED_BITS_MASK (GL_PENDING + GL_OWNED)

// FACS Flags definitions

#define FACS_S4BIOS_SUPPORTED_BIT \
  0  // flag indicates whether or not the BIOS will save/restore memory around
     // S4
#define FACS_S4BIOS_SUPPORTED (1 << FACS_S4BIOS_SUPPORTED_BIT)

#define FADT_SIGNATURE 0x50434146  // "FACP"

typedef struct _FADT {  // Fixed ACPI description table

  DESCRIPTION_HEADER Header;

  ULONG facs;  // Physical address of the Firmware ACPI Control Structure
  ULONG
  dsdt;  // Physical address of the Differentiated System Description Table
  UCHAR int_model;   // System's Interrupt mode, 0=Dual PIC, 1=Multiple APIC, >1
                     // reserved
  UCHAR pm_profile;  // System's preferred power profile
  USHORT sci_int_vector;  // Vector of SCI interrupt.
  ULONG smi_cmd_io_port;  // Address in System I/O Space of the SMI Command
                          // port, used to enable and disable ACPI.
  UCHAR acpi_on_value;    // Value out'd to smi_cmd_port to activate ACPI
  UCHAR acpi_off_value;   // Value out'd to smi_cmd_port to deactivate ACPI
  UCHAR s4bios_req;       // Value to write to SMI_CMD to enter the S4 state.
  UCHAR pstate_control;   // Value to write to SMI_CMD to assume control of
                          // processor performance states
  ULONG pm1a_evt_blk_io_port;   // Address in System I/O Space of the
                                // PM1a_EVT_BLK register block
  ULONG pm1b_evt_blk_io_port;   // Address in System I/O Space of the
                                // PM1b_EVT_BLK register block
  ULONG pm1a_ctrl_blk_io_port;  // Address in System I/O Space of the
                                // PM1a_CNT_BLK register block
  ULONG pm1b_ctrl_blk_io_port;  // Address in System I/O Space of the
                                // PM1b_CNT_BLK register block
  ULONG pm2_ctrl_blk_io_port;  // Address in System I/O Space of the PM2_CNT_BLK
                               // register block
  ULONG pm_tmr_blk_io_port;    // Address in System I/O Space of the PM_TMR
                               // register block
  ULONG
  gp0_blk_io_port;  // Address in System I/O Space of the GP0 register block
  ULONG
  gp1_blk_io_port;     // Address in System I/O Space of the GP1 register block
  UCHAR pm1_evt_len;   // number of bytes decoded for PM1_BLK (must be >= 4)
  UCHAR pm1_ctrl_len;  // number of bytes decoded for PM1_CNT (must be >= 2)
  UCHAR pm2_ctrl_len;  // number of bytes decoded for PM1a_CNT (must be >= 1)
  UCHAR pm_tmr_len;    // number of bytes decoded for PM_TMR (must be >= 4)
  UCHAR gp0_blk_len;   // number of bytes decoded for GP0_BLK (must be multiple
                       // of 2)
  UCHAR gp1_blk_len;   // number of bytes decoded for GP1_BLK (must be multiple
                       // of 2)
  UCHAR gp1_base;      // index at which GP1 based events start
  UCHAR cstate_control;  // Value to write to SMI_CMD to assume control of _CST
                         // states
  USHORT lvl2_latency;   // Worst case latency in microseconds required to enter
                         // and leave the C2 processor state
  USHORT lvl3_latency;   // Worst case latency in microseconds required to enter
                         // and leave the C3 processor state
  USHORT flush_size;  // Ignored if WBINVD flag is 1 -- indicates size of memory
                      // read to flush dirty lines from
                      //      any processors memory caches. A size of zero
                      //      indicates this is not supported.
  USHORT flush_stride;  // Ignored if WBINVD flag is 1 -- the memory stride
                        // width, in bytes, to perform reads to flush
                        //      the processor's memory caches.
  UCHAR duty_offset;    // zero based index of where the processor's duty cycle
                        // setting is within the processor's P_CNT register.
  UCHAR duty_width;  // bit width of the processor's duty cycle setting value in
                     // the P_CNT register.
                     //      a value of zero indicates that processor duty cycle
                     //      is not supported
  UCHAR day_alarm_index;
  UCHAR month_alarm_index;
  UCHAR century_alarm_index;
  USHORT boot_arch;
  UCHAR reserved3[1];
  ULONG flags;  // This is the last field if the table Revision is 1
  GEN_ADDR reset_reg;
  UCHAR reset_val;       // This is the last field if the table Revision is 2
  USHORT arm_boot_arch;  // Pending ECR (expected to be in Revision 5.0b)
  UCHAR minor_version_number;
  PHYSICAL_ADDRESS x_firmware_ctrl;
  PHYSICAL_ADDRESS x_dsdt;
  GEN_ADDR x_pm1a_evt_blk;
  GEN_ADDR x_pm1b_evt_blk;
  GEN_ADDR x_pm1a_ctrl_blk;
  GEN_ADDR x_pm1b_ctrl_blk;
  GEN_ADDR x_pm2_ctrl_blk;
  GEN_ADDR x_pm_tmr_blk;
  GEN_ADDR x_gp0_blk;
  GEN_ADDR x_gp1_blk;  // This is the last field if the table Revision is 3 or 4
  GEN_ADDR sleep_control_reg;
  GEN_ADDR
  sleep_status_reg;  // This is the last field if the table Revision is 5
} FADT, *PFADT;

#define FADT_REV_1_SIZE (FIELD_OFFSET(FADT, flags) + sizeof(ULONG))
#define FADT_REV_2_SIZE (FIELD_OFFSET(FADT, reset_val) + sizeof(UCHAR))
#define FADT_REV_3_SIZE (FIELD_OFFSET(FADT, x_gp1_blk) + sizeof(GEN_ADDR))
#define FADT_REV_4_SIZE (FIELD_OFFSET(FADT, x_gp1_blk) + sizeof(GEN_ADDR))
#define FADT_REV_5_SIZE \
  (FIELD_OFFSET(FADT, sleep_status_reg) + sizeof(GEN_ADDR))

#define ACPI_IS_VALID_TABLE_ENTRY(Entry, Type, TableEnd)                    \
  ((((PUCHAR)(Entry)) + RTL_SIZEOF_THROUGH_FIELD(Type, Length) <=           \
    (PUCHAR)(TableEnd)) &&                                                  \
   (((Type *)(Entry))->Length >= RTL_SIZEOF_THROUGH_FIELD(Type, Length)) && \
   (((PUCHAR)(Entry)) + ((Type *)(Entry))->Length <= (PUCHAR)(TableEnd)))

//
// System Resource Affinity Table
//
// This table describes the system topology of a ccNUMA machine.
//

#define ACPI_SRAT_SIGNATURE 0x54415253  // "SRAT"

typedef struct _ACPI_SRAT {
  DESCRIPTION_HEADER Header;
  ULONG TableRevision;
  ULONG Reserved[2];
} ACPI_SRAT, *PACPI_SRAT;

#if _MSC_VER >= 1200
#pragma warning(push)
#endif

#pragma warning(disable : 4214)  // nonstandard extension used : bit field types
                                 // other than int
#pragma warning( \
    disable : 4201)  // nonstandard extension used : nameless struct/union

typedef struct _ACPI_SRAT_ENTRY {
  UCHAR Type;
  UCHAR Length;
  union {
    struct {
      UCHAR ProximityDomainLow;
      UCHAR ApicId;
      struct {
        ULONG Enabled : 1;
        ULONG Reserved : 31;
      } Flags;
      UCHAR SApicEid;
      UCHAR ProximityDomainHigh[3];
      ULONG ClockDomain;
    } ApicAffinity;
    struct {
      ULONG ProximityDomain;
      UCHAR Reserved[2];
      PHYSICAL_ADDRESS Base;
      ULONGLONG Length;
      ULONG Reserved2;
      struct {
        ULONG Enabled : 1;
        ULONG HotPlug : 1;
        ULONG Reserved : 30;
      } Flags;
      UCHAR Reserved3[8];
    } MemoryAffinity;
    struct {
      UCHAR Reserved[2];
      ULONG ProximityDomain;
      ULONG ApicId;
      struct {
        ULONG Enabled : 1;
        ULONG Reserved : 31;
      } Flags;
      ULONG ClockDomain;
      ULONG Reserved2;
    } X2ApicAffinity;
    struct {
      ULONG ProximityDomain;
      ULONG ProcessorUid;
      struct {
        ULONG Enabled : 1;
        ULONG Reserved : 31;
      } Flags;
      ULONG ClockDomain;
    } GiccAffinity;
  } DUMMYUNIONNAME;
} ACPI_SRAT_ENTRY, *PACPI_SRAT_ENTRY;

#define SRAT_APIC_ENTRY_LENGTH                   \
  (FIELD_OFFSET(ACPI_SRAT_ENTRY, ApicAffinity) + \
   RTL_FIELD_SIZE(ACPI_SRAT_ENTRY, ApicAffinity))

#define SRAT_MEMORY_ENTRY_LENGTH                   \
  (FIELD_OFFSET(ACPI_SRAT_ENTRY, MemoryAffinity) + \
   RTL_FIELD_SIZE(ACPI_SRAT_ENTRY, MemoryAffinity))

#define SRAT_X2APIC_ENTRY_LENGTH                   \
  (FIELD_OFFSET(ACPI_SRAT_ENTRY, X2ApicAffinity) + \
   RTL_FIELD_SIZE(ACPI_SRAT_ENTRY, X2ApicAffinity))

#define SRAT_GICC_ENTRY_LENGTH                   \
  (FIELD_OFFSET(ACPI_SRAT_ENTRY, GiccAffinity) + \
   RTL_FIELD_SIZE(ACPI_SRAT_ENTRY, GiccAffinity))

#define PROXIMITY_DOMAIN(SratTable, SratEntry)                         \
  (((SratTable)->Header.Revision == 1) ? PROXIMITY_DOMAIN_1(SratEntry) \
                                       : PROXIMITY_DOMAIN_2(SratEntry))

#define PROXIMITY_DOMAIN_1(SratEntry) \
  (SratEntry)->ApicAffinity.ProximityDomainLow

#define PROXIMITY_DOMAIN_2(SratEntry)                                          \
  (((SratEntry)->Type == SratProcessorLocalAPIC)                               \
       ? (((ULONG)((SratEntry)->ApicAffinity.ProximityDomainLow)) +            \
          (((ULONG)((SratEntry)->ApicAffinity.ProximityDomainHigh[0])) << 8) + \
          (((ULONG)((SratEntry)->ApicAffinity.ProximityDomainHigh[1]))         \
           << 16) +                                                            \
          (((ULONG)((SratEntry)->ApicAffinity.ProximityDomainHigh[2])) << 24)) \
       : (((SratEntry)->Type == SratProcessorLocalX2APIC)                      \
              ? (SratEntry)->X2ApicAffinity.ProximityDomain                    \
              : (((SratEntry)->Type == SratMemory)                             \
                     ? (SratEntry)->MemoryAffinity.ProximityDomain             \
                     : (SratEntry)->GiccAffinity.ProximityDomain)))

#if _MSC_VER >= 1200
#pragma warning(pop)
#endif

typedef enum {
  SratProcessorLocalAPIC,
  SratMemory,
  SratProcessorLocalX2APIC,
  SratGicc
} SRAT_ENTRY_TYPE;

#define ACPI_MPST_SIGNATURE 0x5453504D  // "MPST"

//
// Memory power state table : this describes the system's memory power nodes.
//

typedef struct _MEMORY_POWER_STATE {
  UCHAR PowerStateValue;
  UCHAR PowerStateInformationIndex;
} MEMORY_POWER_STATE, *PMEMORY_POWER_STATE;

typedef struct _MEMORY_POWER_NODE {
  UCHAR Flag;
  UCHAR Reserved1;
  USHORT MpnId;
  ULONG Length;
  ULONG BaseAddressLow;
  ULONG BaseAddressHigh;
  ULONG LengthLow;
  ULONG LengthHigh;
  ULONG PowerStateCount;
  ULONG PhysicalComponentCount;
  MEMORY_POWER_STATE MpState;  // Start of PowerStateCount structures,
                               // followed by 'PhysicalComponentCount'
                               // physical component identifiers.
} MEMORY_POWER_NODE, *PMEMORY_POWER_NODE;

//
// MPST table: Memory Power State Table.
//
// This table contains information about:
//
// a) MPNs (memory power nodes): How many MPNs there are and their
//    characteristics. Characteristics include their base addresses,
//    lengths, and identifiers of physical components which compose
//    the MPN.  As well as which power states are present and their
//    index in the memory power state characteristics table which is
//    in the MPST table itself.
//
// b) Memory power state characteristics table: This table is an array which
//    describes each state's characteristics such as average power consumed,
//    exit latency etc. All we look at from this table is the flags, which tell
//    us whether contents can be preserved in that power state and if that
//    power state can be exited autonomously upon a memory reference.
//
// There can be multiple MPNs exposed by firmware but it cannot be
// determined by examination whether a given MPN represents a
// Channel/DIMM/Rank/Bank.  Thus it is recommended in the platform
// developers guide to populate the MPST tables with appropriate MPNs.
//

typedef struct _ACPI_MPST {
  DESCRIPTION_HEADER Header;
  UCHAR SubspaceId;
  UCHAR Reserved2[3];
  USHORT MpnCount;
  UCHAR Reserved[2];
  MEMORY_POWER_NODE Mpn;  // MpnCount Mpn structures begin here.
                          // Followed by a USHORT memory power
                          // characteristic count.
                          // Followed by an array of memory power
                          // state characteristics structures.
} ACPI_MPST, *PACPI_MPST;

#if _MSC_VER >= 1200
#pragma warning(push)
#endif

#pragma warning(disable : 4214)  // nonstandard extension used : bit field types
                                 // other than int
#pragma warning( \
    disable : 4201)  // nonstandard extension used : nameless struct/union

typedef struct _POWER_STATE_CHARACTERISTICS {
  union {
    UINT8 AsUINT8;
    struct {
      UINT8 Value : 6;
      UINT8 Revision : 2;
    } DUMMYSTRUCTNAME;
  } ID;
  UINT8 Flag;
  USHORT Reserved1;
  ULONG PowerInMPS0;
  ULONG PowerSavingToMPS0;
  ULONGLONG ExitLatencyNs;
  UCHAR Reserved2[8];
} POWER_STATE_CHARACTERISTICS, *PPOWER_STATE_CHARACTERISTICS;

#if _MSC_VER >= 1200
#pragma warning(pop)
#endif

//
// Definition of shared memory region to initialize before issuing PCC commands.
//

typedef struct _MEMORY_COOLING_REGION {
  ULONG PowerCommandRegister;
  ULONG PowerStatusRegister;
  ULONG PowerStateId;
  ULONG MpnId;
  ULONGLONG EnergyConsumed;
  ULONGLONG ExpectedPowerConsumed;
} MEMORY_COOLING_REGION, *PMEMORY_COOLING_REGION;

//
// Maximum System Characteristics Table (MSCT)
//
// This table describes maximum topology capabilities of a ccNUMA machine.
//

#define ACPI_MSCT_SIGNATURE 0x5443534D  // "MSCT"

typedef struct _ACPI_MSCT {
  DESCRIPTION_HEADER Header;
  ULONG DomainInfoOffset;
  ULONG ProximityDomainCount;
  ULONG ClockDomainCount;
  PHYSICAL_ADDRESS MaximumPhysicalAddress;
} ACPI_MSCT, *PACPI_MSCT;

typedef struct _ACPI_MSCT_ENTRY {
  UCHAR Revision;
  UCHAR Length;
  ULONG DomainIdRangeLow;
  ULONG DomainIdRangeHigh;
  ULONG ProcessorCapacity;
  ULONG64 MemoryCapacity;
} ACPI_MSCT_ENTRY, *PACPI_MSCT_ENTRY;

#define ACPI_MSCT_MINIMUM_LENGTH sizeof(ACPI_MSCT)

#ifdef _IA64_
// FLUSH WORKS IS FOR IA64
#define FLUSH_WORKS_BIT 0
#define FLUSH_WORKS (1 << FLUSH_WORKS_BIT)
#endif  // IA64

// definition of FADT.flags bits

// this one bit flag indicates whether or not the WBINVD instruction works
// properly,if this bit is not set we can not use S2, S3 states, or C3 on MP
// machines
#define WRITEBACKINVALIDATE_WORKS_BIT 0
#define WRITEBACKINVALIDATE_WORKS (1 << WRITEBACKINVALIDATE_WORKS_BIT)

//  this flag indicates if wbinvd works EXCEPT that it does not invalidate the
//  cache
#define WRITEBACKINVALIDATE_DOESNT_INVALIDATE_BIT 1
#define WRITEBACKINVALIDATE_DOESNT_INVALIDATE \
  (1 << WRITEBACKINVALIDATE_DOESNT_INVALIDATE_BIT)

//  this flag indicates that the C1 state is supported on all processors.
#define SYSTEM_SUPPORTS_C1_BIT 2
#define SYSTEM_SUPPORTS_C1 (1 << SYSTEM_SUPPORTS_C1_BIT)

// this one bit flag indicates whether support for the C2 state is restricted to
// uniprocessor machines
#define P_LVL2_UP_ONLY_BIT 3
#define P_LVL2_UP_ONLY (1 << P_LVL2_UP_ONLY_BIT)

//      this bit indicates whether the PWR button is treated as a fix feature
//      (0) or a generic feature (1)
#define PWR_BUTTON_GENERIC_BIT 4
#define PWR_BUTTON_GENERIC (1 << PWR_BUTTON_GENERIC_BIT)

#define SLEEP_BUTTON_GENERIC_BIT 5
#define SLEEP_BUTTON_GENERIC (1 << SLEEP_BUTTON_GENERIC_BIT)

//      this bit indicates whether the RTC wakeup status is reported in fix
//      register space (0) or not (1)
#define RTC_WAKE_GENERIC_BIT 6
#define RTC_WAKE_GENERIC (1 << RTC_WAKE_GENERIC_BIT)

#define RTC_WAKE_FROM_S4_BIT 7
#define RTC_WAKE_FROM_S4 (1 << RTC_WAKE_FROM_S4_BIT)

// This bit indicates whether the machine implements a 24 or 32 bit timer.
#define TMR_VAL_EXT_BIT 8
#define TMR_VAL_EXT (1 << TMR_VAL_EXT_BIT)

// This bit indicates whether the machine supports docking
#define DCK_CAP_BIT 9
#define DCK_CAP (1 << DCK_CAP_BIT)

// This bit indicates whether the machine supports reset
#define RESET_CAP_BIT 10
#define RESET_CAP (1 << RESET_CAP_BIT)

// This bit indicates whether the machine case can be opened
#define SEALED_CASE_BIT 11
#define SEALED_CASE_CAP (1 << SEALED_CASE_BIT)

// This bit indicates whether the machine has no video
#define HEADLESS_BIT 12
#define HEADLESS_CAP (1 << HEADLESS_BIT)

//
// This bit indicates whether a processor native instruction must be executed
// after writing the SLP_TYPx register.
//

#define CPU_SW_SLP 13
#define CPU_SW_SLP_CAP (1 << CPU_SW_SLP)

//
// If set, indicates the platform supports the PCIEXP_WAKE_STS bit in the PM1
// Status register and the PCIEXP_WAKE_EN bit in the PM1 Enable register.
//

#define PCI_EXP_WAK_BIT 14
#define PCI_EXP_WAK (1 << PCI_EXP_WAK_BIT)

// This bit indicates whether to use an alternate clock, i.e., HPET or PM Timer
#define USE_PLATFORM_CLOCK_BIT 15
#define USE_PLATFORM_CLOCK (1 << USE_PLATFORM_CLOCK_BIT)

//
// This bit indicates whether or not the RTC_STS flag in pm1_evt_blk is valid or
// not when coming out of S4. This bit is only available on FADT revisions >= 4.
//

#define RTC_WAKE_VALID_FROM_S4_BIT 16
#define RTC_WAKE_VALID_FROM_S4 (1 << RTC_WAKE_VALID_FROM_S4_BIT)

//
// Indicates whether the platform is compatible with remote power on.
//

#define REMOTE_POWER_ON_CAPABLE_BIT 17
#define REMOTE_POWER_ON_CAPABLE (1 << REMOTE_POWER_ON_CAPABLE_BIT)

//
// This bit indicates whether the machine needs to run in clustered APIC mode.
//

#define FADT_FORCE_CLUSTERED_APIC_MODE_BIT 18
#define FADT_FORCE_CLUSTERED_APIC_MODE (1 << FADT_FORCE_CLUSTERED_APIC_MODE_BIT)

//
// This bit indicates whether the machine requires all APIC messages to use
// physical destination mode.
//

#define FADT_FORCE_APIC_PHYSICAL_DESTINATION_MODE_BIT 19
#define FADT_FORCE_APIC_PHYSICAL_DESTINATION_MODE \
  (1 << FADT_FORCE_APIC_PHYSICAL_DESTINATION_MODE_BIT)

//
// If set, this flag indicates that the platform lacks ACPI hardware, i.e., the
// ACPI driver should operate in hardware reduced ACPI mode.
//

#define ACPI_HARDWARE_NOT_PRESENT_BIT 20
#define ACPI_HARDWARE_NOT_PRESENT (1 << ACPI_HARDWARE_NOT_PRESENT_BIT)

//
// If set, this flag indicates that the platform was designed to be always-on/
// always-connected.
//

#define AOAC_CAPABLE_PLATFORM_BIT 21
#define AOAC_CAPABLE_PLATFORM (1 << AOAC_CAPABLE_PLATFORM_BIT)

//
// Spec defines maximum entry/exit latency values for C2 and C3, if the FADT
// indicates that these values are exceeded then we do not use that C state.

#define C2_MAX_LATENCY 100
#define C3_MAX_LATENCY 1000

//
// Definition of FADT.boot_arch flags.  These flags are typically valid only on
// IA-PCs.
//

//
// If set, indicates that the motherboard supports user-visible devices on the
// LPC or ISA bus. User-visible devices are devices that have end-user
// accessible connectors (for example, LPT port), or devices for which the OS
// must load a device driver so that an end-user application can use a device.
// If clear, the OS may assume there are no such devices and that all devices
// in the system can be detected exclusively via industry standard device
// enumeration mechanisms (including the ACPI namespace).
//

#define LEGACY_DEVICES_BIT 0
#define LEGACY_DEVICES (1 << LEGACY_DEVICES_BIT)

//
// If set, indicates that the motherboard contains support for a port 60 and
// 64 based keyboard controller, usually implemented as an 8042 or equivalent
// micro-controller.
//

#define I8042_BIT 1
#define I8042 (1 << I8042_BIT)

//
// If set, indicates to OSPM that it must not blindly probe the VGA hardware
// (that responds to MMIO addresses A0000h-BFFFFh and IO ports 3B0h-3BBh and
// 3C0h-3DFh) that may cause machine check on this system. If clear, indicates
// to OSPM that it is safe to probe the VGA hardware.
//

#define VGA_NOT_PRESENT_BIT 2
#define VGA_NOT_PRESENT (1 << VGA_NOT_PRESENT_BIT)

//
// If set, indicates to OSPM that it must not enable Message Signaled
// Interrupts (MSI) on this platform.
//

#define MSI_NOT_SUPPORTED_BIT 3
#define MSI_NOT_SUPPORTED (1 << MSI_NOT_SUPPORTED_BIT)

//
// If set, indicates to OSPM that it must not enable Active State Power
// Management (ASPM) on this platform.
//

#define ASPM_NOT_SUPPORTED_BIT 4
#define ASPM_NOT_SUPPORTED (1 << ASPM_NOT_SUPPORTED_BIT)

//
// If set indicates that legacy CMOS RTC is not present in the system.
//

#define CMOS_RTC_NOT_PRESENT_BIT 5
#define CMOS_RTC_NOT_PRESENT (1 << CMOS_RTC_NOT_PRESENT_BIT)

//
// Definition of FADT.arm_boot_arch flags. These flags are valid only on ARM.
//

//
// If set indicates that PSCI is implemented.
//

#define ARM_BOOT_ARCH_PSCI_COMPLIANT_BIT 0
#define ARM_BOOT_ARCH_PSCI_COMPLIANT (1 << ARM_BOOT_ARCH_PSCI_COMPLIANT_BIT)

//
// If set indicates that HVC should be used instead of SMC for PSCI.
//

#define ARM_BOOT_ARCH_PSCI_USE_HVC_BIT 1
#define ARM_BOOT_ARCH_PSCI_USE_HVC (1 << ARM_BOOT_ARCH_PSCI_USE_HVC_BIT)

#ifndef ANYSIZE_ARRAY
#define ANYSIZE_ARRAY 1
#endif

// Multiple APIC description table

typedef struct _MAPIC {
  DESCRIPTION_HEADER Header;
  ULONG LocalAPICAddress;  // Physical Address at which each processor can
                           // access its local APIC
  ULONG Flags;
  ULONG APICTables[ANYSIZE_ARRAY];  // A list of APIC tables.

} MAPIC;

typedef MAPIC *PMAPIC;

// Multiple APIC structure flags

#define PCAT_COMPAT_BIT \
  0  // indicates that the system also has a dual 8259 pic setup.
#define PCAT_COMPAT (1 << PCAT_COMPAT_BIT)

// APIC Structure Types
#define PROCESSOR_LOCAL_APIC 0
#define IO_APIC 1
#define ISA_VECTOR_OVERRIDE 2
#define IO_NMI_SOURCE 3
#define LOCAL_NMI_SOURCE 4
#define ADDRESS_EXTENSION_STRUCTURE 5
#define IO_SAPIC 6
#define LOCAL_SAPIC 7
#define PLATFORM_INTERRUPT_SOURCE 8
#define PROCESSOR_LOCAL_X2APIC 9
#define LOCAL_X2APIC_NMI_SOURCE 10
#define PROCESSOR_LOCAL_GIC 11
#define GICDISTRIBUTOR 12
#define GICMSIFRAME 13
#define GICREDISTRIBUTOR 14
#define GICITS 15

#define PROCESSOR_LOCAL_APIC_LENGTH 8
#define IO_APIC_LENGTH 12
#define ISA_VECTOR_OVERRIDE_LENGTH 10

#define IO_NMI_SOURCE_LENGTH 8
#define LOCAL_NMI_SOURCE_LENGTH 6
#define PLATFORM_INTERRUPT_SOURCE_LENGTH 16
#define IO_SAPIC_LENGTH 16
#define PROCESSOR_LOCAL_SAPIC_LENGTH 12  // minimal length.
#define PROCESSOR_LOCAL_X2APIC_LENGTH 16
#define LOCAL_X2APIC_NMI_SOURCE_LENGTH 12

// Platform Interrupt Types
#define PLATFORM_INT_PMI 1
#define PLATFORM_INT_INIT 2
#define PLATFORM_INT_CPE 3

// These defines come from the MPS 1.4 spec, section 4.3.4 and they are
// referenced as such in the ACPI spec.
#define PO_BITS 3
#define POLARITY_HIGH 1
#define POLARITY_LOW 3
#define POLARITY_CONFORMS_WITH_BUS 0
#define EL_BITS 0xc
#define EL_BIT_SHIFT 2
#define EL_EDGE_TRIGGERED 4
#define EL_LEVEL_TRIGGERED 0xc
#define EL_CONFORMS_WITH_BUS 0

// The shared beginning info in all APIC Structures

typedef struct _APICTABLE {
  UCHAR Type;
  UCHAR Length;
} APICTABLE;

typedef APICTABLE UNALIGNED *PAPICTABLE;

typedef struct _PROCLOCALAPIC {
  UCHAR Type;    // should be zero to identify a ProcessorLocalAPIC structure
  UCHAR Length;  // better be 8
  UCHAR ACPIProcessorID;  // ProcessorID for which this processor is listed in
                          // the ACPI processor declaration operator.
  UCHAR APICID;           //  The processor's local APIC ID.
  ULONG Flags;

} PROCLOCALAPIC;

typedef PROCLOCALAPIC UNALIGNED *PPROCLOCALAPIC;

// Processor Local APIC, SAPIC and x2APIC Flags
#define PLAF_ENABLED_BIT 0
#define PLAF_ENABLED (1 << PLAF_ENABLED_BIT)

typedef struct _IOAPIC {
  UCHAR Type;
  UCHAR Length;  // better be 12
  UCHAR IOAPICID;
  UCHAR Reserved;
  ULONG IOAPICAddress;     // Physical address at which this IO APIC resides.
  ULONG SystemVectorBase;  // system interrupt vector index for this APIC

} IOAPIC;

typedef IOAPIC UNALIGNED *PIOAPIC;

// Interrupt Source Override
typedef struct _ISA_VECTOR {
  UCHAR Type;                         // Must be 2
  UCHAR Length;                       // Must be 10
  UCHAR Bus;                          // Must be 0
  UCHAR Source;                       // BusRelative IRQ
  ULONG GlobalSystemInterruptVector;  // Global IRQ
  USHORT Flags;                       // Same as MPS INTI Flags
} ISA_VECTOR;

typedef ISA_VECTOR UNALIGNED *PISA_VECTOR;

// I/O Non-Maskable Source Interrupt
typedef struct _IO_NMISOURCE {
  UCHAR Type;                         // must be 3
  UCHAR Length;                       // better be 8
  USHORT Flags;                       // Same as MPS INTI Flags
  ULONG GlobalSystemInterruptVector;  // Interrupt connected to NMI
} IO_NMISOURCE;

typedef IO_NMISOURCE UNALIGNED *PIO_NMISOURCE;

// Local Non-Maskable Interrupt Source
typedef struct _LOCAL_NMISOURCE {
  UCHAR Type;         // must be 4
  UCHAR Length;       // better be 6
  UCHAR ProcessorID;  // which processor?  0xff means all
  USHORT Flags;
  UCHAR LINTIN;  // which LINTIN# signal on the processor
} LOCAL_NMISOURCE;

typedef LOCAL_NMISOURCE UNALIGNED *PLOCAL_NMISOURCE;

typedef struct _PROCLOCALSAPIC {
  UCHAR Type;             // LOCAL_SAPIC
  UCHAR Length;           // PROCESSOR_LOCAL_SAPIC_LENGTH
  UCHAR ACPIProcessorID;  // ProcessorID for which this processor is listed in
                          // the ACPI processor declaration
  UCHAR APICID;           //  The processor's local APIC ID.
  UCHAR APICEID;          //  The processor's local APIC EID.
  UCHAR Reserved[3];
  ULONG Flags;
  ULONG ACPIProcessorUIDInteger;
  CHAR ACPIProcessorUIDString[ANYSIZE_ARRAY];
} PROCLOCALSAPIC;

typedef PROCLOCALSAPIC UNALIGNED *PPROCLOCALSAPIC;

typedef struct _IOSAPIC {
  UCHAR Type;    // IO_SAPIC
  UCHAR Length;  // IO_SAPIC_LENGTH
  UCHAR IOSAPICID;
  UCHAR Reserved;
  ULONG SystemVectorBase;  // system interrupt vector index for this SAPIC
  ULONG_PTR
  IOSAPICAddress;  // 64-bit Physical address at which this IO APIC resides.
} IOSAPIC;

typedef IOSAPIC UNALIGNED *PIOSAPIC;

typedef struct _PLATFORM_INTERRUPT {
  UCHAR Type;    // PLATFORM_INTERRUPT_SOURCE
  UCHAR Length;  // PLATFORM_INTERRUPT_SOURCE_LENGTH
  USHORT Flags;  // Same as MPS INTI Flags
  UCHAR InterruptType;
  UCHAR APICID;
  UCHAR ACPIEID;
  UCHAR IOSAPICVector;
  ULONG GlobalVector;
  ULONG Reserved;
} PLATFORM_INTERRUPT;

typedef PLATFORM_INTERRUPT UNALIGNED *PPLATFORM_INTERRUPT;

typedef struct _PROCLOCALX2APIC {
  UCHAR Type;    // Must be 9 (LOCAL_X2APIC)
  UCHAR Length;  // Must be 16 (LOCAL_X2APIC_LENGTH)
  USHORT Reserved;
  ULONG APICID;           // Local extended xAPIC ID
  ULONG Flags;            // Local extended xAPIC flags
  ULONG ACPIProcessorID;  // Matches _UID of the ACPI processor device
} PROCLOCALX2APIC;

typedef PROCLOCALX2APIC UNALIGNED *PPROCLOCALX2APIC;

// Local x2APIC Non-Maskable Interrupt Source
typedef struct _LOCAL_X2APIC_NMISOURCE {
  UCHAR Type;         // Must be 10 (LOCAL_X2APIC_NMI_SOURCE)
  UCHAR Length;       // Must be 12 (LOCAL_X2APIC_NMI_SOURCE_LENGTH)
  USHORT Flags;       // MPS INTI flags
  ULONG ProcessorID;  // Matches _UID of the ACPI processor device;
                      // 0xffffffff means all processors in machine
  UCHAR LINTIN;       // Extended xAPIC interrupt input LINTn to which
                      // the NMI is connected
  UCHAR Reserved[3];
} LOCAL_X2APIC_NMISOURCE;

typedef LOCAL_X2APIC_NMISOURCE UNALIGNED *PLOCAL_X2APIC_NMISOURCE;

//
// Macros that convert between the compressed and
// uncompressed representation of an MPIDR.
//

#define COMPRESS_MPIDR(_MPIDR_, _COMPRESSED_)           \
  {                                                     \
    ULONG _AFF3_;                                       \
                                                        \
    _AFF3_ = (ULONG)((_MPIDR_ & 0xFF00000000ULL) >> 8); \
    _COMPRESSED_ = (ULONG)(_MPIDR_ & 0xFFFFFF);         \
    _COMPRESSED_ |= _AFF3_;                             \
  }

#define UNCOMPRESS_MPIDR(_COMPRESSED_, _MPIDR_)                  \
  {                                                              \
    (_MPIDR_) = (ULONGLONG)(_COMPRESSED_)&0x00FFFFFFULL;         \
    (_MPIDR_) |= ((ULONGLONG)(_COMPRESSED_)&0xFF000000ULL) << 8; \
  }

typedef struct _PROCLOCALGIC {
  UCHAR Type;                           // +00  PROCESSOR_LOCAL_GIC (0xb)
  UCHAR Length;                         // +01  >= 40 (0x28)
  USHORT Reserved;                      // +02
  ULONG Identifier;                     // +04
  ULONG AcpiProcessorId;                // +08
  ULONG Flags;                          // +12
  ULONG ParkingProtocolVersion;         // +16
  ULONG PerformanceInterruptGsi;        // +20
  ULONGLONG ParkedPhysicalAddress;      // +24
  ULONGLONG ControllerPhysicalAddress;  // +32

  //
  // The following fields are optional.
  //

  ULONGLONG Gicv;                       // +40
  ULONGLONG Gich;                       // +48
  ULONG VgicMaintenanceInterrupt;       // +56
  ULONGLONG GicrBaseAddress;            // +60
  ULONGLONG Mpidr;                      // +68
  UCHAR ProcessorPowerEfficiencyClass;  // +76
  UCHAR Reserved2[3];                   // +77
} PROCLOCALGIC;

typedef PROCLOCALGIC UNALIGNED *PPROCLOCALGIC;

#define PROCESSOR_LOCAL_GIC_LENGTH \
  (RTL_SIZEOF_THROUGH_FIELD(PROCLOCALGIC, ControllerPhysicalAddress))

#define PROCESSOR_LOCAL_GIC_LENGTH_THROUGH_MPIDR \
  (RTL_SIZEOF_THROUGH_FIELD(PROCLOCALGIC, Mpidr))

#define PROCESSOR_LOCAL_GIC_LENGTH_THROUGH_EFFICIENCY_CLASS \
  (RTL_SIZEOF_THROUGH_FIELD(PROCLOCALGIC, ProcessorPowerEfficiencyClass))

#define PROCESSOR_LOCAL_GIC_LENGTH_THROUGH_RESERVED2 \
  (RTL_SIZEOF_THROUGH_FIELD(PROCLOCALGIC, Reserved2))

//
// Processor Local GIC flags.
//

#define PLGF_ENABLED_BIT 0
#define PLGF_ENABLED (1 << PLGF_ENABLED_BIT)

#define PLGF_PERF_INTERRUPT_EDGE_TRIGGERED_BIT 1
#define PLGF_PERF_INTERRUPT_EDGE_TRIGGERED \
  (1 << PLGF_PERF_INTERRUPT_EDGE_TRIGGERED_BIT)

typedef struct _GIC_DISTRIBUTOR {
  UCHAR Type;
  UCHAR Length;
  USHORT Reserved1;
  ULONG Identifier;
  ULONGLONG ControllerPhysicalAddress;
  ULONG GsivBase;
  UCHAR Version;
  UCHAR Reserved[3];
} GIC_DISTRIBUTOR;

typedef GIC_DISTRIBUTOR UNALIGNED *PGIC_DISTRIBUTOR;

#define GIC_DISTRIBUTOR_LENGTH (sizeof(GIC_DISTRIBUTOR))

typedef struct _GIC_MSIFRAME_ENTRY {
  UCHAR Type;
  UCHAR Length;
  USHORT Reserved1;
  ULONG Identifier;
  ULONGLONG PhysicalAddress;
  ULONG Flags;
  USHORT SpiCount;
  USHORT SpiBase;
} GIC_MSIFRAME_ENTRY;

typedef GIC_MSIFRAME_ENTRY UNALIGNED *PGIC_MSIFRAME_ENTRY;
#define GIC_MSIFRAME_ENTRY_LENGTH (sizeof(GIC_MSIFRAME_ENTRY))
#define GIC_MSIFRAME_SPI_BASE_COUNT_VALID 1

typedef struct _GIC_REDISTRIBUTOR {
  UCHAR Type;
  UCHAR Length;
  USHORT Reserved;
  ULONGLONG DiscoveryRangeBaseAddress;
  ULONG DiscoveryRangeLength;
} GIC_REDISTRIBUTOR;

typedef GIC_REDISTRIBUTOR UNALIGNED *PGIC_REDISTRIBUTOR;

#define GIC_REDISTRIBUTOR_LENGTH (sizeof(GIC_REDISTRIBUTOR))

typedef struct _GIC_ITS {
  UCHAR Type;
  UCHAR Length;
  USHORT Reserved1;
  ULONG Identifier;
  ULONGLONG PhysicalAddress;
  ULONG Reserved2;
} GIC_ITS;

typedef GIC_ITS UNALIGNED *PGIC_ITS;

#define GIC_ITS_LENGTH (sizeof(GIC_ITS))

//
// Smart Battery
//

typedef struct _SMARTBATTTABLE {
  DESCRIPTION_HEADER Header;
  ULONG WarningEnergyLevel;  // mWh at which the OEM suggests we warn the user
                             // that the battery is getting low.
  ULONG LowEnergyLevel;      // mWh at which the OEM suggests we put the machine
                             // into a sleep state.
  ULONG CriticalEnergyLevel;  // mWH at which the OEM suggests we do an
                              // emergency shutdown.

} SMARTBATTTABLE;

typedef SMARTBATTTABLE *PSMARTBATTTABLE;

//
// Embedded controller
//

#define ECDT_SIGNATURE 0x54444345  // "ECDT"

typedef struct {
  DESCRIPTION_HEADER Header;
  GEN_ADDR Control;
  GEN_ADDR Data;
  ULONG Uid;
  UCHAR GpeBit;
  CHAR Id[ANYSIZE_ARRAY];
} ECDT, *PECDT;

typedef struct _RSDT_32 {  // Root System Description Table

  DESCRIPTION_HEADER Header;
  ULONG Tables[ANYSIZE_ARRAY];  // The structure contains an n length array of
                                // physical addresses each of which point to
                                // another table.
} RSDT_32;

typedef struct _RSDT_64 {  // Root System Description Table
  DESCRIPTION_HEADER Header;
  ULONG
  Reserved;  // 4 bytes reserved as per 64 bit extensions to ACPI spec v0.7
  ULONG_PTR Tables[ANYSIZE_ARRAY];  // The structure contains an n length array
                                    // of physical addresses each of which point
                                    // to another table.
} RSDT_64;

#ifdef _IA64_  // XXTF
typedef RSDT_64 RSDT;
#else
typedef RSDT_32 RSDT;
#endif  // _IA64_ XXTF

typedef RSDT *PRSDT;

typedef struct _XSDT {
  DESCRIPTION_HEADER Header;
  UNALIGNED PHYSICAL_ADDRESS Tables[ANYSIZE_ARRAY];
} XSDT, *PXSDT;

// The below macro uses the min macro to protect against the case where we are
// running on machine which is compliant with a spec prior to .99.  If you had a
// .92 compliant header and one table pointer we would end of subtracting 32-36
// resulting in a really big number and hence we would think we had lots and
// lots of tables...  Using the min macro we end up subtracting the
// length-length getting zero which will be harmless and cause us to fail to
// load (with a red screen on Win9x) which is the best we can do in this case.

#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif

#define NumTableEntriesFromRSDTPointer(p)                                  \
  (p->Header.Length - min(p->Header.Length, sizeof(DESCRIPTION_HEADER))) / \
      sizeof(ULONG)
#define NumTableEntriesFromXSDTPointer(p)                                  \
  (p->Header.Length - min(p->Header.Length, sizeof(DESCRIPTION_HEADER))) / \
      sizeof(PHYSICAL_ADDRESS)

#define RSDT_SIGNATURE 0x54445352  // "RSDT"
#define XSDT_SIGNATURE 0x54445358  // "XSDT"
#define APIC_SIGNATURE 0x43495041  // "APIC"
#define SPIC_SIGNATURE \
  0x43495053  // "SPIC"  = SAPIC (IA64 extensions to ACPI requirement)
#define DSDT_SIGNATURE 0x54445344  // "DSDT"
#define SSDT_SIGNATURE 0x54445353  // "SSDT"
#define PSDT_SIGNATURE 0x54445350  // "PSDT"
#define SBST_SIGNATURE 0x54534253  // "SBST"
#define DBG2_SIGNATURE 0x32474244  // "DBG2"
#define DBGP_SIGNATURE 0x50474244  // "DBGP"
#define WDTT_SIGNATURE 0x54524457  // "WDRT"
#define SPCR_SIGNATURE 0x52435053  // "SPCR"
#define WDAT_SIGNATURE 0x54414457  // "WDAT"
#define RASF_SIGNATURE 0x46534152  // "RASF"
#define MADT_SIGNATURE 0x5444414D  // "MADT"

typedef struct _DSDT {  // Differentiated System Description Table

  DESCRIPTION_HEADER Header;
  UCHAR DiffDefBlock[ANYSIZE_ARRAY];  // this is the AML describing the base
                                      // system.

} DSDT;

typedef DSDT *PDSDT;

//
// Debug Port Table
//

typedef struct _DEBUG_PORT_TABLE {
  DESCRIPTION_HEADER Header;
  UCHAR InterfaceType;  // Type of registry interface (0 = full 16550 interface)
  UCHAR Reserved0[3];   // should be 0
  GEN_ADDR BaseAddress;  // Base address of the Debug Port register set
                         // described using the Generic Register Address
                         // Structure.
                         // 0   - console redirection disabled.
                         // e.g. COM1 (0x3F8) would be 0x1800000003F8
                         //      COM2 (Ox2F8) would be 0x1800000002F8

  //
  // Below fields only valid for Revision 2 and greater.
  //

  ULONG OffsetDbgDevInfo;
  ULONG NumberDbgDevInfo;
} DEBUG_PORT_TABLE, *PDEBUG_PORT_TABLE;

//
// Debug Device Information Structure
//

typedef struct _DEBUG_DEVICE_INFORMATION {
  UCHAR Revision;
  UCHAR Length;
  USHORT InterfaceType;
  ULONG InterfaceSubType;
  GEN_ADDR DeviceAddress;
  ULONG64 AddressSize;
} DEBUG_DEVICE_INFORMATION, *PDEBUG_DEVICE_INFORMATION;

//
// ACPI debug device port types.  The bottom 15 bits of these values should
// match the BCDE_DEBUGGER_TYPE values that are defined in the header
// minkernel\published\base\bcdtypes.w
//

#define DEBUG_DEVICE_PORT_SERIAL 0x8000
#define DEBUG_DEVICE_PORT_1394 0x8001
#define DEBUG_DEVICE_PORT_USB 0x8002
#define DEBUG_DEVICE_PORT_NET 0x8003
#define DEBUG_DEVICE_PORT_LOCAL 0x8004

#define DEBUG_DEVICE_SERIAL_LEGACY_16550 0x0
#define DEBUG_DEVICE_SERIAL_GEN_16550 0x1
#define DEBUG_DEVICE_SERIAL_SPI_MAX311XE 0x2
#define DEBUG_DEVICE_SERIAL_PL011 0x3
#define DEBUG_DEVICE_SERIAL_Q8X60 0x4
#define DEBUG_DEVICE_SERIAL_NVIDIA 0x5
#define DEBUG_DEVICE_SERIAL_OMAP 0x6
#define DEBUG_DEVICE_SERIAL_UEFI_DBGPORT 0x7
#define DEBUG_DEVICE_SERIAL_APM88XXXX 0x8
#define DEBUG_DEVICE_SERIAL_Q8X74 0x9
#define DEBUG_DEVICE_SERIAL_SAM5250 0xA
#define DEBUG_DEVICE_SERIAL_USIF 0xB
#define DEBUG_DEVICE_SERIAL_IMX6 0xC
#define DEBUG_DEVICE_SERIAL_SBSA32 0xD
#define DEBUG_DEVICE_SERIAL_SBSA 0xE
#define DEBUG_DEVICE_SERIAL_ARM_DCC 0xF
#define DEBUG_DEVICE_SERIAL_BCM2835 0x10
#define DEBUG_DEVICE_SERIAL_SDM845 0x11
#define DEBUG_DEVICE_SERIAL_MM_16550 0x12

//
// v2 Debug Device Information Structure.
//

typedef struct _DEBUG_DEVICE_INFORMATION_V2 {
  UCHAR Revision;
  USHORT Length;
  UCHAR BaseAddressRegisterCount;
  USHORT NameSpaceStringLength;
  USHORT NameSpaceStringOffset;
  USHORT OemDataLength;
  USHORT OemDataOffset;
  USHORT PortType;
  USHORT PortSubtype;
  USHORT Reserved;
  USHORT BaseAddressRegisterOffset;
  USHORT AddressSizeOffset;
  // GEN_ADDR BaseAddressRegister[BaseAddressRegisterCount];
  // ULONG AddressSize[BaseAddressRegisterCount];
  // UCHAR NameSpaceString[NameSpaceStringLength];
  // UCHAR OemData[OemDataLength];
} DEBUG_DEVICE_INFORMATION_V2, *PDEBUG_DEVICE_INFORMATION_V2;

typedef struct _DEBUG_PORT_TABLE_V2 {
  DESCRIPTION_HEADER Header;
  ULONG OffsetDbgDevInfo;
  ULONG NumberDbgDevInfo;
} DEBUG_PORT_TABLE_V2, *PDEBUG_PORT_TABLE_V2;

typedef struct _SERIAL_PORT_REDIRECTION_TABLE {
  DESCRIPTION_HEADER Header;
  UCHAR InterfaceType;  // Type of registry interface (0 = full 16550 interface)
  UCHAR Reserved0[3];   // should be 0
  GEN_ADDR BaseAddress;  // Base address of the Debug Port register set
                         // described using the Generic Register Address
                         // Structure.
                         // 0   - console redirection disabled.
                         // e.g. COM1 (0x3F8) would be 0x1800000003F8
                         //      COM2 (Ox2F8) would be 0x1800000002F8

  UCHAR InterruptType;  // Interrupt type(s) used by the UART.
                        // bit 0 = PC-AT-compatible 8259 IRQ interrupt.
                        // bit 1 = I/O APIC interrupt (Global System INterrupt)
                        // bit 2 = I/O SAPIC interrupt (Global System Interrupt)
                        // (IRQ) bit 3:7 = reserved (and must be 0) Note: bit ==
                        // 1 indicates support, bit == 0 indicates no support.
                        //
                        // Platforms with both a dual 8259 and an I/O APIC or
                        // I/O SAPIC must set the IRQ bit (bit 0) and the
                        // corresponding Global system interrupt bit.  E.g. a
                        // system that supported 8259 and SAPIC would be 0x5.

  UCHAR Irq;  // 0  = none
              // 2  = 2
              // 3  = 3
              // ...
              // 16 = 16
              // 1, 17-255 reserved

  ULONG GlobalSystemInterruptVector;
  // The I/O APIC or I/O SAPIC Global System Interrupt used
  // by the UART.Valid only if Bit[1] or Bit[2] of the
  // Interrupt Type field is set.

  UCHAR BaudRate;  // Baudrate for BIOS redirection
                   // 3 = 9600
                   // 4 = 19200
                   // 6 = 57600
                   // 7 = 115200
                   // 0-2,5, 8-255 reserved

  UCHAR Parity;  // 0 = no parity
                 // 1-255 reserved

  UCHAR StopBits;  // 1 = 1 stop bit
                   // 0, 2-255 = reserved

  UCHAR FlowControl;  // 0 = Hardware Flow Control
                      // 1 - 255 = reserved.

  UCHAR TerminalType;  // The terminal protocol the BIOS was using for
                       // console redirection
                       // 0 = VT100
                       // 1 = Extended VT100
                       // 2-255 = reserved

  UCHAR Language;  // Language which the BIOS was redirecting
                   // 0 = US Western English (standard ASCII)

  USHORT PciDeviceId;  // Designates device ID of a PCI device that
                       // contains a UART to be used as a headless
                       // port.

  USHORT PciVendorId;  // Designates vendor ID of a PCI device that
                       // contains a UART to be used as a headless
                       // port.

  UCHAR PciBusNumber;  // Designates which PCI system bus the PCI device
                       // resides on.

  UCHAR PciSlotNumber;  // Designates which PCI slot the PCI device
                        // resides in.

  UCHAR PciFunctionNumber;  // Which PCI function number describes the UART.

  ULONG PciFlags;  // PCI compatibility flags bitmask.  Should be zero
                   // by default.
                   // 0x1 indicates operating system should NOT suppress
                   // PnP device enumeration or disable power management
                   // for this device.
                   // bits 1-31 reserved.

  UCHAR PciSegment;  // PCI segment number.  For systems w/ < 255 PCI
                     // buses, this number must be 0.

  UCHAR Reserved1[4];  // should be 0

} SERIAL_PORT_REDIRECTION_TABLE, *PSERIAL_PORT_REDIRECTION_TABLE;

//
// Watchdog resource table.
//

typedef struct _WATCHDOG_TIMER_RESOURCE_TABLE {
  DESCRIPTION_HEADER Header;

  //
  // The address of the Watchdog Control register described using the Generic
  // Address Structure as defined in section 5.2.3.1 of the ACPI 2.0
  // Specification.
  //
  // N.B. Only System Memory address spaces are allowed (Address_Space_ID= 0).
  //

  GEN_ADDR ControlRegisterAddress;

  //
  // The base address of the Watchdog Count register described using the
  // Generic Address Structure as defined in section 5.2.3.1 of the ACPI 2.0
  // Specification.
  //
  // N.B. Only System Memory address spaces are allowed (Address_Space_ID= 0).
  //

  GEN_ADDR CountRegisterAddress;

  //
  // PCI Device ID if table describes a PCI device. Must be 0xFFFF if it is
  // not a PCI device.
  //

  USHORT PciDeviceId;

  //
  // PCI Vendor ID if table describes a PCI device. Must be 0xFFFF if it is
  // not a PCI device.
  //

  USHORT PciVendorId;

  //
  // PCI Bus Number if table describes a PCI device. Must be 0x00 if it is not
  // a PCI device.
  //

  UCHAR PciBusNumber;

  //
  // PCI Slot Number if table describes a PCI device. Must be 0x00 if it is
  // not a PCI device.
  //

  UCHAR PciSlotNumber;

  //
  // PCI Function Number if table describes a PCI device. Must be 0x00 if it
  // is not a PCI device.
  //

  UCHAR PciFunctionNumber;

  //
  // PCI segment number. For systems with fewer than 255 PCI buses, this
  // number must be 0.
  //

  UCHAR PciSegment;

  //
  // Contains the maximum counter value that this watchdog implementation
  // supports. Note: Max Count must be >=511 and <= 65,535.
  //

  USHORT MaxCount;

  //
  // Contains the units of the Max Count and Watchdog Count register.
  // 0x0=1 seconds/count
  // 0x1=100 milliseconds/count
  // 0x2=10 milliseconds/count
  //

  UCHAR Units;

} WATCHDOG_TIMER_RESOURCE_TABLE, *PWATCHDOG_TIMER_RESOURCE_TABLE;

//
// Watchdog timer instruction entry.
//

typedef struct _WATCHDOG_TIMER_INSTRUCTION_ENTRY {
  //
  // The watchdog action that this watchdog instruction is a part of.
  //

  UCHAR WatchdogAction;

  //
  // The watchdog instruction flags indicating the operation to be performed.
  //

  UCHAR InstructionFlags;

  //
  // Reserved, must be 0.
  //

  UCHAR Reserved[2];

  //
  // Generic Address Structure as defined in section 5.2.3.1 of the ACPI
  // Specification to describe the address and bit offset.
  //

  GEN_ADDR RegisterRegion;

  //
  // The value corresponding to this watchdog instruction.
  //

  ULONG Value;

  //
  // The bit mask required to obtain the bit(s) corresponding to the watchdog
  // instruction in the given bit range defined by Register Region.
  //

  ULONG Mask;

} WATCHDOG_TIMER_INSTRUCTION_ENTRY, *PWATCHDOG_TIMER_INSTRUCTION_ENTRY;

//
// Watchdog resource table, Revision 2.
//

typedef struct _WATCHDOG_TIMER_ACTION_TABLE {
  //
  // The ACPI table header.
  //

  DESCRIPTION_HEADER Header;

  //
  // The length in bytes of the watchdog header.  This includes the length
  // of the whole table excluding the instruction entries.
  //

  ULONG WatchdogHeaderLength;

  //
  // The PCI segment number.  If this table doesn't describe a PCI device,
  // then this field must be 0xFFFF.
  //

  USHORT PciSegment;

  //
  // The PCI bus number.  If this table doesn't describe a PCI device, then
  // this field must be 0xFF.
  //

  UCHAR PciBusNumber;

  //
  // The PCI bus number.  If this table doesn't describe a PCI device, then
  // this field must be 0xFF.
  //

  UCHAR PciDeviceNumber;

  //
  // The PCI function number.  If this table doesn't describe a PCI device,
  // then field must be 0xFF.
  //

  UCHAR PciFunctionNumber;

  //
  // Reserved, must be zero.
  //

  UCHAR Reserved1[3];

  //
  // The period of one timer tick in milliseconds.
  //

  ULONG TimerPeriod;

  //
  // The maximum watchdog timer value that this implementation supports (in
  // timer ticks).
  //

  ULONG MaximumCount;

  //
  // The minimum watchdog timer value that this implementation supports (in
  // timer ticks).
  //

  ULONG MinimumCount;

  //
  // Some flags indicating the initial state of the watchdog timer hardware.
  // Each flag that is true for the watchdog hardware should be set in this
  // field.  All other bits should be zero.
  //

  UCHAR WatchdogFlags;

  //
  // Reserved, must be 0.
  //

  UCHAR Reserved2[3];

  //
  // The number of watchdog instruction entries in the watchdog action table.
  //

  ULONG InstructionCount;

  //
  // A series of Watchdog Instruction Entries.
  //

  WATCHDOG_TIMER_INSTRUCTION_ENTRY InstructionEntry[ANYSIZE_ARRAY];

} WATCHDOG_TIMER_ACTION_TABLE, *PWATCHDOG_TIMER_ACTION_TABLE;

//
// Watchdog flags.
//

#define WATCHDOG_ENABLED 0x1
#define WATCHDOG_STOPPED_IN_SLEEP_STATE 0x80

//
// Watchdog actions.
//

#define WATCHDOG_ACTION_RELOAD 0x1
#define WATCHDOG_ACTION_QUERY_REMAINING_COUNTDOWN_PERIOD 0x4
#define WATCHDOG_ACTION_QUERY_COUNTDOWN_PERIOD 0x5
#define WATCHDOG_ACTION_SET_COUNTDOWN_PERIOD 0x6
#define WATCHDOG_ACTION_QUERY_RUNNING_STATE 0x8
#define WATCHDOG_ACTION_SET_RUNNING_STATE 0x9
#define WATCHDOG_ACTION_QUERY_STOPPED_STATE 0xA
#define WATCHDOG_ACTION_SET_STOPPED_STATE 0xB
#define WATCHDOG_ACTION_QUERY_REBOOT 0x10
#define WATCHDOG_ACTION_SET_REBOOT 0x11
#define WATCHDOG_ACTION_QUERY_SHUTDOWN 0x12
#define WATCHDOG_ACTION_SET_SHUTDOWN 0x13
#define WATCHDOG_ACTION_QUERY_BOOT_STATUS 0x20
#define WATCHDOG_ACTION_SET_BOOT_STATUS 0x21

//
// Watchdog instruction flags.
//

#define WATCHDOG_INSTRUCTION_READ_VALUE 0x0
#define WATCHDOG_INSTRUCTION_READ_COUNTDOWN 0x1
#define WATCHDOG_INSTRUCTION_WRITE_VALUE 0x2
#define WATCHDOG_INSTRUCTION_WRITE_COUNTDOWN 0x3
#define WATCHDOG_INSTRUCTION_PRESERVE_REGISTER 0x80

//
// BOOT Table -- based on Simple Boot Flag Specification 1.0
//

typedef struct _BOOT_TABLE {
  DESCRIPTION_HEADER Header;
  UCHAR CMOSIndex;
  UCHAR Reserved[3];
} BOOT_TABLE, *PBOOT_TABLE;

#define BOOT_SIGNATURE 0x544f4f42  // 'BOOT'

//
// Bits in the Boot Register
//

//
// Set by OS to indicate that the bios need only configure boot devices
//

#define SBF_PNPOS_BIT 0
#define SBF_PNPOS (1 << SBF_PNPOS_BIT)

//
// Set by BIOS to indicate beginning of boot, cleared by OS to indicate a
// successful boot
//

#define SBF_BOOTING_BIT 1
#define SBF_BOOTING (1 << SBF_BOOTING_BIT)

//
// Set by BIOS to indicate a diagnostic boot
//

#define SBF_DIAG_BIT 2
#define SBF_DIAG (1 << SBF_DIAG_BIT)

//
// Set to ensure ODD parity
//

#define SBF_PARITY_BIT 7
#define SBF_PARITY (1 << SBF_PARITY_BIT)

//
// IPPT Table --  IA64 Platform Properties Table
//

typedef struct _IPPT_TABLE {
  DESCRIPTION_HEADER Header;
  ULONG Flags;
  ULONG Reserved[3];
} IPPT_TABLE, *PIPPT_TABLE;

#define IPPT_DISABLE_WRITE_COMBINING 0x01L
#define IPPT_ENABLE_CROSS_PARTITION_IPI 0x02L
#define IPPT_DISABLE_PTCG_TB_FLUSH 0x04L
#define IPPT_DISABLE_UC_MAIN_MEMORY 0x08L
#define IPPT_ENABLE_PMTIMER_CLOCK 0x10L

#define IPPT_SIGNATURE 0x54505049  // 'IPPT'

//
// MCFG Table - PCI Memory-Mapped Configuration Space
//

typedef struct _MCFG_TABLE_ENTRY {
  ULONGLONG BaseAddress;
  USHORT SegmentNumber;
  UCHAR StartBusNumber;
  UCHAR EndBusNumber;
  ULONG Reserved;
} MCFG_TABLE_ENTRY, *PMCFG_TABLE_ENTRY;

typedef struct _MCFG_TABLE {
  DESCRIPTION_HEADER Header;
  ULONG Reserved[2];
  MCFG_TABLE_ENTRY TableEntry[ANYSIZE_ARRAY];
} MCFG_TABLE, *PMCFG_TABLE;

#define MCFG_TABLE_SIGNATURE 0x4746434d  // 'MCFG'

//
// High Precision Event Timer Description Table
//

typedef struct _HPET_DESCRIPTION_TABLE {
  DESCRIPTION_HEADER Header;

  //
  // Hardware ID of Event Timer Block:
  // Contents of General_Cap & ID Reg of Timer Block
  // [31:16] = PCI Vendor ID of 1st Timer Block
  // [5] = Legacy IRQ Routing Capable
  // [14] = Reserved
  // [12:8] = Number of Comparitors in 1st Timer Block
  // [7:0] = Hardware Rev ID
  //

  ULONG EventTimerBlockId;

  //
  // Low 32 bits of the base address of the timer block.
  //
  // Each Event Timer Block consumes 1K of system memory,
  // regardless of how many comparators are actually implemented
  // by hardware.
  //

  GEN_ADDR Address;
  UCHAR HpetNumber;
  USHORT MinimumPeriodicTickCount;
  UCHAR PageProtection;

} HPET_DESCRIPTION_TABLE, *PHPET_DESCRIPTION_TABLE;

#define HPET_SIGNATURE 0x54455048  // "HPET"

//
// Definitions for the Moorestown SFI Timer.
//

typedef struct _SFI_TIMER {
  GEN_ADDR Address;
  ULONG FemtosecondsPerTick;
  ULONG Gsi;
} SFI_TIMER, *PSFI_TIMER;

typedef struct _SFI_TIMER_DESCRIPTION_TABLE {
  DESCRIPTION_HEADER Header;
  SFI_TIMER Timers[ANYSIZE_ARRAY];
} SFI_TIMER_DESCRIPTION_TABLE, *PSFI_TIMER_DESCRIPTION_TABLE;

#define SFI_TIMER_SIGNATURE 0x524d544d  // "MTMR"

// TCG Hardware Interface Description Table Formats for Clients and Servers

typedef struct _TCG_DESCRIPTION_TABLE_CLIENT {
  DESCRIPTION_HEADER Header;
  USHORT PlatformClass;
  ULONG LAML;
  PHYSICAL_ADDRESS LASA;
} TCG_DESCRIPTION_TABLE_CLIENT, *PTCG_DESCRIPTION_TABLE_CLIENT;

typedef struct _TCG_DESCRIPTION_TABLE_SERVER {
  DESCRIPTION_HEADER Header;
  USHORT PlatformClass;
  USHORT Reserved1;
  ULONGLONG LAML;
  PHYSICAL_ADDRESS LASA;
  USHORT SpecificationRevision;
  UCHAR DeviceFlags;
  UCHAR InterruptFlags;
  UCHAR GPE;
  UCHAR Reserved2[3];
  ULONG GlobalSystemInterupt;
  GEN_ADDR BaseAddress;
  ULONG Reserved3;
  GEN_ADDR ConfigurationAddress;
  UCHAR PCISegmentGroupNumber;
  UCHAR PCIBusNumber;
  UCHAR PCIDeviceNumber;
  UCHAR PCIFunctionNumber;
} TCG_DESCRIPTION_TABLE_SERVER, *PTCG_DESCRIPTION_TABLE_SERVER;

typedef struct _TCG_DESCRIPTION_TABLE {
  union {
    TCG_DESCRIPTION_TABLE_CLIENT ClientTable;
    TCG_DESCRIPTION_TABLE_SERVER ServerTable;
  } u;
} TCG_DESCRIPTION_TABLE, *PTCG_DESCRIPTION_TABLE;

#define TCPA_SIGNATURE 0x41504354  //'TCPA'

//
// WAE Windows ACPI Enlightenment Table - used to describe characteristics
// of emulated devices in the system.
//

typedef struct _WAE_DESCRIPTION_TABLE {
  DESCRIPTION_HEADER Header;
  ULONG EmulatedDeviceFlags;
} WAE_DESCRIPTION_TABLE, *PWAE_DESCRIPTION_TABLE;

#define WAET_SIGNATURE 0x54454157  // 'WAET'

#define WAET_DEV_RTC_ENLIGHTENED_BIT 0
#define WAET_DEV_RTC_ENLIGHTENED (1 << WAET_DEV_RTC_ENLIGHTENED_BIT)

#define WAET_DEV_PMTMR_GOOD_BIT 1
#define WAET_DEV_PMTMR_GOOD (1 << WAET_DEV_PMTMR_GOOD_BIT)

#define WAET_REV_0_SIZE RTL_SIZEOF_THROUGH_FIELD(WAET, EmulatedDeviceFlags)

//
// A non-auto-generated copy of this appears in some assembly code.
//

C_ASSERT(WAET_DEV_RTC_ENLIGHTENED == 1);

//
// IORT table definitions.
//

#define IORT_SIGNATURE 0x54524f49  // "IORT"
#define IORT_MIN_SIZE 0x30
#define IORT_TABLE_REVISION 0

//
// Node types.
//

#define IORT_NODE_TYPE_ITS_GROUP 0
#define IORT_NODE_TYPE_NAMED_COMPONENT 1
#define IORT_NODE_TYPE_ROOT_COMPLEX 2
#define IORT_NODE_TYPE_SMMUV1V2 3
#define IORT_NODE_TYPE_SMMUV3 4
#define IORT_NODE_TYPE_PMCG 5

#define IORT_SMMUV2_NODE_REVISION 0
#define IORT_TYPE_GENERIC_SMMUV1 0
#define IORT_TYPE_GENERIC_SMMUV2 1
#define IORT_TYPE_ARM_CORELINK_MMU400 2
#define IORT_TYPE_ARM_CORELINK_MMU500 3

#define IORT_SMMUV3_NODE_REVISION 0
#define IORT_TYPE_GENERIC_SMMUV3 0

#define IORT_SMMUV3_NODE_REVISION 0
#define IORT_TYPE_GENERIC_SMMUV3 0

// TODO: Update to 1.
#define IORT_NAMED_COMPONENT_NODE_REVISION 0

#define IORT_ROOT_COMPLEX_NODE_REVISION 0

#if _MSC_VER >= 1200
#pragma warning(push)
#endif

#pragma warning(disable : 4214)  // nonstandard extension used : bit field types
                                 // other than int
#pragma warning( \
    disable : 4201)  // nonstandard extension used : nameless struct/union

//
// Top-level IORT table
//

typedef struct _IORT {
  DESCRIPTION_HEADER Header;
  ULONG NodeCount;
  ULONG NodeArrayOffset;
  UCHAR Reserved[4];

  //
  // Optional padding
  //

  //
  // Array of IORT nodes
  //

} IORT, *PIORT;

//
// Common shared header for all IORT nodes.
//

typedef struct _IORT_NODE_HEADER {
  UCHAR Type;
  USHORT Length;
  UCHAR Revision;
  UCHAR Reserved[4];
  ULONG IdMappingCount;
  ULONG IdMappingArrayOffset;
} IORT_NODE_HEADER, *PIORT_NODE_HEADER;

//
// IORT ID mapping.
//

typedef struct _IORT_IDMAPPING {
  ULONG InputBase;
  ULONG IdCount;
  ULONG OutputBase;
  ULONG OutputNodeOffset;

  union {
    ULONG AsULONG;
    struct {
      ULONG SingleMapping : 1;
      ULONG Reserved : 31;
    } DUMMYSTRUCTNAME;

  } Flags;

} IORT_IDMAPPING, *PIORT_IDMAPPING;

//
// SMMU v2 interrupt definition.
//

typedef union _IORT_SMMUV2_INTERRUPT {
  ULONGLONG AsULONGLONG;

  struct {
    ULONG Gsiv;

    union {
      ULONG AsULONG;

      struct {
        ULONG InterruptFlags : 1;
        ULONG Reserved : 31;
      } DUMMYSTRUCTNAME;

    } Flags;

  } DUMMYSTRUCTNAME;

} IORT_SMMUV2_INTERRUPT, *PIORT_SMMUV2_INTERRUPT;

typedef struct _IORT_SMMUV2_NODE {
  IORT_NODE_HEADER Header;
  ULONGLONG BaseAddress;
  ULONGLONG Span;
  ULONG Model;
  union {
    ULONG AsULONG;

    struct {
      ULONG DvmSupported : 1;
      ULONG CoherentPageTableWalk : 1;
      ULONG Reserved : 30;
    } DUMMYSTRUCTNAME;

  } Flags;

  ULONG GlobalInterruptArrayOffset;
  ULONG ContextInterruptCount;
  ULONG ContextInterruptArrayOffset;
  ULONG PmuInterruptCount;
  ULONG PmuInterruptArrayOffset;
  IORT_SMMUV2_INTERRUPT NSgIrpt;
  IORT_SMMUV2_INTERRUPT NSgCfgIrpt;

  //
  // Context interrupt array (IORT_SMMUV2_INTERRUPTs)
  //

  // IORT_SMMUV2_INTERRUPT ContextInterruptArray[ANYSIZE_ARRAY];

  //
  // PMU Interrupt array (IORT_SMMUV2_INTERRUPTs)
  //

  //
  // ID mapping array (IORT_SMMUV2_INTERRUPTs)
  //
} IORT_SMMUV2_NODE, *PIORT_SMMUV2_NODE;

typedef struct _IORT_SMMUV3_NODE {
  IORT_NODE_HEADER Header;
  ULONGLONG BaseAddress;

  union {
    ULONG AsULONG;

    struct {
      ULONG CohaccOverride : 1;
      ULONG HttuOverride : 2;
      ULONG Reserved : 29;
    } DUMMYSTRUCTNAME;

  } Flags;

  UCHAR Reserved[4];
  ULONGLONG VatosAddress;
  ULONG Model;
  ULONG EventGsiv;
  ULONG PriGsiv;
  ULONG GerrGsiv;
  ULONG SyncGsiv;

  //
  // ID mapping array
  //

} IORT_SMMUV3_NODE, *PIORT_SMMUV3_NODE;

//
// Memory access properties.
//

typedef union _IORT_NODE_MEMORY_ATTRIBUTES {
  ULONGLONG AsULONGLONG;

  struct {
    ULONG Cca;

    union {
      UCHAR AsUCHAR;

      struct {
        UCHAR Transient : 1;
        UCHAR WriteAllocate : 1;
        UCHAR ReadAllocate : 1;
        UCHAR Aho : 1;
        UCHAR Reserved : 4;
      } DUMMYSTRUCTNAME;

    } Ah;

    UCHAR Reserved[2];

    union {
      UCHAR AsUCHAR;

      struct {
        UCHAR Cpm : 1;
        UCHAR Dacs : 1;
        UCHAR Reserved : 6;
      } DUMMYSTRUCTNAME;

    } Maf;

  } DUMMYSTRUCTNAME;

} IORT_NODE_MEMORY_ATTRIBUTES, *PIORT_NODE_MEMORY_ATTRIBUTES;

typedef struct _IORT_ITS_GROUP_NODE {
  IORT_NODE_HEADER Header;
  ULONG ItsCount;
  ULONG ItsArray[ANYSIZE_ARRAY];
} IORT_ITS_GROUP_NODE, *PIORT_ITS_GROUP_NODE;

typedef struct _IORT_NAMED_COMPONENT_NODE {
  IORT_NODE_HEADER Header;
  UCHAR Reserved[4];  // Defined as node flags in the spec but reserved.
  IORT_NODE_MEMORY_ATTRIBUTES MemoryProperties;
  UCHAR MemoryAccessWidth;
  UCHAR DeviceName[ANYSIZE_ARRAY];

  //
  // Padding
  //

  //
  // Array of ID mappings
  //
} IORT_NAMED_COMPONENT_NODE, *PIORT_NAMED_COMPONENT_NODE;

typedef struct _IORT_ROOT_COMPLEX_NODE {
  IORT_NODE_HEADER Header;
  IORT_NODE_MEMORY_ATTRIBUTES MemoryProperties;
  ULONG AtsAttribute;
  ULONG PciSegmentNumber;

  //
  // Id mappings
  //

} IORT_ROOT_COMPLEX_NODE, *PIORT_ROOT_COMPLEX_NODE;

#if _MSC_VER >= 1200
#pragma warning(pop)
#endif

//
// DMA remapping reporting table.
//

#define DMAR_SIGNATURE 0x52414D44  // "DMAR"
#define DMAR_DRHD 0
#define DMAR_DRHD_MIN_SIZE 16
#define DMAR_RMRR 1
#define DMAR_ARIR 2
#define DMAR_ATSR 2
#define DMAR_RHSA 3
#define DMAR_ANDD 4
#define DMAR_FLAG_INT_REMAPPING 1
#define DMAR_FLAG_X2APIC_OPT_OUT 2
#define DMAR_FLAG_DMA_CTRL_PLATFORM_OPT_IN 4
#define ARIR_SIOAPIC 0x1
#define ARIR_PCI_ENUMERABLE 0x2
#define ATSR_FLAG_ALL_PORTS 1
#define DRHD_INCLUDE_ALL 1
#define DEVICE_SCOPE_PCI_DEVICE 1
#define DEVICE_SCOPE_PCI_SUB 2
#define DEVICE_SCOPE_IOAPIC 3
#define DEVICE_SCOPE_HPET 4
#define DEVICE_SCOPE_ACPI 5
#define DEVICE_SCOPE_MIN_SIZE 8

typedef struct _DEVICESCOPE {
  UCHAR Type;
  UCHAR Length;
  UCHAR Reserved[2];
  UCHAR EnumerationId;
  UCHAR StartBusNumber;

  struct {
    UCHAR Device;
    UCHAR Function;
  } PCIPath[ANYSIZE_ARRAY];
} DEVICESCOPE, *PDEVICESCOPE;

typedef struct _DRHD {
  UCHAR Flags;
  UCHAR Reserved;
  USHORT SegmentNumber;
  ULONGLONG BaseAddress;
  DEVICESCOPE DeviceScope[ANYSIZE_ARRAY];
} DRHD, *PDRHD;

typedef struct _RMRR {
  UCHAR Reserved[2];
  USHORT SegmentNumber;
  ULONGLONG RegionBaseAddress;
  ULONGLONG RegionLimitAddress;
  DEVICESCOPE DeviceScope[ANYSIZE_ARRAY];
} RMRR, *PRMRR;

typedef struct _ATSR {
  UCHAR Flags;
  UCHAR Reserved;
  USHORT SegmentNumber;
  DEVICESCOPE DeviceScope[ANYSIZE_ARRAY];
} ATSR, *PATSR;

typedef struct _RHSA {
  USHORT Type;
  USHORT Length;
  ULONG ReservedZ;
  ULONGLONG RegisterBaseAddress;
  ULONG ProximityDomain;
} RHSA, *PRHSA;

#if _MSC_VER >= 1200
#pragma warning(push)
#endif

#pragma warning(disable : 4214)  // nonstandard extension used : bit field types
                                 // other than int
#pragma warning( \
    disable : 4201)  // nonstandard extension used : nameless struct/union

typedef struct _DMARTABLE {
  USHORT Type;
  USHORT Length;
  union {
    DRHD Drhd;
    RMRR Rmrr;
    ATSR Atsr;
    RHSA Rhsa;
  } DUMMYUNIONNAME;
} DMARTABLE, *PDMARTABLE;

typedef struct _DMAR {
  DESCRIPTION_HEADER Header;
  UCHAR HostAddressWidth;
  UCHAR Flags;
  UCHAR Reserved[10];
  DMARTABLE DMARTables[ANYSIZE_ARRAY];
} DMAR, *PDMAR;

typedef union _DMAR_GCMD_REG {
  ULONG AsULONG;
  struct {
    ULONG Reserved : 23;
    ULONG CompatibilityFormatInterruptEnable : 1;  // bit 23   CFI
    ULONG SetInterruptRemappingTablePointer : 1;   // bit 24   SIRTP
    ULONG InterruptRemappingEnable : 1;            // bit 25   IRE
    ULONG QueuedInvalidationEnable : 1;            // bit 26   QIE
    ULONG WriteBufferFlush : 1;                    // bit 27   WBF
    ULONG EnableAdvancedFaultLogging : 1;          // bit 28   EAFL
    ULONG SetFaultLog : 1;                         // bit 29   SFL
    ULONG SetRootTablePointer : 1;                 // bit 30   SRTP
    ULONG TranslationEnable : 1;                   // bit 31   TE
  } DUMMYSTRUCTNAME;

} DMAR_GCMD_REG, *PDMAR_GCMD_REG;

typedef union _DMAR_GSTS_REG {
  ULONG AsULONG;
  struct {
    ULONG Reserved : 23;
    ULONG CompatibilityFormatInterruptStatus : 1;       // bit 23   CFIS
    ULONG SetInterruptRemappingTablePointerStatus : 1;  // bit 24   IRTPS
    ULONG InterruptRemappingEnableStatus : 1;           // bit 25   IRES
    ULONG QueuedInvalidationEnableStatus : 1;           // bit 26   QIES
    ULONG WriteBufferFlushStatus : 1;                   // bit 27   WBFS
    ULONG EnableAdvancedFaultLoggingStatus : 1;         // bit 28   AFLS
    ULONG SetFaultLogStatus : 1;                        // bit 29   FLS
    ULONG SetRootTablePointerStatus : 1;                // bit 30   RTPS
    ULONG TranslationEnableStatus : 1;                  // bit 31   TES
  } DUMMYSTRUCTNAME;

} DMAR_GSTS_REG, *PDMAR_GSTS_REG;

//
// AMD IOMMU related ACPI data structures.
//
#define IVRS_SIGNATURE 0x53525649  // "IVRS"

typedef union _IVRS_IVINFO {
  UINT32 AsUINT32;

  struct {
    UINT32 EFRSup : 1;
    UINT32 DmaGuardOptIn : 1;
    UINT32 ReservedZ0 : 3;
    UINT32 GVASize : 3;
    UINT32 PASize : 7;     // Physical address size
    UINT32 VASize : 7;     // Virtual address size
    UINT32 HtAtsResv : 1;  // ATS address translation range reserved
    UINT32 ReservedZ1 : 9;
  } DUMMYSTRUCTNAME;

} IVRS_IVINFO, *PIVRS_IVINFO;

typedef struct _IVRS {
  DESCRIPTION_HEADER Header;
  IVRS_IVINFO IVInfo;
  UINT64 Reserved;
  UINT8 DefinitionBlocks[1];

} IVRS, *PIVRS;

typedef enum _IVRS_BLOCK_TYPE {

  IommuDefinitionBlockTypeIvhd = 0x10,
  IommuDefinitionBlockType11Ivhd = 0x11,
  IommuDefinitionBlockTypeIvmdAll = 0x20,
  IommuDefinitionBlockTypeIvmdSpecified = 0x21,
  IommuDefinitionBlockTypeIvmdRange = 0x22,
  IommuDefinitionBlockType40Ivhd = 0x40,

} IVRS_BLOCK_TYPE;

typedef struct _IVRS_BLOCK_HEADER {
  UINT8 Type;
  UINT8 Flags;
  UINT16 Length;

} IVRS_BLOCK_HEADER, *PIVRS_BLOCK_HEADER;

typedef struct _IVHD_BLOCK {
  UINT8 Type;

  union {
    UINT8 AsUINT8;

    struct {
      UINT8 HtTunEn : 1;    // Overrides MMIO Offset 0018h[HtTunEn]
      UINT8 PassPW : 1;     // Overrides MMIO Offset 0018h[PassPw]
      UINT8 ResPassPW : 1;  // Overrides MMIO Offset 0018h[ResPassPW]
      UINT8 Isoc : 1;       // Overrides MMIO Offset 0018h[Isoc]
      UINT8 IotlbSup : 1;   // Overrides Capability Offset 00h[IotlbSup]
      UINT8 ReservedZ0 : 3;
    } DUMMYSTRUCTNAME;

  } Flags;

  UINT16 Length;
  UINT16 DeviceId;
  UINT16 CapabilityOffset;
  UINT64 IommuBaseAddress;
  UINT16 PciSegment;

  union {
    UINT16 AsUINT16;

    struct {
      UINT16 MsiNum : 5;
      UINT16 Reserved1 : 3;
      UINT16 UnitId : 5;
      UINT16 Reserved2 : 3;
    } DUMMYSTRUCTNAME;

  } IommuInfo;

  UINT32 ReservedZ0;

  union {
    UINT8 DeviceEntries[1];

    struct {
      UINT64 EfrRegisterImage;
      UINT64 ReservedZ1;
      UINT8 Type11DeviceEntries[1];
    } DUMMYSTRUCTNAME;

  } DUMMYUNIONNAME;

} IVHD_BLOCK, *PIVHD_BLOCK;

typedef enum _IVHD_ENTRY_TYPE {

  IvhdEntryTypePad4 = 0,
  IvhdEntryTypeAll = 1,
  IvhdEntryTypeSelect = 2,
  IvhdEntryTypeStartRange = 3,
  IvhdEntryTypeEndRange = 4,
  IvhdEntryTypePad8 = 64,
  IvhdEntryTypeAliasSelect = 66,
  IvhdEntryTypeAliasStartRange = 67,
  IvhdEntryTypeExtendedSelect = 70,
  IvhdEntryTypeExtendedStartRange = 71,
  IvhdEntryTypeSpecialDevice = 72,
  IvhdEntryTypeAcpiDevice = 0xF0

} IVRS_TABLE_TYPE;

typedef enum _APCI_IVHD_ENTRY_VARIETY {

  IvhdEntryVarietyIoapic = 1,
  IvhdEntryVarietyHpet = 2

} IVHD_ENTRY_VARIETY;

//
// IVHD Device Entry Type size is based on upper two bits of the type value.
//
//      Type    | Uppermost 2-bits | Device Entry Size
//      ---------------------------------------------------
//      0-63    | 00b              | 4
//      64-127  | 01b              | 8
//      128-191 | 10b              | 16
//      192-255 | 11b              | 32
//
#define IVHD_DEVICE_ENTRY_TYPE_TO_SIZE(_Type_) \
  ((ULONG_PTR)(1ULL << (((_Type_) >> 6) + 2)))

typedef struct _IVHD_DEVICE_ENTRY {
  UINT8 Type;
  UINT16 DeviceId;

  union {
    UINT8 AsUINT8;

    struct {
      UINT8 INITPass : 1;    // Device can assert INIT interrupt
      UINT8 ExtIntPass : 1;  // Device can assert ExtInt
      UINT8 NMIPass : 1;     // Device can assert NMI
      UINT8 ReservedZ0 : 1;
      UINT8 SysMgt : 2;     // Device can assert SMI
      UINT8 LINT0Pass : 1;  // Device can assert LINT0 interrupts
      UINT8 LINT1Pass : 1;  // Device can assert LINT1 interrupts
    } DUMMYSTRUCTNAME;

  } DataSetting;

  union {
    UINT32 AsUINT32;

    struct {
      UINT8 Handle;          // For Type 72
      UINT16 AliasDeviceId;  // For Type 66, 67, or 72
      UINT8 Variety;         // For Type 72
    };

    struct {
      UINT32 Reserved : 31;
      UINT32 AtsDisabled : 1;  // For Type 70 or 71
    };

  } ExtendedDataSetting;

} IVHD_DEVICE_ENTRY, *PIVHD_DEVICE_ENTRY;

typedef struct _IVHD_ACPI_DEVICE_ENTRY {
  UINT8 Type;
  UINT16 DeviceId;

  union {
    UINT8 AsUINT8;

    struct {
      UINT8 INITPass : 1;    // Device can assert INIT interrupt
      UINT8 ExtIntPass : 1;  // Device can assert ExtInt
      UINT8 NMIPass : 1;     // Device can assert NMI
      UINT8 ReservedZ0 : 1;
      UINT8 SysMgt : 2;     // Device can assert SMI
      UINT8 LINT0Pass : 1;  // Device can assert LINT0 interrupts
      UINT8 LINT1Pass : 1;  // Device can assert LINT1 interrupts
    } DUMMYSTRUCTNAME;

  } DataSetting;

  UINT64 HardwareId;
  UINT64 CompatibleId;
  UINT8 UniqueIdFormat;
  UINT8 UniqueIdLength;

  //
  // variable lengthed unique ID follows here.
  //

} IVHD_ACPI_DEVICE_ENTRY, *PIVHD_ACPI_DEVICE_ENTRY;

typedef struct _IVMD_BLOCK {
  UINT8 Type;  // 0x20, 0x21 and 0x22

  union {
    UINT8 AsUINT8;

    struct {
      UINT8 Unity : 1;  // 1b VA == PA
      UINT8 IR : 1;     // 1b Readable, 0b Not readable
      UINT8 IW : 1;     // 1b Writable, 0b Not writable
      UINT8 ExclusionRange : 1;
      UINT8 ReservedZ0 : 4;
    } DUMMYSTRUCTNAME;

  } Flags;

  UINT16 Length;

  union {
    UINT16 DeviceId;       // Type 0x21
    UINT16 StartDeviceId;  // Type 0x22
  } u1;

  union {
    UINT16 AuxiliaryData;  // Type 0x20, 0x21 Reserved Zero
    UINT16 EndDeviceId;    // Type 0x22
  } u2;

  UINT64 ReservedZ0;
  UINT64 StartAddress;       // Starting SPA of memory block
  UINT64 MemoryBlockLength;  // Memory block length

} IVMD_BLOCK, *PIVMD_BLOCK;

//
// Input Output Remapping Table
//

#define IORT_SIGNATURE 0x54524f49  // "IORT"

//
// Secure Device Table
//

#define SDEV_SIGNATURE 0x56454453  // "SDEV"
#define SDEV_SECURE_ACPI_TYPE 0
#define SDEV_SECURE_PCI_TYPE 1

#define SDEV_ENTRY_FLAG_OPTIONALLY_SECURE 1

typedef struct _SDEV_ENTRY_HEADER {
  UCHAR Type;
  UCHAR Flags;
  USHORT Length;
} SDEV_ENTRY_HEADER, *PSDEV_ENTRY_HEADER;

typedef struct _SDEV_SECURE_PCI_INFO_ENTRY {
  SDEV_ENTRY_HEADER Header;
  USHORT PciSegmentNumber;
  USHORT StartBusNumber;
  USHORT PciPathOffset;
  USHORT PciPathLength;
  USHORT VendorInfoOffset;
  USHORT VendorInfoLength;
} SDEV_SECURE_PCI_INFO_ENTRY, *PSDEV_SECURE_PCI_INFO_ENTRY;

typedef struct _SDEV_SECURE_ACPI_INFO_ENTRY {
  SDEV_ENTRY_HEADER Header;
  USHORT IdentifierOffset;
  USHORT IdentifierLength;
  USHORT VendorInfoOffset;
  USHORT VendorInfoLength;
} SDEV_SECURE_ACPI_INFO_ENTRY, *PSDEV_SECURE_ACPI_INFO_ENTRY;

typedef struct _SDEV {
  DESCRIPTION_HEADER Header;
  ULONG SDEVTables[ANYSIZE_ARRAY];
} SDEV, *PSDEV;

//
// Definitions pertaining to the Core System Resource Table (CSRT) The CSRT
// contains a CSRT table header followed by one or more Resource Groups.
// Each Resource Group contains a Resource Group header followed by one or
// more Resource Descriptors. Each Resource Descriptor contains a Resource
// Descriptor header followed by information describing the resource.
//
// The relationship of these fields is:
//
// CSRT Table Header
// Resource Group[0]
//   Resource Group Header
//   Resource Group Header Shared Info
//   Resource Descriptor[0]
//     Resource Descriptor Header
//     Resource Information (not defined here)
//   {Resource Descriptor[1..n]}
// {Resource Group[1..n]}
//

#define CSRT_SIGNATURE ((ULONG)('TRSC'))
#define CSRT_REVISION 0x00

//
// CSRT Table Header
//

typedef struct _CSRT_TABLE_HEADER {
  DESCRIPTION_HEADER Header;
} CSRT_TABLE_HEADER, *PCSRT_TABLE_HEADER;

//
// Resource Group Header
//

typedef struct _CSRT_RESOURCE_GROUP_HEADER {
  UINT32 Length;            // Length of Group, including size of header.
  UINT32 VendorId;          // 4-char ID
  UINT32 SubvendorId;       // 4-char ID, 0 => No Subvendor/subdevice Id
  UINT16 DeviceId;          // 16-bit Device Id
  UINT16 SubdeviceId;       // 16-bit Subdevice Id
  UINT16 Revision;          // 16-bit Revision
  UINT16 Instance;          // 16-bit Instance. Normally set to 0.
  UINT32 SharedInfoLength;  // Length of appended info, 0 => no shared info
} CSRT_RESOURCE_GROUP_HEADER, *PCSRT_RESOURCE_GROUP_HEADER;

//
// Resource Descriptor Types
//

#define CSRT_RD_TYPE_UNKNOWN 0
#define CSRT_RD_SUBTYPE_UNKNOWN 0

#define CSRT_RD_TYPE_ANY 0xFFFF
#define CSRT_RD_SUBTYPE_ANY 0xFFFF

#define CSRT_RD_TYPE_INTERRUPT 1
#define CSRT_RD_SUBTYPE_INTERRUPT_LINES 0
#define CSRT_RD_SUBTYPE_INTERRUPT_CONTROLLER 1

#define CSRT_RD_TYPE_TIMER 2
#define CSRT_RD_SUBTYPE_TIMER 0

#define CSRT_RD_TYPE_DMA 3
#define CSRT_RD_SUBTYPE_DMA_CHANNEL 0
#define CSRT_RD_SUBTYPE_DMA_CONTROLLER 1

#define CSRT_RD_TYPE_CACHE 4
#define CSRT_RD_SUBTYPE_CACHE 0

#define CSRT_RD_UID_ANY 0xFFFF

//
// Resource Descriptor Header
//

typedef struct _CSRT_RESOURCE_DESCRIPTOR_HEADER {
  UINT32 Length;   // Length of Descriptor, including size of header.
  UINT16 Type;     // Type, one of CSRT_RD_TYPE*
  UINT16 Subtype;  // Subtype, one of CSRT_RD_SUBTYPE*
  UINT32 Uid;      // Unique (within Resource Group) Identifier
} CSRT_RESOURCE_DESCRIPTOR_HEADER, *PCSRT_RESOURCE_DESCRIPTOR_HEADER;

//
// ACPI PCC table format. This table supplies the location and usage semantics
// for low overhead communications with a platform entity.
//

typedef struct _PCC_SUBSPACE_HEADER {
  UCHAR Type;
  UCHAR Length;
} PCC_SUBSPACE_HEADER, *PPCC_SUBSPACE_HEADER;

#define PCC_SUBSPACE_TYPE_GENERIC 0

typedef struct _PCC_GENERIC_SUBSPACE {
  PCC_SUBSPACE_HEADER Header;
  USHORT Reserved1;
  ULONG Reserved2;
  PHYSICAL_ADDRESS BaseAddress;
  ULONGLONG Length;
  GEN_ADDR DoorbellRegister;
  ULONGLONG DoorbellPreserve;
  ULONGLONG DoorbellWrite;
  ULONG NominalLatency;
  ULONG MaximumPeriodicAccessRate;
  USHORT MinimumRequestTurnaroundTime;
} PCC_GENERIC_SUBSPACE, *PPCC_GENERIC_SUBSPACE;

#define PCC_GENERIC_SHARED_REGION_SIGNATURE 0x50434300  // " CCP"

typedef struct _PCC_GENREIC_SHARED_REGION {
  ULONG Signature;
  union {
    struct {
      USHORT CommandCode : 8;  // 7:0
      USHORT ReservedZ : 7;    // 14:8
      USHORT SciDoorbell : 1;  // 15
    };

    USHORT AsUShort;
  } Command;

  union {
    struct {
      USHORT CommandComplete : 1;       // 0
      USHORT SciReceived : 1;           // 1
      USHORT Error : 1;                 // 2
      USHORT PlatformNotification : 1;  // 3
      USHORT Reserved : 12;             // 15:4
    };

    USHORT AsUShort;
  } Status;

  UCHAR CommunicationSpace[ANYSIZE_ARRAY];
} PCC_GENERIC_SHARED_REGION, *PPCC_GENERIC_SHARED_REGION;

#define PCCT_SIGNATURE 0x54434350  // "PCCT"

typedef struct _PCC_TABLE {
  DESCRIPTION_HEADER Header;
  union {
    ULONG AsULong;

    struct {
      ULONG SciSupported : 1;  // SCI notification is supported
      ULONG Reserved : 31;
    } DUMMYSTRUCTNAME;

  } Flags;
  ULONG64 Reserved;
  PCC_SUBSPACE_HEADER Subspaces;  // packed list of subspaces
} PCC_TABLE, *PPCC_TABLE;

//
// Boot Graphics Resource Table
//

#define BGRT_STATUS_IMAGE_VALID 0x01
#define BGRT_STATUS_DISPLAY_ROTATION 0x06

#define BGRT_STATUS_GET_DISPLAY_ROTATION(_Status_) \
  ((UCHAR)((ULONG)((_Status_)&BGRT_STATUS_DISPLAY_ROTATION) >> 1))

typedef enum _BGRT_IMAGE_TYPE {
  BgrtImageTypeBitmap,
  BgrtImageTypeMax
} BGRT_IMAGE_TYPE,
    *PBGRT_IMAGE_TYPE;

typedef struct _BGRT_TABLE {
  DESCRIPTION_HEADER Header;
  USHORT Version;
  UCHAR Status;
  UCHAR ImageType;
  ULONGLONG LogoAddress;
  ULONG OffsetX;
  ULONG OffsetY;
} BGRT_TABLE, *PBGRT_TABLE;

#define BGRT_SIGNATURE 0x54524742  // "BGRT"

//
// Boot Background Resource Table
//

typedef struct _BBRT_TABLE {
  DESCRIPTION_HEADER Header;
  ULONG Background;
  ULONG Foreground;
} BBRT_TABLE, *PBBRT_TABLE;

#define BBRT_SIGNATURE 0x54524242  // "BBRT"

//
// Generic Timer Description Table
//

#define GTDT_SIGNATURE 0x54445447  // "GTDT"

typedef struct _GTDT_TABLE {
  DESCRIPTION_HEADER Header;
  ULONGLONG CntControlBasePhysicalAddress;
  ULONG Reserved;
  ULONG SecurePhysicalTimerGsiv;
  ULONG SecurePhysicalTimerFlags;
  ULONG NonSecurePhysicalTimerGsiv;
  ULONG NonSecurePhysicalTimerFlags;
  ULONG VirtualTimerEventGsiv;
  ULONG VirtualTimerEventFlags;
  ULONG NonSecurePhysicalTimer2Gsiv;
  ULONG NonSecurePhysicalTimer2Flags;
  ULONGLONG CntReadBasePhysicalAddress;
  ULONG TimerBlockCount;
  ULONG TimerBlockOffset;
} GTDT_TABLE, *PGTDT_TABLE;

//
// GTDT Table timer flags.
//

#define GTDT_TIMER_EDGE_TRIGGERED 0x00000001
#define GTDT_TIMER_ACTIVE_LOW 0x00000002
#define GTDT_TIMER_ALWAYS_ON 0x00000004

//
// GTDT Platform Timer block types.
//

typedef enum _GTDT_PLATFORM_TIMER_BLOCK_TYPE {
  GtdtPlatformGtBlock = 0,
  GtdtPlatformWatchdogBlock = 1,
} GTDT_PLATFORM_TIMER_BLOCK_TYPE,
    *PGTDT_PLATFORM_TIMER_BLOCK_TYPE;

typedef struct _GTDT_PLATFORM_TIMER_HEADER {
  UCHAR Type;
  USHORT Length;
} GTDT_PLATFORM_TIMER_HEADER, *PGTDT_PLATFORM_TIMER_HEADER;

//
// GTDT Platform Timer GT block definitions.
//

typedef struct _GTDT_PLATFORM_GT_BLOCK {
  GTDT_PLATFORM_TIMER_HEADER Header;
  UCHAR Reserved;
  ULONGLONG CntCtlBasePhysicalAddress;
  ULONG BlockTimerCount;
  ULONG BlockTimerOffset;
} GTDT_PLATFORM_GT_BLOCK, *PGTDT_PLATFORM_GT_BLOCK;

#define GTDT_PLATFORM_GT_EDGE_TRIGGERED 0x00000001
#define GTDT_PLATFORM_GT_ACTIVE_LOW 0x00000002

#define GTDT_PLATFORM_GT_COMMON_SECURE 0x00000001
#define GTDT_PLATFORM_GT_COMMON_ALWAYS_ON 0x00000002

typedef struct _GTDT_PLATFORM_GT_INSTANCE {
  UCHAR FrameNumber;
  UCHAR Reserved[3];
  ULONGLONG CntBasePhysicalAddress;
  ULONGLONG CntEL0BasePhysicalAddress;
  ULONG PhysicalTimerGsiv;
  ULONG PhysicalTimerFlags;
  ULONG VirtualTimerGsiv;
  ULONG VirtualTimerFlags;
  ULONG CommonFlags;
} GTDT_PLATFORM_GT_INSTANCE, *PGTDT_PLATFORM_GT_INSTANCE;

//
// GTDT Platform Timer GT block definitions.
//

#define GTDT_WATCHDOG_EDGE_TRIGGERED 0x00000001
#define GTDT_WATCHDOG_ACTIVE_LOW 0x00000002
#define GTDT_WATCHDOG_SECURE 0x00000004

typedef struct _GTDT_PLATFORM_WATCHDOG_BLOCK {
  GTDT_PLATFORM_TIMER_HEADER Header;
  UCHAR Reserved;
  ULONGLONG RefreshFramePhysicalAddress;
  ULONGLONG ControlFramePhysicalAddress;
  ULONG WatchdogTimerGsiv;
  ULONG WatchdogTimerFlags;
} GTDT_PLATFORM_WATCHDOG_BLOCK, *PGTDT_PLATFORM_WATCHDOG_BLOCK;

#if _MSC_VER >= 1200
#pragma warning(pop)
#endif

//
// ACPI OEM0 table format. This table is created by Hyper-V to pass
// entropy from the host to the guest.
//

#define OEM0_RANDOM_BYTES 64

typedef struct _OEM0_TABLE {
  DESCRIPTION_HEADER Header;
  UCHAR Data[OEM0_RANDOM_BYTES];
} OEM0_TABLE, *POEM0_TABLE;

#define OEM0_SIGNATURE 0x304d454f  // "OEM0"

//
// System Locality Distance Information Table (SLIT).
//

#define SLIT_SIGNATURE 0x54494c53  // "SLIT"

typedef struct SLIT_TABLE {
  DESCRIPTION_HEADER Header;
  ULONGLONG LocalityCount;
  // UCHAR Entry[LocalityCount][LocalityCount]
} SLIT_TABLE, *PSLIT_TABLE;

#define SLIT_NORMALIZATION_VALUE 10

//
// The three tables below (s3pt, fbpt, and fpdt) are all part of work to
// enable the firmware to pass up boot/on performance metrics to the OS.
// The layout of these tables is as follows: the fpdt - firmware performance
// data table - contains records of type s3 table pointer and boot table
// pointer.  Both of these types of records contain the physical address of
// their respective table.  The FPDT table does not change except on reboot.
// The s3pt and fbpt are the tables pointed at by these physical addresses.
// The contents of memory at these physical locations can and will change
// across s3 and s4 transitions, and across boot.  The s3 performance table
// can contain multiple records, one for sleep performance and one for resume.
//

//
// S3 Performance Data Table
//

#define S3PT_SIGNATURE 0x54503353  // "S3PT"

typedef struct _FIRMWARE_S3_PERFORMANCE_TABLE {
  ULONG Signature;
  ULONG Length;
  // s3 performance records
} FIRMWARE_S3_PERFORMANCE_TABLE, *PFIRMWARE_S3_PERFORMANCE_TABLE;

typedef struct _S3PT_RECORD_HEADER {
  USHORT RecordSubtype;
  CHAR RecordLength;
  CHAR Revision;
} S3PT_RECORD_HEADER, *PS3PT_RECORD_HEADER;

typedef struct _S3PT_PERFORMANCE_RECORD {
  S3PT_RECORD_HEADER RecordHeader;
  union {
    struct {
      ULONG ResumeCount;
      ULONGLONG FullResume;
      ULONGLONG AverageResume;
    } ResumeRecord;
    struct {
      ULONGLONG SuspendStart;
      ULONGLONG SuspendEnd;
    } SuspendRecord;
  } u;
} S3PT_PERFORMANCE_RECORD, *PS3PT_PERFORMANCE_RECORD;

typedef enum _FPDT_RECORD_SUBTYPE {
  S3PTTypeResumePerformance = 0,
  S3PTTypeSuspendPerformance = 1,
  BasicBootPerformanceType = 2
} FPDT_RECORD_SUBTYPE,
    *PFPDT_RECORD_SUBTYPE;

//
// Firmware Basic Boot Performance Table
//

#define FBPT_SIGNATURE 0x54504246  // "FBPT"

typedef struct _FIRMWARE_BOOT_PERFORMANCE_TABLE {
  ULONG Signature;
  ULONG Length;
  // FBPT_RECORD Record[];
} FIRMWARE_BOOT_PERFORMANCE_TABLE, *PFIRMWARE_BOOT_PERFORMANCE_TABLE;

typedef struct _FBPT_RECORD_HEADER {
  USHORT RecordSubtype;
  CHAR RecordLength;
  CHAR Revision;
} FBPT_RECORD_HEADER, *PFBPT_RECORD_HEADER;

typedef struct _FBPT_RECORD {
  FBPT_RECORD_HEADER RecordHeader;
  ULONG Reserved;
  ULONGLONG ResetEnd;
  ULONGLONG LoadImageStart;
  ULONGLONG StartImageStart;
  ULONGLONG ExitBootServicesEntry;
  ULONGLONG ExitBootServicesExit;
} FBPT_RECORD, *PFBPT_RECORD;

//
// Firmware Performance Data Table
//

#define FPDT_SIGNATURE 0x54445046  // "FPDT"

typedef struct _FPDT_TABLE {
  DESCRIPTION_HEADER Header;
  // fpdt performance records
} FPDT_TABLE, *PFPDT_TABLE;

typedef enum _FPDT_RECORD_TYPE {
  FpdtTypeBootTablePointer = 0,
  FpdtTypeS3TablePointer = 1
} FPDT_RECORD_TYPE,
    *PFPDT_RECORD_TYPE;

typedef struct _FPDT_RECORD_HEADER {
  USHORT RecordType;
  CHAR RecordLength;
  CHAR Revision;
  ULONG Reserved;
} FPDT_RECORD_HEADER, *PFPDT_RECORD_HEADER;

typedef struct _FPDT_RECORD {
  FPDT_RECORD_HEADER RecordHeader;
  union {
    struct {
      PHYSICAL_ADDRESS PhysicalAddress;
    } S3TablePointer;
    struct {
      PHYSICAL_ADDRESS PhysicalAddress;
    } BasicBootPointer;
  } u;
} FPDT_RECORD, *PFPDT_RECORD;

//
//  Windows Platform Binary Table (WPBT)
//

#define WPBT_SIGNATURE 'TBPW'

typedef enum _ACPI_WPBT_CONTENT_LAYOUT {
  WpbtContentLayoutPE = 1
} ACPI_WPBT_CONTENT_LAYOUT;

typedef enum _ACPI_WPBT_CONTENT_TYPE {
  WpbtContentTypeNative = 1
} ACPI_WPBT_CONTENT_TYPE;

#define WPBT_MINIMUM_REVISION 1
#define WPBT_MAXIMUM_REVISION 1

typedef struct _ACPI_WPBT_TABLE {
  DESCRIPTION_HEADER Header;

  ULONG HandoffMemorySize;
  ULONG64 HandoffMemoryLocation;
  UCHAR ContentLayout;
  UCHAR ContentType;
  USHORT CommandLineArgumentsLength;
  WCHAR CommandLineArguments[ANYSIZE_ARRAY];
} ACPI_WPBT_TABLE, *PACPI_WPBT_TABLE;

#define WPBT_MIN_SIZE 52
#define WPBT_BOOT_SEARCH_START 0x1000
#define WPBT_BOOT_SEARCH_END 0xA0000
#define WPBT_BOOT_SEARCH_INCREMENT 0x40

//
// TPM 2.0 ACPI table.
//

#define TPM20_TABLE_SIGNATURE '2MPT'

typedef struct _TPM20_TABLE {
  DESCRIPTION_HEADER Header;

  union {
    struct {
      UINT32 UseMemoryDescriptors : 1;
      UINT32 CmdListCapable : 1;
      UINT32 NoDeviceIO : 1;
      UINT32 DeviceMemory : 1;
      UINT32 DevMemOnly : 1;
      UINT32 Reserved : 27;
    } Flags;

    UINT32 FlagBits;
  } u;

  UINT64 ControlAreaPA;
  UINT32 StartMethod;
  UINT32 PlatformParameters[8];
} TPM20_TABLE, *PTPM20_TABLE;

typedef enum _TPM20_START_METHOD {
  Tpm20TableStartMethodInvalid = 0,
  Tpm20TableStartMethodSim = 1,
  Tpm20TableStartMethodAcpi = 2,
  Tpm20TableStartMethodTz1 = 3,
  Tpm20TableStartMethodTz2 = 4,
  Tpm20TableStartMethodTz3 = 5,
  Tpm20TableStartMethodTis13 = 6,
  Tpm20TableStartMethodCR = 7,
  Tpm20TableStartMethodCRWithAcpi = 8
} TPM20_START_METHOD,
    *PTPM20_START_METHOD;

//
// ACPI Physical Location Descriptor, Revision 1
// (per ACPI 3.0 spec, section 6.1.6)
//

typedef struct _ACPI_PLD_BUFFER {
  UINT32 Revision : 7;
  UINT32 IgnoreColor : 1;
  UINT32 Color : 24;
  UINT32 Width : 16;
  UINT32 Height : 16;
  UINT32 UserVisible : 1;
  UINT32 Dock : 1;
  UINT32 Lid : 1;
  UINT32 Panel : 3;
  UINT32 VerticalPosition : 2;
  UINT32 HorizontalPosition : 2;
  UINT32 Shape : 4;
  UINT32 GroupOrientation : 1;
  UINT32 GroupToken : 8;
  UINT32 GroupPosition : 8;
  UINT32 Bay : 1;
  UINT32 Ejectable : 1;
  UINT32 EjectionRequired : 1;
  UINT32 CabinetNumber : 8;
  UINT32 CardCageNumber : 8;
  UINT32 Reserved : 14;
} ACPI_PLD_BUFFER, *PACPI_PLD_BUFFER;

//
// ACPI Physical Location Descriptor, Revision 2
// (per ACPI 4.0 spec, section 6.1.6; now ACPI 6.0 spec, section 6.1.8)
//

typedef struct _ACPI_PLD_V2_BUFFER {
  UINT32 Revision : 7;
  UINT32 IgnoreColor : 1;
  UINT32 Color : 24;
  UINT32 Width : 16;
  UINT32 Height : 16;
  UINT32 UserVisible : 1;
  UINT32 Dock : 1;
  UINT32 Lid : 1;
  UINT32 Panel : 3;
  UINT32 VerticalPosition : 2;
  UINT32 HorizontalPosition : 2;
  UINT32 Shape : 4;
  UINT32 GroupOrientation : 1;
  UINT32 GroupToken : 8;
  UINT32 GroupPosition : 8;
  UINT32 Bay : 1;
  UINT32 Ejectable : 1;
  UINT32 EjectionRequired : 1;
  UINT32 CabinetNumber : 8;
  UINT32 CardCageNumber : 8;
  UINT32 Reference : 1;
  UINT32 Rotation : 4;
  UINT32 Order : 5;
  UINT32 Reserved : 4;
  UINT32 VerticalOffset : 16;
  UINT32 HorizontalOffset : 16;
} ACPI_PLD_V2_BUFFER, *PACPI_PLD_V2_BUFFER;

// Color bits 8:31 (Red 8:15, Green 16:23, Blue 24:31)
#define ACPI_PLD_MAKE_COLOR(r, g, b) \
  ((UINT32)(((r)&0xFF) | (((g)&0xFF) << 8) | (((b)&0xFF) << 16)))
#define ACPI_PLD_COLOR_RED(c) ((BYTE)(((c) >> 0) & 0xFF))
#define ACPI_PLD_COLOR_GREEN(c) ((BYTE)(((c) >> 8) & 0xFF))
#define ACPI_PLD_COLOR_BLUE(c) ((BYTE)(((c) >> 16) & 0xFF))

// Panel surface bits 67:69
typedef enum _ACPI_PLD_PANEL {
  AcpiPldPanelTop = 0,
  AcpiPldPanelBottom = 1,
  AcpiPldPanelLeft = 2,
  AcpiPldPanelRight = 3,
  AcpiPldPanelFront = 4,
  AcpiPldPanelBack = 5,
  AcpiPldPanelUnknown = 6,
} ACPI_PLD_PANEL,
    *PACPI_PLD_PANEL;

typedef ACPI_PLD_PANEL AcpiPldPanel;

// Vertical position bits 70:71
typedef enum _ACPI_PLD_VERTICAL_POSITION {
  AcpiPldVPosUpper = 0,
  AcpiPldVPosCenter = 1,
  AcpiPldVPosLower = 2,
} ACPI_PLD_VERTICAL_POSITION,
    *PACPI_PLD_VERTICAL_POSITION;

typedef ACPI_PLD_VERTICAL_POSITION AcpiPldVPos;

// Horizontal position bits 72:73
typedef enum _ACPI_PLD_HORIZONTAL_POSITION {
  AcpiPldHPosLeft = 0,
  AcpiPldHPosCenter = 1,
  AcpiPldHPosRight = 2,
} ACPI_PLD_HORIZONTAL_POSITION,
    *PACPI_PLD_HORIZONTAL_POSITION;

typedef ACPI_PLD_HORIZONTAL_POSITION AcpiPldHPos;

// Shape bits 74:77
typedef enum _ACPI_PLD_SHAPE {
  AcpiPldShapeRound = 0,
  AcpiPldShapeOval = 1,
  AcpiPldShapeSquare = 2,
  AcpiPldShapeVRect = 3,
  AcpiPldShapeHRect = 4,
  AcpiPldShapeVTrap = 5,
  AcpiPldShapeHTrap = 6,
  AcpiPldShapeUnknown = 7,
  AcpiPldShapeChamfered = 8,
} ACPI_PLD_SHAPE,
    *PACPI_PLD_SHAPE;

typedef ACPI_PLD_SHAPE AcpiPldShape;

// Orientation bit 78
typedef enum _ACPI_PLD_ORIENTATION {
  AcpiPldOrientationHorizontal = 0,
  AcpiPldOrientationVertical = 1,
} ACPI_PLD_ORIENTATION,
    *PACPI_PLD_ORIENTATION;

// Rotation bits 115:118
typedef enum _ACPI_PLD_ROTATION {
  AcpiPldRotation0 = 0,
  AcpiPldRotation45 = 1,
  AcpiPldRotation90 = 2,
  AcpiPldRotation135 = 3,
  AcpiPldRotation180 = 4,
  AcpiPldRotation225 = 5,
  AcpiPldRotation270 = 6,
  AcpiPldRotation315 = 7,
} ACPI_PLD_ROTATION,
    *PACPI_PLD_ROTATION;

typedef ACPI_PLD_ROTATION AcpiPldRotation;

// Offset bits 128:160 (Vertical 128:143, Horizontal 144:159)
#define ACPI_PLD_OFFSET_NONE 0xFFFF

//
// ACPI PLD Joint Descriptor Buffer, Revision 1
// (MSFT custom PLD buffer extension, 128-bits)
//

typedef struct _ACPI_PLD_JOINT_BUFFER {
  UINT32 Revision : 5;
  UINT32 JointType : 4;
  UINT32 SourceEdge : 3;
  UINT32 TargetCabinetNumber : 8;
  UINT32 TargetPanel : 3;
  UINT32 TargetEdge : 3;
  UINT32 Reserved1 : 5;
  UINT32 MovementOrientation : 1;
  UINT32 ForwardMovementRange : 16;
  UINT32 BackwardMovementRange : 16;
  UINT32 HorizontalOffset : 16;
  UINT32 VerticalOffset : 16;
  UINT32 GapOffset : 16;
  UINT32 Rotation : 9;
  UINT32 Reserved2 : 7;
} ACPI_PLD_JOINT_BUFFER, *PACPI_PLD_JOINT_BUFFER;

DEFINE_GUID(ACPI_PLD_JOINT_BUFFER_GUID, 0xf01cfc40, 0x3c75, 0x4523, 0x9e, 0x44,
            0x21, 0x5c, 0xb1, 0x54, 0xbd, 0xa6);

// Joint type bits 6:9
typedef enum _ACPI_PLD_JOINT_TYPE {
  AcpiPldJointTypeFixed = 0,
  AcpiPldJointTypePlanar = 1,
  AcpiPldJointTypeHinge = 2,
  AcpiPldJointTypePivot = 3,
  AcpiPldJointTypeSwivel = 4,
} ACPI_PLD_JOINT_TYPE,
    *PACPI_PLD_JOINT_TYPE;

// Joint edge bits 10:12,24:26
typedef enum _ACPI_PLD_EDGE {
  AcpiPldEdgeTop = 0,
  AcpiPldEdgeBottom = 1,
  AcpiPldEdgeLeft = 2,
  AcpiPldEdgeRight = 3,
  AcpiPldEdgeUnknown = 4,
} ACPI_PLD_EDGE,
    *PACPI_PLD_EDGE;

//
// ACPI PLD Spatial Descriptor Buffer, Revision 1
// (MSFT custom PLD buffer extension, 128-bits)
//

typedef struct _ACPI_PLD_SPATIAL_BUFFER {
  UINT32 Revision : 5;
  UINT32 RollRotation : 9;
  UINT32 PitchRotation : 9;
  UINT32 YawRotation : 9;
  UINT32 Width : 16;
  UINT32 Height : 16;
  UINT32 Length : 16;
  UINT32 HorizontalOffset : 16;
  UINT32 VerticalOffset : 16;
  UINT32 DepthOffset : 16;
} ACPI_PLD_SPATIAL_BUFFER, *PACPI_PLD_SPATIAL_BUFFER;

DEFINE_GUID(ACPI_PLD_SPATIAL_BUFFER_GUID, 0x59af1a1f, 0xaba4, 0x4bb8, 0x81,
            0xef, 0x55, 0x93, 0x8e, 0x9b, 0xc5, 0x3a);

//
// ACPI PLD Interface Class Descriptor Buffer, Revision 1
// (MSFT custom PLD buffer extension, 128-bits)
//

typedef struct _ACPI_PLD_INTERFACE_CLASS_BUFFER {
  GUID ClassGuid;
} ACPI_PLD_INTERFACE_CLASS_BUFFER, *PACPI_PLD_INTERFACE_CLASS_BUFFER;

DEFINE_GUID(ACPI_PLD_INTERFACE_CLASS_BUFFER_GUID, 0x1facec76, 0x96a8, 0x4d9e,
            0x84, 0x6e, 0x3a, 0x6d, 0x68, 0x7c, 0x32, 0xfc);

//
// ACPI PLD Interface Instance ANSI/Unicode/GUID Descriptor Buffers, Revision 1
// (MSFT custom PLD buffer extension, 128-bits)
//

typedef struct _ACPI_PLD_INTERFACE_INSTANCE_ANSI_BUFFER {
  CHAR ReferenceString[16];
} ACPI_PLD_INTERFACE_INSTANCE_ANSI_BUFFER,
    *PACPI_PLD_INTERFACE_INSTANCE_ANSI_BUFFER;

typedef struct _ACPI_PLD_INTERFACE_INSTANCE_UNICODE_BUFFER {
  WCHAR ReferenceString[8];
} ACPI_PLD_INTERFACE_INSTANCE_UNICODE_BUFFER,
    *PACPI_PLD_INTERFACE_INSTANCE_UNICODE_BUFFER;

typedef struct _ACPI_PLD_INTERFACE_INSTANCE_GUID_BUFFER {
  GUID ReferenceGuid;
} ACPI_PLD_INTERFACE_INSTANCE_GUID_BUFFER,
    *PACPI_PLD_INTERFACE_INSTANCE_GUID_BUFFER;

DEFINE_GUID(ACPI_PLD_INTERFACE_INSTANCE_ANSI_BUFFER_GUID, 0x1facea4b, 0xda66,
            0x484c, 0xba, 0x5b, 0x51, 0x27, 0xe0, 0x5f, 0x95, 0xb2);

DEFINE_GUID(ACPI_PLD_INTERFACE_INSTANCE_UNICODE_BUFFER_GUID, 0x1face3f6, 0x1a60,
            0x4686, 0x98, 0x33, 0xec, 0x84, 0x02, 0xd4, 0x3b, 0x04);

DEFINE_GUID(ACPI_PLD_INTERFACE_INSTANCE_GUID_BUFFER_GUID, 0x1face9db, 0x2530,
            0x4248, 0x8e, 0xe3, 0x51, 0x05, 0x3a, 0xef, 0x47, 0xc2);

//
// ACPI PLD Container Descriptor Buffer
// (stored in _PLD custom data buffer, fixed size of 128-bits)
//

typedef struct _ACPI_PLD_CONTAINER_BUFFER {
  GUID ContainerId;
} ACPI_PLD_CONTAINER_BUFFER, *PACPI_PLD_CONTAINER_BUFFER;

//
// {c02fa109-6a82-4188-9f66-b190ba62db49}
//

DEFINE_GUID(ACPI_PLD_CONTAINER_BUFFER_GUID, 0xc02fa109, 0x6a82, 0x4188, 0x9f,
            0x66, 0xb1, 0x90, 0xba, 0x62, 0xdb, 0x49);

//
// NFIT ACPI table (ACPI 6.0 section 5.2.25)
//

#define NFIT_TABLE_SIGNATURE 0x5449464e  // 'NFIT'

typedef struct _NFIT_TABLE {
  DESCRIPTION_HEADER Header;

  ULONG Reserved;
  UCHAR Structures[ANYSIZE_ARRAY];
} NFIT_TABLE, *PNFIT_TABLE;

//
// NFIT Structure common header
//

typedef struct _NFIT_STRUCT_HEADER {
  USHORT Type;
  USHORT Length;
} NFIT_STRUCT_HEADER, *PNFIT_STRUCT_HEADER;

//
// NFIT Structure Types
//

typedef enum _NFIT_STRUCTURE_TYPE {
  NfitSystemPhysicalAddressRange = 0,
  NfitNvdimmRegionMapping = 1,
  NfitInterleave = 2,
  NfitSmbiosManagementInformation = 3,
  NfitNvdimmControlRegion = 4,
  NfitNvdimmBlockDataWindowRegion = 5,
  NfitFlushHintAddress = 6,
  NfitPlatformCapabilities = 7,
  NfitMaximum
} NFIT_STRUCTURE_TYPE,
    *PNFIT_STRUCTURE_TYPE;

#include <guiddef.h>

//
// System Physical Address Range Types (GUIDs)
//

DEFINE_GUID(/* 7305944F-FDDA-44E3-B16C3F22D252E5D0 */
            NFitSpaRangeVolatileMemory, 0x7305944F, 0xFDDA, 0x44E3, 0xB1, 0x6C,
            0x3F, 0x22, 0xD2, 0x52, 0xE5, 0xD0);

DEFINE_GUID(/* 66F0D379-B4F3-4074-AC430D3318B78CDB */
            NFitSpaRangeByteAddressablePersistentMemory, 0x66F0D379, 0xB4F3,
            0x4074, 0xAC, 0x43, 0x0D, 0x33, 0x18, 0xB7, 0x8C, 0xDB);

DEFINE_GUID(/* 92F701F6-13B4-405D-910B299367E8234C */
            NFitSpaRangeNvdimmControlRegion, 0x92F701F6, 0x13B4, 0x405D, 0x91,
            0x0B, 0x29, 0x93, 0x67, 0xE8, 0x23, 0x4C);

DEFINE_GUID(/* 91AF0530-5D86-470E-A6B00A2DB9408249 */
            NFitSpaRangeNvdimmBlockDataWindow, 0x91AF0530, 0x5D86, 0x470E, 0xA6,
            0xB0, 0x0A, 0x2D, 0xB9, 0x40, 0x82, 0x49);

DEFINE_GUID(/* 77AB535A-45FC-624B-5560F7B281D1F96E */
            NFitSpaRangeVolatileVirtualDisk, 0x77AB535A, 0x45FC, 0x624B, 0x55,
            0x60, 0xF7, 0xB2, 0x81, 0xD1, 0xF9, 0x6E);

DEFINE_GUID(/* 3D5ABD30-4175-87CE-6D64D2ADE523C4BB */
            NFitSpaRangeVolatileVirtualCD, 0x3D5ABD30, 0x4175, 0x87CE, 0x6D,
            0x64, 0xD2, 0xAD, 0xE5, 0x23, 0xC4, 0xBB);

DEFINE_GUID(/* 5CEA02C9-4D07-69D3-269F4496FBE096F9 */
            NFitSpaRangePersistentVirtualDisk, 0x5CEA02C9, 0x4D07, 0x69D3, 0x26,
            0x9F, 0x44, 0x96, 0xFB, 0xE0, 0x96, 0xF9);

DEFINE_GUID(/* 08018188-42CD-BB48-100F5387D53DED3D */
            NFitSpaRangePersistentVirtualCD, 0x08018188, 0x42CD, 0xBB48, 0x10,
            0x0F, 0x53, 0x87, 0xD5, 0x3D, 0xED, 0x3D);

//
// System Physical Address Range Flags
//

typedef enum _NFIT_SPA_RANGE_FLAG {
  NfitSpaRangeFlagManagementOnlyControlRegion = 0x0001,
  NfitSpaRangeFlagProximityDomainValid = 0x0002,
} NFIT_SPA_RANGE_FLAG,
    *PNFIT_SPA_RANGE_FLAG;

#define NFIT_VALID_SPA_RANGE_FLAGS               \
  (NfitSpaRangeFlagManagementOnlyControlRegion | \
   NfitSpaRangeFlagProximityDomainValid)
//
// System Physical Address Range Memory Mapping Attributes (defined in UEFI)
//

typedef enum _NFIT_SPA_RANGE_MEMORY_MAP_ATTRIBUTES {
  NfitSpaRangeAttributeUC = 0x00000001,
  NfitSpaRangeAttributeWC = 0x00000002,
  NfitSpaRangeAttributeWT = 0x00000004,
  NfitSpaRangeAttributeWB = 0x00000008,
  NfitSpaRangeAttributeUCE = 0x00000010,
  NfitSpaRangeAttributeWP = 0x00001000,
  NfitSpaRangeAttributeRP = 0x00002000,
  NfitSpaRangeAttributeXP = 0x00004000,
  NfitSpaRangeAttributeNV = 0x00008000,
  NfitSpaRangeAttributeMoreReliable = 0x00010000
} NFIT_SPA_RANGE_MEMORY_MAP_ATTRIBUTES,
    *PNFIT_SPA_RANGE_MEMORY_MAP_ATTRIBUTES;

#define NFIT_VALID_SPA_RANGE_MEMORY_MAP_ATTRIBUTES      \
  (NfitSpaRangeAttributeUC | NfitSpaRangeAttributeWC |  \
   NfitSpaRangeAttributeWT | NfitSpaRangeAttributeWB |  \
   NfitSpaRangeAttributeUCE | NfitSpaRangeAttributeWP | \
   NfitSpaRangeAttributeRP | NfitSpaRangeAttributeXP |  \
   NfitSpaRangeAttributeNV | NfitSpaRangeAttributeMoreReliable)

//
// NFIT System Physical Address Range Structure
//

typedef struct _NFIT_SYSTEM_PHYSICAL_ADDRESS_RANGE {
  NFIT_STRUCT_HEADER Header;
  USHORT SPARangeIndex;
  USHORT Flags;
  UCHAR Reserved[4];
  ULONG ProximityDomain;
  GUID AddressRangeType;
  ULONG64 SPARangeBase;
  ULONG64 SPARangeLength;
  ULONG64 AddressRangeMappingAttribute;
} NFIT_SYSTEM_PHYSICAL_ADDRESS_RANGE, *PNFIT_SYSTEM_PHYSICAL_ADDRESS_RANGE;

#if _MSC_VER >= 1200
#pragma warning(push)
#endif

#pragma warning(disable : 4214)  // nonstandard extension used : bit field types
                                 // other than int

typedef struct _NFIT_DEVICE_HANDLE {
  union {
    struct {
      ULONG DIMMNumber : 4;
      ULONG MemoryChannelNumber : 4;
      ULONG MemoryControllerId : 4;
      ULONG SocketId : 4;
      ULONG NodeControllerId : 12;
      ULONG Reserved : 4;
    } NfitHandle;

    ULONG AsUlong;
  } u;
} NFIT_DEVICE_HANDLE, *PNFIT_DEVICE_HANDLE;

#if _MSC_VER >= 1200
#pragma warning(pop)
#endif

//
// Memory Device State Flags
//

typedef enum _NFIT_NVDIMM_STATE_FLAG {
  NfitNvdimmStateFlagSaveFail = 0x0001,
  NfitNvdimmStateFlagRestoreFail = 0x0002,
  NfitNvdimmStateFlagPlatformFlushFail = 0x0004,
  NfitNvdimmStateFlagNotArm = 0x0008,
  NfitNvdimmStateFlagHealthEvent = 0x0010,
  NfitNvdimmStateFlagNotifyEnabled = 0x0020,
  NfitNvdimmStateNoSPAMapping = 0x0040,
} NFIT_NVDIMM_STATE_FLAG,
    *PNFIT_NVDIMM_STATE_FLAG;

#define NFIT_VALID_NVDIMM_STATE_FLAGS                                  \
  (NfitNvdimmStateFlagSaveFail | NfitNvdimmStateFlagRestoreFail |      \
   NfitNvdimmStateFlagPlatformFlushFail | NfitNvdimmStateFlagNotArm |  \
   NfitNvdimmStateFlagHealthEvent | NfitNvdimmStateFlagNotifyEnabled | \
   NfitNvdimmStateNoSPAMapping)

//
// NFIT Memory Device to System Physical Address Range Mapping Structure
//

typedef struct _NFIT_NVDIMM_REGION_MAPPING {
  NFIT_STRUCT_HEADER Header;
  NFIT_DEVICE_HANDLE NfitHandle;
  USHORT NVDIMMPhysicalId;
  USHORT NVDIMMRegionId;
  USHORT SpaRangeIndex;
  USHORT NVDIMMControlRegionIndex;
  ULONG64 NVDIMMRegionSize;
  ULONG64 RegionOffset;
  ULONG64 NVDIMMPhysicalAddrRegionBase;
  USHORT InterleaveIndex;
  USHORT InterleaveWays;
  USHORT NVDIMMStateFlags;
  UCHAR Reserved[2];
} NFIT_NVDIMM_REGION_MAPPING, *PNFIT_NVDIMM_REGION_MAPPING;

//
// NFIT Interleave Structure
//

typedef struct _NFIT_INTERLEAVE {
  NFIT_STRUCT_HEADER Header;
  USHORT InterleaveIndex;
  UCHAR Reserved[2];
  ULONG LinesDescribed;
  ULONG LineSize;
  ULONG LineOffsets[ANYSIZE_ARRAY];
} NFIT_INTERLEAVE, *PNFIT_INTERLEAVE;

//
// NFIT SMBIOS Management Information Structure
//

typedef struct _NFIT_SMBIOS_MANAGEMENT_INFO {
  NFIT_STRUCT_HEADER Header;
  UCHAR Reserved[4];
  UCHAR Data[ANYSIZE_ARRAY];
} NFIT_SMBIOS_MANAGEMENT_INFO, *PNFIT_SMBIOS_MANAGEMENT_INFO;

//
// NVDIMM Control Region Flags
//

typedef enum _NFIT_CONTROL_REGION_FLAG {
  NfitControlRegionFlagBufferBlockDataWindow = 0x0001
} NFIT_CONTROL_REGION_FLAG,
    *PNFIT_CONTROL_REGION_FLAG;

#define NFIT_VALID_CONTROL_REGION_FLAGS \
  (NfitControlRegionFlagBufferBlockDataWindow)

//
// NFIT Control Region Structure. Definition based on ACPI 6.1 as
// that will be the requirement.
//

#if _MSC_VER >= 1200
#pragma warning(push)
#endif

#pragma warning(disable : 4214)  // nonstandard extension used : bit field types
                                 // other than int

typedef struct _NFIT_NVDIMM_CONTROL_REGION {
  NFIT_STRUCT_HEADER Header;
  USHORT NvdimmControlRegionIndex;
  USHORT VendorId;
  USHORT DeviceId;
  USHORT RevisionId;
  USHORT SubsystemVendorId;
  USHORT SubsystemDeviceId;
  USHORT SubsystemRevisionId;
  union {
    UCHAR AsUchar;
    struct {
      UCHAR ManufacturingFields : 1;
      UCHAR Reserved : 7;
    } u;
  } ValidFields;
  UCHAR ManufacturingLocation;
  UCHAR ManufacturingYear;
  UCHAR ManufacturingWeek;
  UCHAR Reserved[2];
  ULONG SerialNumber;
  USHORT RegionFormatInterfaceCode;
  USHORT BCWCount;
  ULONG64 BCWSize;
  ULONG64 BCWCommandRegisterOffset;
  ULONG64 BCWCommandRegisterSize;
  ULONG64 BCWStatusRegisterOffset;
  ULONG64 BCWStatusRegisterSize;
  USHORT NvdimmControlRegionFlag;
  UCHAR Reserved1[6];
} NFIT_NVDIMM_CONTROL_REGION, *PNFIT_NVDIMM_CONTROL_REGION;

#define NVDIMM_CONTROL_REGION_BASE_SIZE \
  (FIELD_OFFSET(NFIT_NVDIMM_CONTROL_REGION, BCWSize))
#define NVDIMM_CONTROL_REGION_EXTENDED_SIZE (sizeof(NFIT_NVDIMM_CONTROL_REGION))

//
// NFIT Block Data Window Region Structure
//

typedef struct _NFIT_BLOCK_DATA_WINDOW_REGION {
  NFIT_STRUCT_HEADER Header;
  USHORT NvdimmControlRegionIndex;
  USHORT BlockDataWindowCount;
  ULONG64 BlockDataWindowOffset;
  ULONG64 BlockDataWindowSize;
  ULONG64 BlockAccessibleCapacity;
  ULONG64 FirstAccessibleBlockAddress;
} NFIT_BLOCK_DATA_WINDOW_REGION, *PNFIT_BLOCK_DATA_WINDOW_REGION;

//
// NFIT Flush Hint Address Structure
//

typedef struct _NFIT_FLUSH_HINT_ADDRESS {
  NFIT_STRUCT_HEADER Header;
  NFIT_DEVICE_HANDLE NfitHandle;
  USHORT FlushHintAddressCount;
  UCHAR Reserved[6];
  ULONG64 FlushHintAddress[ANYSIZE_ARRAY];
} NFIT_FLUSH_HINT_ADDRESS, *PNFIT_FLUSH_HINT_ADDRESS;

//
// NFIT Platform Capabilities Structure
// Originally defined in ACPI 6.2 Errata A.
//
typedef struct _NFIT_PLATFORM_CAPABILITIES {
  NFIT_STRUCT_HEADER Header;
  UCHAR HighestValidCapability;
  UCHAR Reserved[3];
  union {
    ULONG AsUlong;
    struct {
      ULONG CpuCacheFlushOnPowerLoss : 1;
      ULONG MemCtrlrFlushOnPowerLoss : 1;
      ULONG PmemHardwareMirroring : 1;
    } u;
  } Capabilities;
  UCHAR Reserved1[4];
} NFIT_PLATFORM_CAPABILITIES, *PNFIT_PLATFORM_CAPABILITIES;

#if _MSC_VER >= 1200
#pragma warning(pop)
#endif

//
// WSMT ACPI Table definition
//

#define WSMT_SIGNATURE 0x544D5357  // "WSMT"

#if _MSC_VER >= 1200
#pragma warning(push)
#endif

#pragma warning( \
    disable : 4201)  // nonstandard extension used : nameless struct/union
#pragma warning(disable : 4214)  // nonstandard extension used : bit field types
                                 // other than int

typedef union _WSMT_PROTECTION_FLAGS {
  ULONG AsUlong;

  struct {
    ULONG FixedCommBuffers : 1;
    ULONG CommBufferNestedPtrProtection : 1;
    ULONG SystemResourceProtection : 1;
    ULONG Reserved : 29;
  } DUMMYSTRUCTNAME;

} WSMT_PROTECTION_FLAGS, *PWSMT_PROTECTION_FLAGS;

#if _MSC_VER >= 1200
#pragma warning(pop)
#endif

typedef struct _WSMT {
  DESCRIPTION_HEADER Header;
  WSMT_PROTECTION_FLAGS ProtectionFlags;
} WSMT, *PWSMT;

//
// LPIT ACPI Table definition
//

#define LPIT_SIGNATURE 0x5449504C  // "LPIT"

#if _MSC_VER >= 1200
#pragma warning(push)
#endif

#pragma warning( \
    disable : 4201)  // nonstandard extension used : nameless struct/union
#pragma warning(disable : 4214)  // nonstandard extension used : bit field types
                                 // other than int

typedef union _LPI_STATE_FLAGS {
  struct {
    ULONG Disabled : 1;
    ULONG CounterUnavailable : 1;
    ULONG Reserved : 30;
  };
  ULONG AsUlong;
} LPI_STATE_FLAGS, *PLPI_STATE_FLAGS;

typedef struct _LPI_STATE_DESCRIPTOR {
  ULONG Type;
  ULONG Length;
  USHORT UniqueId;
  UCHAR Reserved[2];
  LPI_STATE_FLAGS Flags;
  GEN_ADDR EntryTrigger;
  ULONG Residency;
  ULONG Latency;
  GEN_ADDR ResidencyCounter;
  ULONGLONG ResidencyCounterFrequency;
} LPI_STATE_DESCRIPTOR, *PLPI_STATE_DESCRIPTOR;

typedef struct _LPIT {
  DESCRIPTION_HEADER Header;
  LPI_STATE_DESCRIPTOR LpiStates[ANYSIZE_ARRAY];
} LPIT, *PLPIT;

#if _MSC_VER >= 1200
#pragma warning(pop)
#endif

//
// PPTT (Processor Properties Topology Table) definition
//

#define PPTT_SIGNATURE 0x54545050  // "PPTT"

#if _MSC_VER >= 1200
#pragma warning(push)
#endif

#pragma warning( \
    disable : 4201)  // nonstandard extension used : nameless struct/union
#pragma warning(disable : 4214)  // nonstandard extension used : bit field types
                                 // other than int

typedef union _PROC_TOPOLOGY_NODE_FLAGS {
  struct {
    ULONG PhysicalPackage : 1;
    ULONG ACPIProcessorIdValid : 1;
    ULONG Reserved : 30;
  };

  ULONG AsULONG;
} PROC_TOPOLOGY_NODE_FLAGS, *PPROC_TOPOLOGY_NODE_FLAGS;

typedef union _PROC_TOPOLOGY_CACHE_FLAGS {
  struct {
    ULONG SizeValid : 1;
    ULONG SetsValid : 1;
    ULONG AssociativityValid : 1;
    ULONG AllocationTypeValid : 1;
    ULONG CacheTypeValid : 1;
    ULONG WritePolicyValid : 1;
    ULONG LineSizeValid : 1;
    ULONG Reserved : 25;
  };

  ULONG AsULONG;
} PROC_TOPOLOGY_CACHE_FLAGS, *PPROC_TOPOLOGY_CACHE_FLAGS;

#define PROC_TOPOLOGY_NODE_CACHE_TYPE_DATA(CacheType) (CacheType == 0)

#define PROC_TOPOLOGY_NODE_CACHE_TYPE_INSTRUCTION(CacheType) (CacheType == 1)

#define PROC_TOPOLOGY_NODE_CACHE_TYPE_UNIFIED(CacheType) \
  ((CacheType == 2) || (CacheType == 3))

typedef union _PROC_TOPOLOGY_CACHE_ATTRIBUTES {
  struct {
    UCHAR ReadAllocate : 1;
    UCHAR WriteAllocate : 1;
    UCHAR CacheType : 2;
    UCHAR WritePolicy : 1;
    UCHAR Reserved : 3;
  };

  UCHAR AsUCHAR;
} PROC_TOPOLOGY_CACHE_ATTRIBUTES, *PPROC_TOPOLOGY_CACHE_ATTRIBUTES;

typedef struct _PROC_TOPOLOGY_NODE PROC_TOPOLOGY_NODE, *PPROC_TOPOLOGY_NODE;

#define PROC_TOPOLOGY_NODE_HEIRARCHY 0
#define PROC_TOPOLOGY_NODE_CACHE 1
#define PROC_TOPOLOGY_NODE_ID 2

struct _PROC_TOPOLOGY_NODE {
  struct {
    UCHAR Type;
    UCHAR Length;
    UCHAR Reserved[2];
  };

  union {
    struct {
      PROC_TOPOLOGY_NODE_FLAGS Flags;
      ULONG Parent;
      ULONG ACPIProcessorId;
      ULONG NumberPrivateResources;
      ULONG PrivateResources[ANYSIZE_ARRAY];
    } HeirarchyNode;

    struct {
      PROC_TOPOLOGY_CACHE_FLAGS Flags;
      ULONG NextLevelCacheOffset;
      ULONG Size;
      ULONG Sets;
      UCHAR Associativity;
      PROC_TOPOLOGY_CACHE_ATTRIBUTES Attributes;
      USHORT LineSize;
    } CacheNode;

    struct {
      ULONG Vendor;
      ULONG64 Level1;
      ULONG64 Level2;
      USHORT Major;
      USHORT Minor;
      USHORT Spin;
    } IdNode;
  };
};

typedef struct _PPTT {
  DESCRIPTION_HEADER Header;
  PROC_TOPOLOGY_NODE HeirarchyNodes[ANYSIZE_ARRAY];
} PPTT, *PPPTT;

#if _MSC_VER >= 1200
#pragma warning(pop)
#endif

//
// PDTT structure.
//

#define PDTT_SIGNATURE 0x54544450  // "PDTT"

#if _MSC_VER >= 1200
#pragma warning(push)
#endif

#pragma warning(disable : 4214)

typedef struct _PDTT_PCC_SUBCHANNEL_INDENTIFIER {
  UCHAR SubChannelId;
  UCHAR Runtime : 1;
  UCHAR WaitCompletion : 1;
  UCHAR Reserved : 6;
} PDTT_PCC_SUBCHANNEL_INDENTIFIER, *PPDTT_PCC_SUBCHANNEL_INDENTIFIER;

typedef struct _ACPI_PDTT {
  DESCRIPTION_HEADER Header;
  UCHAR TriggerCount;
  UCHAR Reserved[3];
  ULONG TriggerOffset;
  // PDTT pcc sub channel indetifiers
} ACPI_PDTT, *PACPI_PDTT;

#if _MSC_VER >= 1200
#pragma warning(pop)
#endif

//
// Heterogeneous Memory Attribute Table (HMAT) definition. The following
// abbreviations are used for the various table entry types:
//
//     MSAR - Memory Subsystem Address Range
//
//     SLLBI - System Locality Latency and Bandwidth Information
//
//     MSCI - Memory Side Cache Information
//
// The identifiers use these abbreviations or they would have become
// prohibitively long.
//

#if _MSC_VER >= 1200
#pragma warning(push)
#endif

#pragma warning(disable : 4214)  // nonstandard extension used : bit field types
                                 // other than int
#pragma warning( \
    disable : 4201)  // nonstandard extension used : nameless struct/union

#define HMAT_SIGNATURE 0x54414D48  // "HMAT"

#define HMAT_ENTRY_TYPE_MSAR 0
#define HMAT_ENTRY_TYPE_SLLBI 1
#define HMAT_ENTRY_TYPE_MSCI 2

#define HMAT_SLLBI_DATA_TYPE_ACCESS_LATENCY 0
#define HMAT_SLLBI_DATA_TYPE_READ_LATENCY 1
#define HMAT_SLLBI_DATA_TYPE_WRITE_LATENCY 2
#define HMAT_SLLBI_DATA_TYPE_ACCESS_BANDWIDTH 3
#define HMAT_SLLBI_DATA_TYPE_READ_BANDWIDTH 4
#define HMAT_SLLBI_DATA_TYPE_WRITE_BANDWIDTH 5

#define HMAT_MSCI_CACHEATTRIBUTES_LEVELS_NONE 0
#define HMAT_MSCI_CACHEATTRIBUTES_LEVELS_ONE 1
#define HMAT_MSCI_CACHEATTRIBUTES_LEVELS_TWO 2
#define HMAT_MSCI_CACHEATTRIBUTES_LEVELS_THREE 3

#define HMAT_MSCI_CACHEATTRIBUTES_ASSOCIATIVITY_NONE 0
#define HMAT_MSCI_CACHEATTRIBUTES_ASSOCIATIVITY_DIRECT_MAPPED 1
#define HMAT_MSCI_CACHEATTRIBUTES_ASSOCIATIVITY_COMPLEX 2

#define HMAT_MSCI_CACHEATTRIBUTES_WRITE_POLICY_NONE 0
#define HMAT_MSCI_CACHEATTRIBUTES_WRITE_POLICY_WRITE_BACK 1
#define HMAT_MSCI_CACHEATTRIBUTES_WRITE_POLICY_WRITE_THROUGH 2

typedef struct _HMAT_ENTRY {
  USHORT Type;
  USHORT Reserved;
  ULONG Length;

  union {
    //
    // Memory Subsystem Address Range structure
    //

    struct {
      union {
        struct {
          USHORT ProcessorProximityDomainValid : 1;
          USHORT MemoryProximityDomainValid : 1;
          USHORT ReservationHint : 1;
          USHORT Reserved : 13;
        } DUMMYSTRUCTNAME;

        USHORT AsUSHort;
      } Flags;

      USHORT Reserved1;
      ULONG ProcessorProximityDomain;
      ULONG MemoryProximityDomain;
      ULONG Reserved2;
      PHYSICAL_ADDRESS SystemPhysicalAddressRangeBase;
      ULONGLONG SystemPhysicalAddressRangeLength;
    } Msar;

    //
    // System Locality Latency and Bandwidth Information structure
    //

    struct {
      union {
        struct {
          UCHAR Memory : 1;
          UCHAR LastLevelMemory : 1;
          UCHAR FirstLevelMemorySideCache : 1;
          UCHAR SecondLevelMemorySideCache : 1;
          UCHAR ThirdLevelMemorySideCache : 1;
          UCHAR Reserved : 3;
        } DUMMYSTRUCTNAME;

        UCHAR AsUChar;
      } Flags;

      UCHAR DataType;
      USHORT Reserved1;
      ULONG NumberOfInitiatorProximityDomains;
      ULONG NumberOfTargetProximityDomains;
      ULONG Reserved2;
      ULONGLONG EntryBaseUnit;
      // ULONG InitiatorProximityDomainList[NumberOfInitiatorProximityDomains]
      // ULONG TargetProximityDomainList[NumberOfTargetProximityDomains]
      // USHORT
      // Entry[NumberOfInitiatorProximityDomains][NumberOfTargetProximityDomains]
    } Sllbi;

    //
    // Memory Side Cache Information structure
    //

    struct {
      ULONG MemoryProximityDomain;
      ULONG Reserved1;
      ULONGLONG MemorySideCacheSize;

      union {
        struct {
          ULONG TotalCacheLevels : 4;
          ULONG CacheLevel : 4;
          ULONG CacheAssociativity : 4;
          ULONG WritePolicy : 4;
          ULONG CacheLineSize : 16;
        } DUMMYSTRUCTNAME;

        ULONG AsULong;
      } CacheAttributes;

      USHORT Reserved2;
      USHORT NumberOfSmBiosHandles;
      // USHORT SmBiosHandles[NumberOfSmBiosHandles];
    } Msci;
  } DUMMYUNIONNAME;
} HMAT_ENTRY, *PHMAT_ENTRY;

#define HMAT_ENTRY_HEADER_LENGTH RTL_SIZEOF_THROUGH_FIELD(HMAT_ENTRY, Length);
#define HMAT_ENTRY_LENGTH(_Type) RTL_SIZEOF_THROUGH_FIELD(HMAT_ENTRY, _Type);

typedef struct _HMAT_TABLE {
  DESCRIPTION_HEADER Header;
  ULONG Reserved;
  // HMAT_ENTRY Entries[];
} HMAT_TABLE, *PHMAT_TABLE;

#if _MSC_VER >= 1200
#pragma warning(pop)
#endif

//
// Resume normal structure packing
//

#include <poppack.h>

#endif  // _ACPITBL_H