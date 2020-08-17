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
 * alos1 with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "test-common.h"

void tests1ap_handle_s1_setup_response(ogs_s1ap_message_t *message)
{
    S1AP_SuccessfulOutcome_t *successfulOutcome = NULL;
    S1AP_S1SetupResponse_t *S1SetupResponse = NULL;

    S1AP_S1SetupResponseIEs_t *ie = NULL;

    ogs_assert(message);

    successfulOutcome = message->choice.successfulOutcome;
    ogs_assert(successfulOutcome);
    S1SetupResponse = &successfulOutcome->value.choice.S1SetupResponse;
    ogs_assert(S1SetupResponse);

    ogs_debug("S1 setup response");
}

void tests1ap_handle_downlink_nas_transport(
        test_ue_t *test_ue, ogs_s1ap_message_t *message)
{
    int i;

    S1AP_S1AP_PDU_t pdu;
    S1AP_InitiatingMessage_t *initiatingMessage = NULL;
    S1AP_DownlinkNASTransport_t *DownlinkNASTransport = NULL;

    S1AP_DownlinkNASTransport_IEs_t *ie = NULL;
    S1AP_MME_UE_S1AP_ID_t *MME_UE_S1AP_ID = NULL;
    S1AP_NAS_PDU_t *NAS_PDU = NULL;

    ogs_assert(test_ue);
    ogs_assert(message);

    initiatingMessage = message->choice.initiatingMessage;
    ogs_assert(initiatingMessage);
    DownlinkNASTransport =
        &initiatingMessage->value.choice.DownlinkNASTransport;
    ogs_assert(DownlinkNASTransport);

    for (i = 0; i < DownlinkNASTransport->protocolIEs.list.count; i++) {
        ie = DownlinkNASTransport->protocolIEs.list.array[i];
        switch (ie->id) {
        case S1AP_ProtocolIE_ID_id_MME_UE_S1AP_ID:
            MME_UE_S1AP_ID = &ie->value.choice.MME_UE_S1AP_ID;
            break;
        case S1AP_ProtocolIE_ID_id_NAS_PDU:
            NAS_PDU = &ie->value.choice.NAS_PDU;
            break;
        default:
            break;
        }
    }

    if (MME_UE_S1AP_ID)
        test_ue->mme_ue_s1ap_id = *MME_UE_S1AP_ID;

    if (NAS_PDU)
        tests1ap_send_to_nas(test_ue, NAS_PDU);
}

void tests1ap_handle_initial_context_setup_request(
        test_ue_t *test_ue, ogs_s1ap_message_t *message)
{
    int i, rv;
    test_sess_t *sess = NULL;
    test_bearer_t *bearer = NULL;

    S1AP_S1AP_PDU_t pdu;
    S1AP_InitiatingMessage_t *initiatingMessage = NULL;
    S1AP_InitialContextSetupRequest_t *InitialContextSetupRequest = NULL;

    S1AP_InitialContextSetupRequestIEs_t *ie = NULL;
    S1AP_MME_UE_S1AP_ID_t *MME_UE_S1AP_ID = NULL;
    S1AP_NAS_PDU_t *NAS_PDU = NULL;
    S1AP_E_RABToBeSetupListCtxtSUReq_t *E_RABToBeSetupListCtxtSUReq = NULL;

    ogs_assert(test_ue);
    ogs_assert(message);

    initiatingMessage = message->choice.initiatingMessage;
    ogs_assert(initiatingMessage);
    InitialContextSetupRequest =
        &initiatingMessage->value.choice.InitialContextSetupRequest;
    ogs_assert(InitialContextSetupRequest);

    for (i = 0; i < InitialContextSetupRequest->protocolIEs.list.count; i++) {
        ie = InitialContextSetupRequest->protocolIEs.list.array[i];
        switch (ie->id) {
        case S1AP_ProtocolIE_ID_id_MME_UE_S1AP_ID:
            MME_UE_S1AP_ID = &ie->value.choice.MME_UE_S1AP_ID;
            break;
        case S1AP_ProtocolIE_ID_id_E_RABToBeSetupListCtxtSUReq:
            E_RABToBeSetupListCtxtSUReq =
                &ie->value.choice.E_RABToBeSetupListCtxtSUReq;
            break;
        default:
            break;
        }
    }

    if (MME_UE_S1AP_ID)
        test_ue->mme_ue_s1ap_id = *MME_UE_S1AP_ID;

    for (i = 0; i < E_RABToBeSetupListCtxtSUReq->list.count; i++) {
        S1AP_E_RABToBeSetupItemCtxtSUReqIEs_t *ie2 = NULL;
        S1AP_E_RABToBeSetupItemCtxtSUReq_t *e_rab = NULL;

        ie2 = (S1AP_E_RABToBeSetupItemCtxtSUReqIEs_t *)
                E_RABToBeSetupListCtxtSUReq->list.array[i];
        ogs_assert(ie2);
        e_rab = &ie2->value.choice.E_RABToBeSetupItemCtxtSUReq;

        sess = ogs_list_first(&test_ue->sess_list);
        ogs_assert(sess);

        bearer = ogs_list_first(&sess->bearer_list);
        ogs_assert(bearer);
        bearer->ebi = e_rab->e_RAB_ID;

        memcpy(&bearer->sgw_s1u_teid, e_rab->gTP_TEID.buf, 
                sizeof(bearer->sgw_s1u_teid));
        bearer->enb_s1u_teid = be32toh(bearer->enb_s1u_teid);
        rv = ogs_asn_BIT_STRING_to_ip(
                &e_rab->transportLayerAddress, &bearer->sgw_s1u_ip);
        ogs_assert(rv == OGS_OK);
        if (e_rab->nAS_PDU)
            tests1ap_send_to_nas(test_ue, e_rab->nAS_PDU);
    }
}
