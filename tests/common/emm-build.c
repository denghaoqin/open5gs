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
        test_ue_t *test_ue, ogs_pkbuf_t *esmbuf)
{
    int i;
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
    ogs_assert(esmbuf);

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

    esm_message_container->length = esmbuf->len;
    esm_message_container->buffer = esmbuf->data;
    ogs_pkbuf_free(esmbuf);

    memcpy(eps_attach_type, &test_ue->nas.data, sizeof(*eps_attach_type));

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
        eps_mobile_identity->length =
            sizeof(ogs_nas_mobile_identity_imsi_t);
        memcpy(&eps_mobile_identity->imsi,
                &test_ue->mobile_identity_imsi, eps_mobile_identity->length);
    }

    ue_network_capability->length = 7;
    ue_network_capability->eea = 0xf0;
    ue_network_capability->eia = 0xf0;
    ue_network_capability->uea = 0xc0;
    ue_network_capability->uia = 0x40;
    ue_network_capability->notification_procedure = 1;
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

ogs_pkbuf_t *testemm_build_identity_response(test_ue_t *test_ue)
{
    ogs_nas_eps_message_t message;
    ogs_pkbuf_t *pkbuf = NULL;
    ogs_nas_eps_identity_response_t *identity_response =
            &message.emm.identity_response;

    ogs_assert(test_ue);

    memset(&message, 0, sizeof(message));
    message.emm.h.protocol_discriminator =
            OGS_NAS_PROTOCOL_DISCRIMINATOR_EMM;
    message.emm.h.message_type = OGS_NAS_EPS_IDENTITY_RESPONSE;

    identity_response->mobile_identity.length =
            sizeof(ogs_nas_mobile_identity_imsi_t);
    memcpy(&identity_response->mobile_identity.imsi,
            &test_ue->mobile_identity_imsi,
            identity_response->mobile_identity.length);

    return ogs_nas_eps_plain_encode(&message);
}

ogs_pkbuf_t *testemm_build_authentication_response(test_ue_t *test_ue)
{
    ogs_nas_eps_message_t message;
    ogs_pkbuf_t *pkbuf = NULL;
    ogs_nas_eps_authentication_response_t *authentication_response =
            &message.emm.authentication_response;
    ogs_nas_authentication_response_parameter_t
        *authentication_response_parameter =
            &authentication_response->authentication_response_parameter;

    uint8_t ik[OGS_KEY_LEN];
    uint8_t ck[OGS_KEY_LEN];
    uint8_t ak[OGS_AK_LEN];
    uint8_t sqn[OGS_SQN_LEN];
    uint8_t res[OGS_MAX_RES_LEN];

    uint8_t kasme[32];

    int i;

    ogs_assert(test_ue);

    memset(&message, 0, sizeof(message));
    message.emm.h.protocol_discriminator = OGS_NAS_PROTOCOL_DISCRIMINATOR_EMM;
    message.emm.h.message_type = OGS_NAS_EPS_AUTHENTICATION_RESPONSE;

    milenage_f2345(test_ue->opc, test_ue->k, test_ue->rand,
            res, ck, ik, ak, NULL);

	for (i = 0; i < 6; i++)
		sqn[i] = test_ue->autn[i] ^ ak[i];

    ogs_auc_kasme(ck, ik, &test_ue->e_tai.plmn_id, sqn, ak, test_ue->kasme);

    authentication_response_parameter->length = 8;
    memcpy(authentication_response_parameter->res, res, 8);

    return ogs_nas_eps_plain_encode(&message);
}

ogs_pkbuf_t *testemm_build_security_mode_complete(test_ue_t *test_ue)
{
    ogs_nas_eps_message_t message;
    ogs_pkbuf_t *pkbuf = NULL;
    ogs_nas_eps_security_mode_complete_t *security_mode_complete =
            &message.emm.security_mode_complete;
    ogs_nas_mobile_identity_t *imeisv = &security_mode_complete->imeisv;

    ogs_nas_mobile_identity_imeisv_t mobile_identity_imeisv;

    ogs_assert(test_ue);

    memset(&message, 0, sizeof(message));
    message.h.security_header_type =
        OGS_NAS_SECURITY_HEADER_INTEGRITY_PROTECTED_AND_CIPHTERD_WITH_NEW_INTEGRITY_CONTEXT;
    message.h.protocol_discriminator = OGS_NAS_PROTOCOL_DISCRIMINATOR_EMM;

    message.emm.h.protocol_discriminator = OGS_NAS_PROTOCOL_DISCRIMINATOR_EMM;
    message.emm.h.message_type = OGS_NAS_EPS_SECURITY_MODE_COMPLETE;

    if (test_ue->mobile_identity_imeisv.type == OGS_NAS_MOBILE_IDENTITY_IMEISV) {
        security_mode_complete->presencemask |=
            OGS_NAS_EPS_SECURITY_MODE_COMPLETE_IMEISV_PRESENT;
        memset(&mobile_identity_imeisv, 0, sizeof(mobile_identity_imeisv));
        imeisv->length = sizeof(mobile_identity_imeisv);
        memcpy(&imeisv->imeisv, &test_ue->mobile_identity_imeisv, imeisv->length);
    }

    return test_nas_eps_security_encode(test_ue, &message);
}
