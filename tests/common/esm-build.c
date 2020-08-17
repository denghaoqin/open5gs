/*
 * Copyright (C) 2019,2020 by Sukchan Lee <acetcom@gmail.com>
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

#include "test-common.h"

ogs_pkbuf_t *testesm_build_pdn_connectivity_request(test_sess_t *sess)
{
    ogs_nas_eps_message_t message;
    ogs_nas_eps_pdn_connectivity_request_t *pdn_connectivity_request =
        &message.esm.pdn_connectivity_request;
    ogs_nas_request_type_t *request_type =
        &pdn_connectivity_request->request_type;
    ogs_nas_protocol_configuration_options_t *protocol_configuration_options =
        &pdn_connectivity_request->protocol_configuration_options;
    uint8_t ue_pco[29] =
            "\x80\x80\x21\x10\x01\x01\x00\x10\x81\x06\x00\x00\x00\x00"
            "\x83\x06\x00\x00\x00\x00\x00\x03\x00\x00\x0a\x00\x00\x0d\x00";

    test_ue_t *test_ue = NULL;
    ogs_pkbuf_t *pkbuf = NULL;

    ogs_assert(sess);
    test_ue = sess->test_ue;
    ogs_assert(test_ue);

    memset(&message, 0, sizeof(message));

    message.esm.h.eps_bearer_identity = 0;
    message.esm.h.protocol_discriminator = OGS_NAS_PROTOCOL_DISCRIMINATOR_ESM;
    message.esm.h.procedure_transaction_identity = sess->pti;
    message.esm.h.message_type = OGS_NAS_EPS_PDN_CONNECTIVITY_REQUEST;

    request_type->type = OGS_NAS_EPS_PDN_TYPE_IPV4V6;
    request_type->value = OGS_NAS_5GS_REQUEST_TYPE_INITIAL;

    pdn_connectivity_request->presencemask |= OGS_NAS_EPS_PDN_CONNECTIVITY_REQUEST_PROTOCOL_CONFIGURATION_OPTIONS_PRESENT;
    protocol_configuration_options->length = sizeof(ue_pco);
    memcpy(protocol_configuration_options->buffer, ue_pco, sizeof(ue_pco));

    return ogs_nas_eps_plain_encode(&message);
}

ogs_pkbuf_t *testesm_build_activate_default_eps_bearer_context_accept(
        test_ue_t *test_ue)
{
    ogs_nas_eps_message_t message;
    ogs_nas_eps_activate_default_eps_bearer_context_accept_t
        *activate_default_eps_bearer_context_accept =
            &message.esm.activate_default_eps_bearer_context_accept;

    test_sess_t *sess = NULL;
    test_bearer_t *bearer = NULL;
    ogs_pkbuf_t *pkbuf = NULL;

    ogs_assert(test_ue);
    sess = ogs_list_first(&test_ue->sess_list);
    ogs_assert(sess);
    bearer = ogs_list_first(&sess->bearer_list);
    ogs_assert(bearer);

    memset(&message, 0, sizeof(message));

    message.esm.h.eps_bearer_identity = bearer->ebi;
    message.esm.h.protocol_discriminator = OGS_NAS_PROTOCOL_DISCRIMINATOR_ESM;
    message.esm.h.message_type =
        OGS_NAS_EPS_ACTIVATE_DEFAULT_EPS_BEARER_CONTEXT_ACCEPT;

    return ogs_nas_eps_plain_encode(&message);
}
