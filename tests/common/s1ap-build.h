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

#ifndef TEST_S1AP_BUILD_H
#define TEST_S1AP_BUILD_H

#ifdef __cplusplus
extern "C" {
#endif

ogs_pkbuf_t *test_s1ap_build_s1_setup_request(
        S1AP_ENB_ID_PR present, uint32_t enb_id,
        int tac, uint16_t mcc, uint16_t mnc, uint16_t mnc_len);

ogs_pkbuf_t *test_s1ap_build_initial_ue_message(
        test_ue_t *test_ue, ogs_pkbuf_t *emmbuf, bool s_tmsi);
ogs_pkbuf_t *test_s1ap_build_uplink_nas_transport(
        test_ue_t *test_ue, ogs_pkbuf_t *emmbuf);

#ifdef __cplusplus
}
#endif

#endif /* TEST_S1AP_BUILD_H */
