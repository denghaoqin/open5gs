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
