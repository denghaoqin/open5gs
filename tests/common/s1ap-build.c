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

ogs_pkbuf_t *test_s1ap_build_s1_setup_request(
        S1AP_ENB_ID_PR present, uint32_t enb_id,
        int tac, uint16_t mcc, uint16_t mnc, uint16_t mnc_len)
{
    ogs_plmn_id_t plmn_id;

    S1AP_S1AP_PDU_t pdu;
    S1AP_InitiatingMessage_t *initiatingMessage = NULL;
    S1AP_S1SetupRequest_t *S1SetupRequest = NULL;

    S1AP_S1SetupRequestIEs_t *ie = NULL;
    S1AP_Global_ENB_ID_t *Global_ENB_ID = NULL;
    S1AP_SupportedTAs_t *SupportedTAs = NULL;
    S1AP_SupportedTAs_Item_t *SupportedTAs_Item = NULL;
    S1AP_PLMNidentity_t *PLMNidentity = NULL;
    S1AP_PagingDRX_t *PagingDRX = NULL;

    memset(&pdu, 0, sizeof (S1AP_S1AP_PDU_t));
    pdu.present = S1AP_S1AP_PDU_PR_initiatingMessage;
    pdu.choice.initiatingMessage =
        CALLOC(1, sizeof(S1AP_InitiatingMessage_t));

    initiatingMessage = pdu.choice.initiatingMessage;
    initiatingMessage->procedureCode = S1AP_ProcedureCode_id_S1Setup;
    initiatingMessage->criticality = S1AP_Criticality_reject;
    initiatingMessage->value.present =
        S1AP_InitiatingMessage__value_PR_S1SetupRequest;

    S1SetupRequest = &initiatingMessage->value.choice.S1SetupRequest;

    ie = CALLOC(1, sizeof(S1AP_S1SetupRequestIEs_t));
    ASN_SEQUENCE_ADD(&S1SetupRequest->protocolIEs, ie);

    ie->id = S1AP_ProtocolIE_ID_id_Global_ENB_ID;
    ie->criticality = S1AP_Criticality_reject;
    ie->value.present = S1AP_S1SetupRequestIEs__value_PR_Global_ENB_ID;

    Global_ENB_ID = &ie->value.choice.Global_ENB_ID;

    ie = CALLOC(1, sizeof(S1AP_S1SetupRequestIEs_t));
    ASN_SEQUENCE_ADD(&S1SetupRequest->protocolIEs, ie);

    ie->id = S1AP_ProtocolIE_ID_id_SupportedTAs;
    ie->criticality = S1AP_Criticality_reject;
    ie->value.present = S1AP_S1SetupRequestIEs__value_PR_SupportedTAs;

    SupportedTAs = &ie->value.choice.SupportedTAs;

    ie = CALLOC(1, sizeof(S1AP_S1SetupRequestIEs_t));
    ASN_SEQUENCE_ADD(&S1SetupRequest->protocolIEs, ie);
    
    ie->id = S1AP_ProtocolIE_ID_id_DefaultPagingDRX;
    ie->criticality = S1AP_Criticality_ignore;
    ie->value.present = S1AP_S1SetupRequestIEs__value_PR_PagingDRX;

    PagingDRX = &ie->value.choice.PagingDRX;

    ogs_plmn_id_build(&plmn_id, mcc, mnc, mnc_len);

    ogs_s1ap_uint32_to_ENB_ID(present, enb_id, &Global_ENB_ID->eNB_ID);
    ogs_s1ap_buffer_to_OCTET_STRING(
            &plmn_id, OGS_PLMN_ID_LEN, &Global_ENB_ID->pLMNidentity);

    SupportedTAs_Item = (S1AP_SupportedTAs_Item_t *)
        CALLOC(1, sizeof(S1AP_SupportedTAs_Item_t));
    ogs_asn_uint16_to_OCTET_STRING(tac, &SupportedTAs_Item->tAC);
    PLMNidentity = (S1AP_PLMNidentity_t *)
        CALLOC(1, sizeof(S1AP_PLMNidentity_t));
    ogs_s1ap_buffer_to_OCTET_STRING(
            &plmn_id, OGS_PLMN_ID_LEN, PLMNidentity);
    ASN_SEQUENCE_ADD(&SupportedTAs_Item->broadcastPLMNs.list, PLMNidentity);

    ASN_SEQUENCE_ADD(&SupportedTAs->list, SupportedTAs_Item);

    *PagingDRX = S1AP_PagingDRX_v64;

    return ogs_s1ap_encode(&pdu);
}

ogs_pkbuf_t *test_s1ap_build_initial_ue_message(
        test_ue_t *test_ue, ogs_pkbuf_t *emmbuf, bool s_tmsi)
{
    ogs_pkbuf_t *pkbuf = NULL;
    int i, j;
    char buf[5];

    S1AP_S1AP_PDU_t pdu;
    S1AP_InitiatingMessage_t *initiatingMessage = NULL;
    S1AP_InitialUEMessage_t *InitialUEMessage = NULL;

    S1AP_InitialUEMessage_IEs_t *ie = NULL;
    S1AP_ENB_UE_S1AP_ID_t *ENB_UE_S1AP_ID = NULL;
    S1AP_NAS_PDU_t *NAS_PDU = NULL;
    S1AP_TAI_t *TAI = NULL;
    S1AP_EUTRAN_CGI_t *EUTRAN_CGI = NULL;
    S1AP_RRC_Establishment_Cause_t *RRC_Establishment_Cause = NULL;

    ogs_assert(test_ue);
    ogs_assert(emmbuf);

    memset(&pdu, 0, sizeof (S1AP_S1AP_PDU_t));
    pdu.present = S1AP_S1AP_PDU_PR_initiatingMessage;
    pdu.choice.initiatingMessage =
        CALLOC(1, sizeof(S1AP_InitiatingMessage_t));

    initiatingMessage = pdu.choice.initiatingMessage;
    initiatingMessage->procedureCode = S1AP_ProcedureCode_id_initialUEMessage;
    initiatingMessage->criticality = S1AP_Criticality_reject;
    initiatingMessage->value.present =
        S1AP_InitiatingMessage__value_PR_InitialUEMessage;

    InitialUEMessage = &initiatingMessage->value.choice.InitialUEMessage;

    ie = CALLOC(1, sizeof(S1AP_InitialUEMessage_IEs_t));
    ASN_SEQUENCE_ADD(&InitialUEMessage->protocolIEs, ie);

    ie->id = S1AP_ProtocolIE_ID_id_eNB_UE_S1AP_ID;
    ie->criticality = S1AP_Criticality_reject;
    ie->value.present = S1AP_InitialUEMessage_IEs__value_PR_ENB_UE_S1AP_ID;

    ENB_UE_S1AP_ID = &ie->value.choice.ENB_UE_S1AP_ID;

    test_ue->enb_ue_s1ap_id++;
    *ENB_UE_S1AP_ID = test_ue->enb_ue_s1ap_id;

    ie = CALLOC(1, sizeof(S1AP_InitialUEMessage_IEs_t));
    ASN_SEQUENCE_ADD(&InitialUEMessage->protocolIEs, ie);

    ie->id = S1AP_ProtocolIE_ID_id_NAS_PDU;
    ie->criticality = S1AP_Criticality_reject;
    ie->value.present = S1AP_InitialUEMessage_IEs__value_PR_NAS_PDU;

    NAS_PDU = &ie->value.choice.NAS_PDU;

    NAS_PDU->size = emmbuf->len;
    NAS_PDU->buf = CALLOC(NAS_PDU->size, sizeof(uint8_t));
    memcpy(NAS_PDU->buf, emmbuf->data, NAS_PDU->size);
    ogs_pkbuf_free(emmbuf);

    ie = CALLOC(1, sizeof(S1AP_InitialUEMessage_IEs_t));
    ASN_SEQUENCE_ADD(&InitialUEMessage->protocolIEs, ie);

    ie->id = S1AP_ProtocolIE_ID_id_TAI;
    ie->criticality = S1AP_Criticality_reject;
    ie->value.present = S1AP_InitialUEMessage_IEs__value_PR_TAI;

    TAI = &ie->value.choice.TAI;

    ogs_asn_uint16_to_OCTET_STRING(
            test_ue->e_tai.tac, &TAI->tAC);
    ogs_s1ap_buffer_to_OCTET_STRING(
            &test_ue->e_tai.plmn_id, OGS_PLMN_ID_LEN, &TAI->pLMNidentity);

    ie = CALLOC(1, sizeof(S1AP_InitialUEMessage_IEs_t));
    ASN_SEQUENCE_ADD(&InitialUEMessage->protocolIEs, ie);

    ie->id = S1AP_ProtocolIE_ID_id_EUTRAN_CGI;
    ie->criticality = S1AP_Criticality_ignore;
    ie->value.present = S1AP_InitialUEMessage_IEs__value_PR_EUTRAN_CGI;

    EUTRAN_CGI = &ie->value.choice.EUTRAN_CGI;

    ogs_s1ap_buffer_to_OCTET_STRING(
            &test_ue->e_cgi.plmn_id,
            OGS_PLMN_ID_LEN, &EUTRAN_CGI->pLMNidentity);
    EUTRAN_CGI->cell_ID.size = 4;
    EUTRAN_CGI->cell_ID.buf =  CALLOC(
         EUTRAN_CGI->cell_ID.size, sizeof(uint8_t));
    ogs_assert(EUTRAN_CGI->cell_ID.buf);
    EUTRAN_CGI->cell_ID.buf[0] = (test_ue->e_cgi.cell_id >> 24);
    EUTRAN_CGI->cell_ID.buf[1] = (test_ue->e_cgi.cell_id >> 16);
    EUTRAN_CGI->cell_ID.buf[2] = (test_ue->e_cgi.cell_id >> 8);
    EUTRAN_CGI->cell_ID.buf[3] = (test_ue->e_cgi.cell_id);
    EUTRAN_CGI->cell_ID.bits_unused = 4;

    ie = CALLOC(1, sizeof(S1AP_InitialUEMessage_IEs_t));
    ASN_SEQUENCE_ADD(&InitialUEMessage->protocolIEs, ie);

    ie->id = S1AP_ProtocolIE_ID_id_RRC_Establishment_Cause;
    ie->criticality = S1AP_Criticality_ignore;
    ie->value.present =
        S1AP_InitialUEMessage_IEs__value_PR_RRC_Establishment_Cause;

    RRC_Establishment_Cause = &ie->value.choice.RRC_Establishment_Cause;

    *RRC_Establishment_Cause = S1AP_RRC_Establishment_Cause_mo_Signalling;

    return ogs_s1ap_encode(&pdu);
}

ogs_pkbuf_t *test_s1ap_build_uplink_nas_transport(
        test_ue_t *test_ue, ogs_pkbuf_t *emmbuf)
{
    S1AP_S1AP_PDU_t pdu;
    S1AP_InitiatingMessage_t *initiatingMessage = NULL;
    S1AP_UplinkNASTransport_t *UplinkNASTransport = NULL;

    S1AP_UplinkNASTransport_IEs_t *ie = NULL;
    S1AP_MME_UE_S1AP_ID_t *MME_UE_S1AP_ID = NULL;
    S1AP_ENB_UE_S1AP_ID_t *ENB_UE_S1AP_ID = NULL;
    S1AP_NAS_PDU_t *NAS_PDU = NULL;
    S1AP_TAI_t *TAI = NULL;
    S1AP_EUTRAN_CGI_t *EUTRAN_CGI = NULL;

    ogs_assert(test_ue);
    ogs_assert(emmbuf);

    memset(&pdu, 0, sizeof (S1AP_S1AP_PDU_t));
    pdu.present = S1AP_S1AP_PDU_PR_initiatingMessage;
    pdu.choice.initiatingMessage =
        CALLOC(1, sizeof(S1AP_InitiatingMessage_t));

    initiatingMessage = pdu.choice.initiatingMessage;
    initiatingMessage->procedureCode =
        S1AP_ProcedureCode_id_uplinkNASTransport;
    initiatingMessage->criticality = S1AP_Criticality_ignore;
    initiatingMessage->value.present =
        S1AP_InitiatingMessage__value_PR_UplinkNASTransport;

    UplinkNASTransport = &initiatingMessage->value.choice.UplinkNASTransport;

    ie = CALLOC(1, sizeof(S1AP_UplinkNASTransport_IEs_t));
    ASN_SEQUENCE_ADD(&UplinkNASTransport->protocolIEs, ie);

    ie->id = S1AP_ProtocolIE_ID_id_MME_UE_S1AP_ID;
    ie->criticality = S1AP_Criticality_reject;
    ie->value.present =
        S1AP_UplinkNASTransport_IEs__value_PR_MME_UE_S1AP_ID;

    MME_UE_S1AP_ID = &ie->value.choice.MME_UE_S1AP_ID;

    *MME_UE_S1AP_ID = test_ue->mme_ue_s1ap_id;

    ie = CALLOC(1, sizeof(S1AP_UplinkNASTransport_IEs_t));
    ASN_SEQUENCE_ADD(&UplinkNASTransport->protocolIEs, ie);

    ie->id = S1AP_ProtocolIE_ID_id_eNB_UE_S1AP_ID;
    ie->criticality = S1AP_Criticality_reject;
    ie->value.present =
        S1AP_UplinkNASTransport_IEs__value_PR_ENB_UE_S1AP_ID;

    ENB_UE_S1AP_ID = &ie->value.choice.ENB_UE_S1AP_ID;

    *ENB_UE_S1AP_ID = test_ue->enb_ue_s1ap_id;

    ie = CALLOC(1, sizeof(S1AP_UplinkNASTransport_IEs_t));
    ASN_SEQUENCE_ADD(&UplinkNASTransport->protocolIEs, ie);

    ie->id = S1AP_ProtocolIE_ID_id_NAS_PDU;
    ie->criticality = S1AP_Criticality_reject;
    ie->value.present = S1AP_UplinkNASTransport_IEs__value_PR_NAS_PDU;

    NAS_PDU = &ie->value.choice.NAS_PDU;

    NAS_PDU->size = emmbuf->len;
    NAS_PDU->buf = CALLOC(NAS_PDU->size, sizeof(uint8_t));
    memcpy(NAS_PDU->buf, emmbuf->data, NAS_PDU->size);
    ogs_pkbuf_free(emmbuf);

    ie = CALLOC(1, sizeof(S1AP_UplinkNASTransport_IEs_t));
    ASN_SEQUENCE_ADD(&UplinkNASTransport->protocolIEs, ie);

    ie->id = S1AP_ProtocolIE_ID_id_TAI;
    ie->criticality = S1AP_Criticality_reject;
    ie->value.present = S1AP_UplinkNASTransport_IEs__value_PR_TAI;

    TAI = &ie->value.choice.TAI;

    ogs_asn_uint16_to_OCTET_STRING(
            test_ue->e_tai.tac, &TAI->tAC);
    ogs_s1ap_buffer_to_OCTET_STRING(
            &test_ue->e_tai.plmn_id, OGS_PLMN_ID_LEN, &TAI->pLMNidentity);

    ie = CALLOC(1, sizeof(S1AP_UplinkNASTransport_IEs_t));
    ASN_SEQUENCE_ADD(&UplinkNASTransport->protocolIEs, ie);

    ie->id = S1AP_ProtocolIE_ID_id_EUTRAN_CGI;
    ie->criticality = S1AP_Criticality_ignore;
    ie->value.present = S1AP_UplinkNASTransport_IEs__value_PR_EUTRAN_CGI;

    EUTRAN_CGI = &ie->value.choice.EUTRAN_CGI;

    ogs_s1ap_buffer_to_OCTET_STRING(
            &test_ue->e_cgi.plmn_id,
            OGS_PLMN_ID_LEN, &EUTRAN_CGI->pLMNidentity);
    EUTRAN_CGI->cell_ID.size = 4;
    EUTRAN_CGI->cell_ID.buf =  CALLOC(
         EUTRAN_CGI->cell_ID.size, sizeof(uint8_t));
    ogs_assert(EUTRAN_CGI->cell_ID.buf);
    EUTRAN_CGI->cell_ID.buf[0] = (test_ue->e_cgi.cell_id >> 24);
    EUTRAN_CGI->cell_ID.buf[1] = (test_ue->e_cgi.cell_id >> 16);
    EUTRAN_CGI->cell_ID.buf[2] = (test_ue->e_cgi.cell_id >> 8);
    EUTRAN_CGI->cell_ID.buf[3] = (test_ue->e_cgi.cell_id);
    EUTRAN_CGI->cell_ID.bits_unused = 4;

    return ogs_s1ap_encode(&pdu);
}

ogs_pkbuf_t *test_s1ap_build_initial_context_setup_response(test_ue_t *test_ue)
{
    int rv;

    test_sess_t *sess = NULL;
    test_bearer_t *bearer = NULL;

    S1AP_S1AP_PDU_t pdu;
    S1AP_SuccessfulOutcome_t *successfulOutcome = NULL;
    S1AP_InitialContextSetupResponse_t *InitialContextSetupResponse = NULL;
    ogs_sockaddr_t *addr = NULL;

    S1AP_InitialContextSetupResponseIEs_t *ie = NULL;
    S1AP_MME_UE_S1AP_ID_t *MME_UE_S1AP_ID = NULL;
    S1AP_ENB_UE_S1AP_ID_t *ENB_UE_S1AP_ID = NULL;
    S1AP_E_RABSetupListCtxtSURes_t *E_RABSetupListCtxtSURes = NULL;

    S1AP_E_RABSetupItemCtxtSUResIEs_t *item = NULL;
    S1AP_E_RABSetupItemCtxtSURes_t *e_rab = NULL;

    ogs_assert(test_ue);
    sess = ogs_list_first(&test_ue->sess_list);
    ogs_assert(sess);
    bearer = ogs_list_first(&sess->bearer_list);
    ogs_assert(bearer);

    memset(&pdu, 0, sizeof (S1AP_S1AP_PDU_t));
    pdu.present = S1AP_S1AP_PDU_PR_successfulOutcome;
    pdu.choice.successfulOutcome =
        CALLOC(1, sizeof(S1AP_SuccessfulOutcome_t));

    successfulOutcome = pdu.choice.successfulOutcome;
    successfulOutcome->procedureCode =
        S1AP_ProcedureCode_id_InitialContextSetup;
    successfulOutcome->criticality = S1AP_Criticality_reject;
    successfulOutcome->value.present =
        S1AP_SuccessfulOutcome__value_PR_InitialContextSetupResponse;

    InitialContextSetupResponse =
        &successfulOutcome->value.choice.InitialContextSetupResponse;

    ie = CALLOC(1, sizeof(S1AP_InitialContextSetupResponseIEs_t));
    ASN_SEQUENCE_ADD(&InitialContextSetupResponse->protocolIEs, ie);

    ie->id = S1AP_ProtocolIE_ID_id_MME_UE_S1AP_ID;
    ie->criticality = S1AP_Criticality_ignore;
    ie->value.present =
        S1AP_InitialContextSetupResponseIEs__value_PR_MME_UE_S1AP_ID;

    MME_UE_S1AP_ID = &ie->value.choice.MME_UE_S1AP_ID;

    *MME_UE_S1AP_ID = test_ue->mme_ue_s1ap_id;

    ie = CALLOC(1, sizeof(S1AP_InitialContextSetupResponseIEs_t));
    ASN_SEQUENCE_ADD(&InitialContextSetupResponse->protocolIEs, ie);

    ie->id = S1AP_ProtocolIE_ID_id_eNB_UE_S1AP_ID;
    ie->criticality = S1AP_Criticality_ignore;
    ie->value.present =
        S1AP_InitialContextSetupResponseIEs__value_PR_ENB_UE_S1AP_ID;

    ENB_UE_S1AP_ID = &ie->value.choice.ENB_UE_S1AP_ID;

    *ENB_UE_S1AP_ID = test_ue->enb_ue_s1ap_id;

    ie = CALLOC(1, sizeof(S1AP_InitialContextSetupResponseIEs_t));
    ASN_SEQUENCE_ADD(&InitialContextSetupResponse->protocolIEs, ie);

    ie->id = S1AP_ProtocolIE_ID_id_E_RABSetupListCtxtSURes;
    ie->criticality = S1AP_Criticality_ignore;
    ie->value.present =
        S1AP_InitialContextSetupResponseIEs__value_PR_E_RABSetupListCtxtSURes;

    E_RABSetupListCtxtSURes = &ie->value.choice.E_RABSetupListCtxtSURes;

    item = CALLOC(1, sizeof(S1AP_E_RABSetupItemCtxtSUResIEs_t));
    ASN_SEQUENCE_ADD(&E_RABSetupListCtxtSURes->list, item);

    item->id = S1AP_ProtocolIE_ID_id_E_RABSetupItemCtxtSURes;
    item->criticality = S1AP_Criticality_ignore;
    item->value.present =
        S1AP_E_RABSetupItemCtxtSUResIEs__value_PR_E_RABSetupItemCtxtSURes;

    e_rab = &item->value.choice.E_RABSetupItemCtxtSURes;

    e_rab->e_RAB_ID = bearer->ebi;

    rv = ogs_asn_ip_to_BIT_STRING(&bearer->enb_s1u_ip,
            &e_rab->transportLayerAddress);
    ogs_assert(rv == OGS_OK);
    ogs_asn_uint32_to_OCTET_STRING(bearer->enb_s1u_teid, &e_rab->gTP_TEID);

    return ogs_s1ap_encode(&pdu);
}
