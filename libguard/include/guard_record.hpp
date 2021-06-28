// SPDX-License-Identifier: Apache-2.0
#pragma once
#include "../guard_common.hpp"

#include <vector>

namespace openpower
{
namespace guard
{

#define GUARD_MAGIC "GUARDREC" //! "GUARDREC"
/* From hostboot: src/include/usr/hwas/hwasPlatDeconfigGard.H */
const uint8_t CURRENT_GARD_VERSION_LAYOUT = 0x1;
#define GUARD_RESOLVED 0xFFFFFFFF

#ifdef PGUARD
/* From hostboot: src/include/usr/hwas/common/deconfigGard.H:GuardRecord */
struct GuardRecord
{
    uint32_t recordId;
    EntityPath targetId;
    uint32_t elogId;
    uint8_t errType;
    uint8_t resRecovery;
    uint8_t pad1[6];
} __attribute__((__packed__));
#else
struct GuardRecord
{
    uint32_t recordId;   ///< id of the guard record all F's indicate an invalid
                         ///< guard record
    EntityPath targetId; ///< Physical path of the target being guardeds
    uint32_t elogId;     ///< Id of the error which initiated the guarding
    uint8_t errType;     ///< from hwasCallout.H GUARD_ErrorType

    union uniqueId_t
    {
        uint8_t uniqueId[80]; ///< Universal generic id
        struct ibm11S_t
        {
            uint8_t serialNum[12]; ///< Serial number of the containing FRU
            uint8_t partNum[7];    ///< Part number of the containg FRU
        } __attribute__((__packed__)) s1;

        struct isdimmDDR3_t
        {
            uint8_t serialNum[4]; ///< Serial number of the containing FRU
            uint8_t partNum[18];  ///< Part number of the containg FRU
        } __attribute__((__packed__)) s2;

        struct isdimmDDR4_t
        {
            uint8_t serialNum[4]; ///< Serial number of the containing FRU
            uint8_t partNum[20];  ///< Part number of the containg FRU
        } __attribute__((__packed__)) s3;

        struct isddimmDDR4_t
        {
            uint8_t serialNum[4]; ///< Serial number of the containing FRU
            uint8_t partNum[30];  ///< Part number of the containg FRU
        } __attribute__((__packed__)) s4;
    } u;
    uint8_t padding[18]; ///< Padding
} __attribute__((__packed__));
#endif

struct GuardRecord_t
{
    uint8_t iv_magicNumber[8];    ///< GUARDREC
    uint8_t iv_version;           ///< Guard records version
    uint8_t iv_padding[7];        ///< Padding
    GuardRecord* iv_guardRecords; ///< List of guard records
};

/* From hostboot: src/include/usr/hwas/common/hwasCallout.H */
enum GardType
{
    GARD_NULL = 0x00,
    GARD_User_Manual = 0xD2,
    GARD_Unrecoverable = 0xE2,
    GARD_Fatal = 0xE3,
    GARD_Predictive = 0xE6,
    GARD_Power = 0xE9,
    GARD_PHYP = 0xEA,
    GARD_Reconfig = 0xEB,
    GARD_Void = 0xFF,
};

using GuardRecords = std::vector<GuardRecord>;
} // namespace guard
} // namespace openpower
