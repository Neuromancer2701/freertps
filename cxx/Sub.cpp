//
// Created by root on 1/18/17.
//

#include "Sub.h"
#include "FreeRtps.h"

#include <algorithm>
using std::find_if;
using std::begin;
using std::end;


void Sub::frudp_add_reader(const frudp_reader_t reader)
{
    if (frudp_readers.size() >= FRUDP_MAX_READERS)
        return;
    // make sure that in the meantime, we haven't already added this


    auto guid_same = [reader] (frudp_reader_t r) { return r.writer_guid.eid == reader.writer_guid.eid; }

    if(find_if(begin(frudp_readers), end(frudp_readers),[]{}) == end(frudp_readers))
    {
        frudp_readers.push_back(reader);
    }
    else
    {
        printf("found reader already; skipping duplicate add\n");
        return;
    }

    bool found = false;
    for (unsigned j = 0; !found && j < g_frudp_num_readers; j++)
    {
        frudp_reader_t *r = &g_frudp_readers[j];
        if (frudp_guid_identical(&r->writer_guid, &match->writer_guid))
            found = true;
    }
    if (found)
    {
        printf("found reader already; skipping duplicate add\n");
        return;
    }

    g_frudp_readers[g_frudp_num_readers] = *match;
    g_frudp_num_readers++;
}

void Sub::frudp_print_readers(void)
{
    for (unsigned i = 0; i < g_frudp_num_readers; i++)
    {
        frudp_reader_t *match = &g_frudp_readers[i];
        printf("    sub %d: writer = ", (int)i); //%08x, reader = %08x\n",
        frudp_print_guid(&match->writer_guid);
        printf(" => %08x\r\n", (unsigned)freertps_htonl(match->reader_eid.u));
    }
}

void Sub::frudp_add_user_sub(const char *topic_name, const char *type_name, freertps_msg_cb_t msg_cb)
{

}

void Sub::frudp_add_sub(const frudp_sub_t *s)
{

}
