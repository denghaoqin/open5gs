/*
 * Copyright (C) 2019 by Sukchan Lee <acetcom@gmail.com>
 *
 * This file is part of Open5GS.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "sxa-build.h"

ogs_pkbuf_t *sgwu_sxa_build_session_establishment_response(uint8_t type,
    sgwu_sess_t *sess, ogs_pfcp_pdr_t *created_pdr[], int num_of_created_pdr)
{
    ogs_pfcp_message_t pfcp_message;
    ogs_pfcp_session_establishment_response_t *rsp = NULL;

    int i = 0;

    ogs_pfcp_node_id_t node_id;
    ogs_pfcp_f_seid_t f_seid;
    int len = 0;

    ogs_debug("Session Establishment Response");

    rsp = &pfcp_message.pfcp_session_establishment_response;
    memset(&pfcp_message, 0, sizeof(ogs_pfcp_message_t));

    /* Node ID */
    ogs_pfcp_sockaddr_to_node_id(
            ogs_pfcp_self()->pfcp_addr, ogs_pfcp_self()->pfcp_addr6,
            ogs_config()->parameter.prefer_ipv4,
            &node_id, &len);
    rsp->node_id.presence = 1;
    rsp->node_id.data = &node_id;
    rsp->node_id.len = len;

    /* Cause */
    rsp->cause.presence = 1;
    rsp->cause.u8 = OGS_PFCP_CAUSE_REQUEST_ACCEPTED;

    /* F-SEID */
    ogs_pfcp_sockaddr_to_f_seid(
            ogs_pfcp_self()->pfcp_addr, ogs_pfcp_self()->pfcp_addr6,
            &f_seid, &len);
    f_seid.seid = htobe64(sess->sgwu_sxa_seid);
    rsp->up_f_seid.presence = 1;
    rsp->up_f_seid.data = &f_seid;
    rsp->up_f_seid.len = len;

    /* Created PDR */
    for (i = 0; i < num_of_created_pdr; i++) {
        ogs_pfcp_tlv_created_pdr_t *message = &rsp->created_pdr[i];
        ogs_assert(message);

        message->presence = 1;
        message->pdr_id.presence = 1;
        message->pdr_id.u16 = created_pdr[i]->id;
    }

    pfcp_message.h.type = type;
    return ogs_pfcp_build_msg(&pfcp_message);
}

ogs_pkbuf_t *sgwu_sxa_build_session_modification_response(uint8_t type,
    sgwu_sess_t *sess, ogs_pfcp_pdr_t *created_pdr[], int num_of_created_pdr)
{
    ogs_pfcp_message_t pfcp_message;
    ogs_pfcp_session_modification_response_t *rsp = NULL;

    int i = 0;

    ogs_debug("Session Modification Response");

    rsp = &pfcp_message.pfcp_session_modification_response;
    memset(&pfcp_message, 0, sizeof(ogs_pfcp_message_t));

    /* Cause */
    rsp->cause.presence = 1;
    rsp->cause.u8 = OGS_PFCP_CAUSE_REQUEST_ACCEPTED;

    /* Created PDR */
    for (i = 0; i < num_of_created_pdr; i++) {
        ogs_pfcp_tlv_created_pdr_t *message = &rsp->created_pdr[i];
        ogs_assert(message);

        message->presence = 1;
        message->pdr_id.presence = 1;
        message->pdr_id.u16 = created_pdr[i]->id;
    }

    pfcp_message.h.type = type;
    return ogs_pfcp_build_msg(&pfcp_message);
}

ogs_pkbuf_t *sgwu_sxa_build_session_deletion_response(uint8_t type,
        sgwu_sess_t *sess)
{
    ogs_pfcp_message_t pfcp_message;
    ogs_pfcp_session_deletion_response_t *rsp = NULL;

    ogs_debug("Session Deletion Response");

    rsp = &pfcp_message.pfcp_session_deletion_response;
    memset(&pfcp_message, 0, sizeof(ogs_pfcp_message_t));

    /* Cause */
    rsp->cause.presence = 1;
    rsp->cause.u8 = OGS_PFCP_CAUSE_REQUEST_ACCEPTED;

    pfcp_message.h.type = type;
    return ogs_pfcp_build_msg(&pfcp_message);
}
