/*
 * Copyright (C) 2019,2020 by Sukchan Lee <acetcom@gmail.com>
 *
 * This file is part of OpenEPS.
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

ogs_pkbuf_t *testemm_build_attach_request(
        test_ue_t *test_ue, ogs_pkbuf_t *nasbuf)
{
    int i;
    test_sess_t *sess = NULL;
    uint16_t psimask = 0;

    ogs_nas_eps_message_t message;
    ogs_pkbuf_t *pkbuf = NULL;
    ogs_nas_eps_attach_request_t *attach_request = &message.emm.attach_request;

    ogs_nas_eps_attach_type_t *eps_attach_type =
        &attach_request->eps_attach_type;
    ogs_nas_eps_mobile_identity_t *eps_mobile_identity =
        &attach_request->eps_mobile_identity;
    ogs_nas_ue_network_capability_t *ue_network_capability =
        &attach_request->ue_network_capability;
    ogs_nas_esm_message_container_t *esm_message_container =
        &attach_request->esm_message_container;

    ogs_assert(test_ue);
    sess = test_ue->sess;
    ogs_assert(sess);
    ogs_assert(nasbuf);

    memset(&message, 0, sizeof(message));
    if (test_ue->attach_request_param.integrity_protected) {
        if (test_ue->attach_request_param.ciphered)
            message.h.security_header_type =
                OGS_NAS_SECURITY_HEADER_INTEGRITY_PROTECTED_AND_CIPHERED;
        else
            message.h.security_header_type =
                OGS_NAS_SECURITY_HEADER_INTEGRITY_PROTECTED;
        message.h.protocol_discriminator =
            OGS_NAS_PROTOCOL_DISCRIMINATOR_EMM;
    }
    message.emm.h.protocol_discriminator =
            OGS_NAS_PROTOCOL_DISCRIMINATOR_EMM;
    message.emm.h.message_type = OGS_NAS_EPS_ATTACH_REQUEST;

    esm_message_container->length = nasbuf->len;
    esm_message_container->buffer = nasbuf->data;
    ogs_pkbuf_free(nasbuf);

    memcpy(&eps_attach_type, &test_ue->nas.data, sizeof(eps_attach_type));

    if (test_ue->attach_request_param.guti) {
        eps_mobile_identity->length =
            sizeof(ogs_nas_eps_mobile_identity_guti_t);
        eps_mobile_identity->guti.odd_even = OGS_NAS_MOBILE_IDENTITY_EVEN;
        eps_mobile_identity->guti.type = OGS_NAS_EPS_MOBILE_IDENTITY_GUTI;
        eps_mobile_identity->guti.nas_plmn_id =
            test_ue->nas_eps_guti.nas_plmn_id;
        eps_mobile_identity->guti.mme_gid = test_ue->nas_eps_guti.mme_gid;
        eps_mobile_identity->guti.mme_code = test_ue->nas_eps_guti.mme_code;
        eps_mobile_identity->guti.m_tmsi = test_ue->nas_eps_guti.m_tmsi;
    } else {
#if 0
        attach_request->mobile_identity.length =
            test_ue->mobile_identity_suci_length;
        attach_request->mobile_identity.buffer =
            &test_ue->mobile_identity_suci;
#endif
    }

    ue_network_capability->length = 7;
    ue_network_capability->eea = 0xf0;
    ue_network_capability->eia = 0xf0;
    ue_network_capability->uea = 0xc0;
    ue_network_capability->uia = 0x40;
    ue_network_capability->notification_procedure = 1;
    ue_network_capability->extended_protocol_configuration_options = 1;
    ue_network_capability->n1_mode = 1;
    ue_network_capability->dual_connectivity_with_nr = 1;

#if 0
    if (test_ue->attach_request_param.last_visited_registered_tai) {
        /* Set Last visited registered TAI */
        attach_request->presencemask |=
        OGS_NAS_EPS_ATTACH_REQUEST_LAST_VISITED_REGISTERED_TAI_PRESENT;
        ogs_nas_from_plmn_id(&last_visited_registered_tai->nas_plmn_id,
                &test_self()->nr_tai.plmn_id);
        last_visited_registered_tai->tac.v = test_self()->nr_tai.tac.v;
    }

    if (test_ue->attach_request_param.ue_usage_setting) {
        /* Set UE's usage setting */
        attach_request->presencemask |=
            OGS_NAS_EPS_ATTACH_REQUEST_UE_USAGE_SETTING_PRESENT;
        ue_usage_setting->length = 1;
        ue_usage_setting->data_centric = 1;
    }

    if (test_ue->attach_request_param.update_type) {
        /* Set EPS update type */
        attach_request->presencemask |=
            OGS_NAS_EPS_ATTACH_REQUEST_EPS_UPDATE_TYPE_PRESENT;
        update_type->length = 1;
        update_type->sms_over_nas_supported = 1;
    }
#endif

#if 0
    if (test_ue->attach_request_param.integrity_protected)
        return test_nas_eps_security_encode(test_ue, &message);
    else
        return ogs_nas_eps_plain_encode(&message);
#else
    return ogs_nas_eps_plain_encode(&message);
#endif
}
