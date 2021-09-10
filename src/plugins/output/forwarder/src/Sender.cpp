/**
 * \file src/plugins/output/forwarder/src/Sender.cpp
 * \author Michal Sedlak <xsedla0v@stud.fit.vutbr.cz>
 * \brief Sender class implementation
 * \date 2021
 */

/* Copyright (C) 2021 CESNET, z.s.p.o.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name of the Company nor the names of its contributors
 *    may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * ALTERNATIVELY, provided that this notice is retained in full, this
 * product may be distributed under the terms of the GNU General Public
 * License (GPL) version 2 or later, in which case the provisions
 * of the GPL apply INSTEAD OF those given above.
 *
 * This software is provided ``as is'', and any express or implied
 * warranties, including, but not limited to, the implied warranties of
 * merchantability and fitness for a particular purpose are disclaimed.
 * In no event shall the company or contributors be liable for any
 * direct, indirect, incidental, special, exemplary, or consequential
 * damages (including, but not limited to, procurement of substitute
 * goods or services; loss of use, data, or profits; or business
 * interruption) however caused and on any theory of liability, whether
 * in contract, strict liability, or tort (including negligence or
 * otherwise) arising in any way out of the use of this software, even
 * if advised of the possibility of such damage.
 *
 */

#include "Sender.h"
#include <libfds.h>

static int32_t
find_drec_after_set(ipx_msg_ipfix_t *msg, fds_ipfix_set_hdr *set_hdr)
{
    uint32_t drec_cnt = ipx_msg_ipfix_get_drec_cnt(msg);

    uint8_t *set_end = (uint8_t *) set_hdr + ntohs(set_hdr->length);

    for (uint32_t i = 0; i < drec_cnt; i++) {
        const ipx_ipfix_record *drec = ipx_msg_ipfix_get_drec(msg, i);

        if (drec->rec.data > set_end) {
            return i;
        }
    }

    return -1;
}

Sender::Sender(std::function<void(Message &)> emit_callback, bool do_withdrawals,
               unsigned int tmplts_resend_pkts, unsigned int tmplts_resend_secs) :
    m_emit_callback(emit_callback),
    m_do_withdrawals(do_withdrawals),
    m_tmplts_resend_pkts(tmplts_resend_pkts),
    m_tmplts_resend_secs(tmplts_resend_secs)
{
}

void
Sender::process_message(ipx_msg_ipfix_t *msg)
{
    // Begin the message, use the original message header with the sequence number replaced
    fds_ipfix_msg_hdr msg_hdr = *(fds_ipfix_msg_hdr *) ipx_msg_ipfix_get_packet(msg);
    msg_hdr.seq_num = htonl(m_seq_num);

    m_message.start(&msg_hdr);

    // Get current time
    time_t now = get_monotonic_time();

    // Send templates update if necessary and possible
    ipx_ipfix_record *drec = ipx_msg_ipfix_get_drec(msg, 0);

    if (drec) {

        const fds_tsnapshot_t *tsnap = drec->rec.snap;

        // If templates changed or any of the resend intervals elapsed
        if (m_tsnap != tsnap
                || (m_tmplts_resend_pkts != 0 && m_pkts_since_tmplts_sent >= m_tmplts_resend_pkts)
                || (m_tmplts_resend_secs != 0 && now - m_last_tmplts_sent_time >= m_tmplts_resend_secs)) {

            process_templates(tsnap, m_seq_num);
        }
    }

    // Get sets from the message
    ipx_ipfix_set *sets;
    size_t num_sets;
    ipx_msg_ipfix_get_sets(msg, &sets, &num_sets);

    for (size_t i = 0; i < num_sets; i++) {

        fds_ipfix_set_hdr *set_hdr = sets[i].ptr;
        uint16_t set_id = ntohs(set_hdr->flowset_id);

        // If it's not a template set, add the set as is
        if (set_id != 2 && set_id != 3) {
            m_message.add_set(set_hdr);
            continue;
        }

        // It is a template set...

        // Find first data record after the template set
        int32_t drec_idx = find_drec_after_set(msg, set_hdr);

        // The template set is at the end, we'll have to wait for next message to grab the template snapshot
        if (drec_idx == -1) {
            break;
        }

        // Get template snapshot from the data record after template set
        ipx_ipfix_record *drec = ipx_msg_ipfix_get_drec(msg, drec_idx);
        const fds_tsnapshot_t *tsnap = drec->rec.snap;

        // In case the template set is at the start of the message and we already sent the templates
        if (m_tsnap == tsnap) {
            continue;
        }

        // The next sequence number in case we'll need to start another message
        uint32_t next_seq_num = m_seq_num + drec_idx;

        process_templates(tsnap, next_seq_num);
    }


    if (!m_message.empty()) {
        m_message.finalize();
        emit_message();
    }

    m_seq_num += ipx_msg_ipfix_get_drec_cnt(msg);
    m_pkts_since_tmplts_sent++;
}


void
Sender::lose_message(ipx_msg_ipfix_t *msg)
{
    m_seq_num += ipx_msg_ipfix_get_drec_cnt(msg);
    m_pkts_since_tmplts_sent++;
}

void
Sender::process_templates(const fds_tsnapshot_t *tsnap, uint32_t next_seq_num)
{
    if (m_do_withdrawals) {
        m_message.add_template_withdrawal_all();
    }

    tsnapshot_for_each(tsnap, [&](const fds_template *tmplt) {
        // Should we start another message?
        if (m_message.length() + sizeof(fds_ipfix_set_hdr) + tmplt->raw.length > TMPLTMSG_MAX_LENGTH && !m_message.empty()) {
            m_message.finalize();

            emit_message();

            fds_ipfix_msg_hdr msg_hdr = *m_message.header();
            msg_hdr.seq_num = htonl(next_seq_num);
            m_message.start(&msg_hdr);
        }

        m_message.add_template(tmplt);
    });


    if (!m_message.empty()) {
        m_message.finalize();
        emit_message();
    }

    m_tsnap = tsnap;
    m_last_tmplts_sent_time = get_monotonic_time();
    m_pkts_since_tmplts_sent = 0;

    fds_ipfix_msg_hdr msg_hdr = *m_message.header();
    msg_hdr.seq_num = htonl(next_seq_num);
    m_message.start(&msg_hdr);
}

void
Sender::emit_message()
{
    m_emit_callback(m_message);
}

void
Sender::clear_templates()
{
    m_tsnap = nullptr;
}
