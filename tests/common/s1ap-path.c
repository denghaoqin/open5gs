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

void tests1ap_recv(test_ue_t *test_ue, ogs_pkbuf_t *pkbuf)
{
    int rv;
    ogs_s1ap_message_t message;

    S1AP_S1AP_PDU_t *pdu = NULL;
    S1AP_InitiatingMessage_t *initiatingMessage = NULL;
    S1AP_SuccessfulOutcome_t *successfulOutcome = NULL;
    S1AP_UnsuccessfulOutcome_t *unsuccessfulOutcome = NULL;

    ogs_assert(pkbuf);

    rv = ogs_s1ap_decode(&message, pkbuf);
    ogs_assert(rv == OGS_OK);

    pdu = &message;
    ogs_assert(pdu);

    switch (pdu->present) {
    case S1AP_S1AP_PDU_PR_initiatingMessage:
        initiatingMessage = pdu->choice.initiatingMessage;
        ogs_assert(initiatingMessage);

        switch (initiatingMessage->procedureCode) {
        case S1AP_ProcedureCode_id_downlinkNASTransport:
#if 0
            tests1ap_handle_downlink_nas_transport(test_ue, pdu);
#endif
            break;
        case S1AP_ProcedureCode_id_InitialContextSetup:
#if 0
            tests1ap_handle_initial_context_setup_request(test_ue, pdu);
#endif
            break;
        case S1AP_ProcedureCode_id_UEContextRelease:
            /* Nothing */
            break;
        case S1AP_ProcedureCode_id_ErrorIndication:
            /* Nothing */
            break;
        default:
            ogs_error("Not implemented(choice:%d, proc:%d)",
                    pdu->present, (int)initiatingMessage->procedureCode);
            break;
        }
        break;
    case S1AP_S1AP_PDU_PR_successfulOutcome :
        successfulOutcome = pdu->choice.successfulOutcome;
        ogs_assert(successfulOutcome);

        switch (successfulOutcome->procedureCode) {
        case S1AP_ProcedureCode_id_S1Setup:
            tests1ap_handle_s1_setup_response(pdu);
            break;
        default:
            ogs_error("Not implemented(choice:%d, proc:%d)",
                    pdu->present, (int)successfulOutcome->procedureCode);
            break;
        }
        break;
    case S1AP_S1AP_PDU_PR_unsuccessfulOutcome :
        unsuccessfulOutcome = pdu->choice.unsuccessfulOutcome;
        ogs_assert(unsuccessfulOutcome);

        switch (unsuccessfulOutcome->procedureCode) {
        case S1AP_ProcedureCode_id_S1Setup:
            break;
        default:
            ogs_error("Not implemented(choice:%d, proc:%d)",
                    pdu->present, (int)unsuccessfulOutcome->procedureCode);
            break;
        }
        break;
    default:
        ogs_error("Not implemented(choice:%d)", pdu->present);
        break;
    }

    ogs_s1ap_free(&message);
    ogs_pkbuf_free(pkbuf);
}

#if 0
void tests1ap_send_to_nas(test_ue_t *test_ue, S1AP_NAS_PDU_t *nasPdu)
{
    ogs_nas_5gs_security_header_t *sh = NULL;
    ogs_nas_security_header_type_t security_header_type;

    ogs_nas_5gmm_header_t *h = NULL;
    ogs_pkbuf_t *nasbuf = NULL;

    ogs_assert(test_ue);
    ogs_assert(nasPdu);

    /* The Packet Buffer(pkbuf_t) for NAS message MUST make a HEADROOM. 
     * When calculating AES_CMAC, we need to use the headroom of the packet. */
    nasbuf = ogs_pkbuf_alloc(NULL, OGS_NAS_HEADROOM+nasPdu->size);
    ogs_pkbuf_reserve(nasbuf, OGS_NAS_HEADROOM);
    ogs_pkbuf_put_data(nasbuf, nasPdu->buf, nasPdu->size);

    sh = (ogs_nas_5gs_security_header_t *)nasbuf->data;
    ogs_assert(sh);

    memset(&security_header_type, 0, sizeof(ogs_nas_security_header_type_t));
    switch(sh->security_header_type) {
    case OGS_NAS_SECURITY_HEADER_PLAIN_NAS_MESSAGE:
        break;
    case OGS_NAS_SECURITY_HEADER_INTEGRITY_PROTECTED:
        security_header_type.integrity_protected = 1;
        ogs_pkbuf_pull(nasbuf, 7);
        break;
    case OGS_NAS_SECURITY_HEADER_INTEGRITY_PROTECTED_AND_CIPHERED:
        security_header_type.integrity_protected = 1;
        security_header_type.ciphered = 1;
        ogs_pkbuf_pull(nasbuf, 7);
        break;
    case OGS_NAS_SECURITY_HEADER_INTEGRITY_PROTECTED_AND_NEW_SECURITY_CONTEXT:
        security_header_type.integrity_protected = 1;
        security_header_type.new_security_context = 1;
        ogs_pkbuf_pull(nasbuf, 7);
        break;
    case OGS_NAS_SECURITY_HEADER_INTEGRITY_PROTECTED_AND_CIPHTERD_WITH_NEW_INTEGRITY_CONTEXT:
        security_header_type.integrity_protected = 1;
        security_header_type.ciphered = 1;
        security_header_type.new_security_context = 1;
        ogs_pkbuf_pull(nasbuf, 7);
        break;
    default:
        ogs_error("Not implemented(security header type:0x%x)",
                sh->security_header_type);
        ogs_assert_if_reached();
    }

    h = (ogs_nas_5gmm_header_t *)nasbuf->data;
    ogs_assert(h);

    if (h->message_type == OGS_NAS_5GS_SECURITY_MODE_COMMAND) {
        ogs_nas_5gs_message_t message;
        int rv;

        rv = ogs_nas_5gmm_decode(&message, nasbuf);
        ogs_assert(rv == OGS_OK);

        testgmm_handle_security_mode_command(test_ue,
                &message.gmm.security_mode_command);
    }

    if (test_nas_5gs_security_decode(test_ue,
            security_header_type, nasbuf) != OGS_OK) {
        ogs_error("nas_eps_security_decode failed()");
        ogs_assert_if_reached();
    }

    if (h->extended_protocol_discriminator ==
            OGS_NAS_EXTENDED_PROTOCOL_DISCRIMINATOR_5GMM) {
        testgmm_recv(test_ue, nasbuf);
    } else {
        ogs_error("Unknown NAS Protocol discriminator 0x%02x",
                  h->extended_protocol_discriminator);
        ogs_assert_if_reached();
    }
}
#endif
