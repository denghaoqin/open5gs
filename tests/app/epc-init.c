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

#include "test-epc.h"

static ogs_thread_t *nrf_thread = NULL;
static ogs_thread_t *pcrf_thread = NULL;
static ogs_thread_t *upf_thread = NULL;
static ogs_thread_t *smf_thread = NULL;
static ogs_thread_t *sgwc_thread = NULL;
static ogs_thread_t *sgwu_thread = NULL;
static ogs_thread_t *hss_thread = NULL;

int app_initialize(const char *const argv[])
{
    int rv;

    const char *argv_out[OGS_ARG_MAX];
    bool user_config = false;
    int i = 0;

    for (i = 0; argv[i]; i++) {
        if (strcmp("-c", argv[i]) == 0) {
            user_config = true; 
        }
        argv_out[i] = argv[i];
    }
    argv_out[i] = NULL;

    if (!user_config) {
        argv_out[i++] = "-c";
        argv_out[i++] = DEFAULT_CONFIG_FILENAME;
        argv_out[i] = NULL;
    }

    if (ogs_config()->parameter.no_nrf == 0)
        nrf_thread = test_child_create("nrf", argv_out);
    if (ogs_config()->parameter.no_pcrf == 0)
        pcrf_thread = test_child_create("pcrf", argv_out);
    if (ogs_config()->parameter.no_hss == 0)
        hss_thread = test_child_create("hss", argv_out);

    /*
     * To avoid freeDiameter error
     *
     * ROUTING ERROR
     * 'No remaining suitable candidate to route the message to' for:
     */
    ogs_msleep(500);

    if (ogs_config()->parameter.no_upf == 0)
        upf_thread = test_child_create("upf", argv_out);
    if (ogs_config()->parameter.no_sgwu == 0)
        sgwu_thread = test_child_create("sgwu", argv_out);
    if (ogs_config()->parameter.no_sgwc == 0)
        sgwc_thread = test_child_create("sgwc", argv_out);

    /*
     * To avoid freeDiameter error
     *
     * ROUTING ERROR
     * 'No remaining suitable candidate to route the message to' for:
     */
    ogs_msleep(500);

    if (ogs_config()->parameter.no_smf == 0)
        smf_thread = test_child_create("smf", argv_out);

    ogs_sctp_init(ogs_config()->usrsctp.udp_port);

    rv = mme_initialize();
    ogs_assert(rv == OGS_OK);
    ogs_info("MME initialize...done");

    return OGS_OK;;
}

void app_terminate(void)
{
    mme_terminate();

    ogs_sctp_final();
    ogs_info("MME terminate...done");

    if (sgwc_thread) ogs_thread_destroy(sgwc_thread);
    if (smf_thread) ogs_thread_destroy(smf_thread);
    if (sgwu_thread) ogs_thread_destroy(sgwu_thread);
    if (upf_thread) ogs_thread_destroy(upf_thread);
    if (hss_thread) ogs_thread_destroy(hss_thread);
    if (pcrf_thread) ogs_thread_destroy(pcrf_thread);
    if (nrf_thread) ogs_thread_destroy(nrf_thread);
}

void test_epc_init(void)
{
    ogs_log_install_domain(&__ogs_sctp_domain, "sctp", OGS_LOG_ERROR);
    ogs_log_install_domain(&__ogs_s1ap_domain, "s1ap", OGS_LOG_ERROR);
    ogs_log_install_domain(&__ogs_diam_domain, "diam", OGS_LOG_ERROR);
    ogs_log_install_domain(&__ogs_dbi_domain, "dbi", OGS_LOG_ERROR);
    ogs_log_install_domain(&__ogs_nas_domain, "nas", OGS_LOG_ERROR);

    ogs_assert(ogs_dbi_init(ogs_config()->db_uri) == OGS_OK);
}
