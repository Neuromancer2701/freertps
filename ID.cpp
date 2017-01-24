

#include <include/freertps/bswap.h>
#include "ID.h"

ID::ID():
frudp_next_user_eid(1)
{

}

string ID::frudp_vendor(const frudp_vid_t vid) {
    switch (vid)
    {
        case 0x0101:                return "RTI Connext";
        case 0x0102:                return "PrismTech OpenSplice";
        case 0x0103:                return "OCI OpenDDS";
        case 0x0104:                return "MilSoft";
        case 0x0105:                return "Gallium InterCOM";
        case 0x0106:                return "TwinOaks CoreDX";
        case 0x0107:                return "Lakota Technical Systems";
        case 0x0108:                return "ICOUP Consulting";
        case 0x0109:                return "ETRI";
        case 0x010a:                return "RTI Connext Micro";
        case 0x010b:                return "PrismTech Vortex Cafe";
        case 0x010c:                return "PrismTech Vortex Gateway";
        case 0x010d:                return "PrismTech Vortex Lite";
        case 0x010e:                return "Technicolor Qeo";
        case 0x010f:                return "eProsima";
        case 0x0120:                return "PrismTech Vortex Cloud";
        case FREERTPS_VENDOR_ID:    return "freertps";
        default:                    return "unknown";
    }
}


void ID::frudp_print_guid_prefix(const frudp_guid_prefix_t *prefix)
{
    printf("%02x%02x%02x%02x:%02x%02x%02x%02x:%02x%02x%02x%02x",
           (unsigned)(*prefix)[0],
           (unsigned)(*prefix)[1],
           (unsigned)(*prefix)[2],
           (unsigned)(*prefix)[3],
           (unsigned)(*prefix)[4],
           (unsigned)(*prefix)[5],
           (unsigned)(*prefix)[6],
           (unsigned)(*prefix)[7],
           (unsigned)(*prefix)[8],
           (unsigned)(*prefix)[9],
           (unsigned)(*prefix)[10],
           (unsigned)(*prefix)[11]);
}


bool ID::frudp_guid_prefix_identical(frudp_guid_prefix_t * const a, frudp_guid_prefix_t * const b)
{
    for (int i = 0; i < FRUDP_GUID_PREFIX_LEN; i++)
        if ((*a)[i] != (*b)[i])
            return false;
    return true;
}

bool ID::frudp_guid_identical(const frudp_guid_t * const a, const frudp_guid_t * const b)
{
    if (a->eid.u != b->eid.u)
        return false;
    for (int i = 0; i < FRUDP_GUID_PREFIX_LEN; i++)
        if (a->prefix[i] != b->prefix[i])
            return false;
    return true;
}


void ID::frudp_stuff_guid(frudp_guid_t *guid,
                      const frudp_guid_prefix_t *prefix,
                      const frudp_eid_t *id)
{
    guid->prefix =  *prefix;
    guid->eid = *id;
}

void ID::frudp_print_guid(const frudp_guid_t *guid)
{
    frudp_print_guid_prefix(&guid->prefix);
    printf(":%08x", (unsigned)freertps_htonl(guid->eid.u));
}

frudp_eid_t ID::frudp_create_user_id(const uint8_t entity_kind)
{
    printf("frudp_create_user_id()\r\n");
    frudp_eid_t eid;
    eid.s.kind = entity_kind; // entity kind must be set by caller of this functionmust be overwritten by FRUDP_ENTITY_KIND_USER_READER_NO_KEY; // has key? dunno
    eid.s.key[0] = 0;
    eid.s.key[1] = 0; // todo: >8 bit ID's
    eid.s.key[2] = frudp_next_user_eid++;
    return eid;
}
