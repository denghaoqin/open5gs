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

void testesm_handle_activate_default_eps_bearer_context_request(
        test_bearer_t *bearer,
        ogs_nas_eps_activate_default_eps_bearer_context_request_t
            *activate_default_eps_bearer_context_request)
{
    test_sess_t *sess = NULL;
    ogs_nas_pdn_address_t *pdn_address = NULL;

    ogs_assert(bearer);
    sess = bearer->sess;
    ogs_assert(sess);
    ogs_assert(activate_default_eps_bearer_context_request);

    pdn_address = &activate_default_eps_bearer_context_request->pdn_address;

    switch (pdn_address->pdn_type) {
    case OGS_PDU_SESSION_TYPE_IPV4:
        sess->ue_ip.ipv4 = 1;
        sess->ue_ip.addr = pdn_address->addr;
        break;
    case OGS_PDU_SESSION_TYPE_IPV6:
        break;
    case OGS_PDU_SESSION_TYPE_IPV4V6:
        sess->ue_ip.ipv4 = 1;
        sess->ue_ip.addr = pdn_address->both.addr;
        break;
    default:
        ogs_fatal("Invalid PDU Address Type [%d]", pdn_address->pdn_type);
        ogs_assert_if_reached();
    }
}
