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

abts_suite *test_mo_idle(abts_suite *suite);
abts_suite *test_mt_idle(abts_suite *suite);
abts_suite *test_mo_active(abts_suite *suite);
abts_suite *test_mt_active(abts_suite *suite);
abts_suite *test_mo_sms(abts_suite *suite);
abts_suite *test_mt_sms(abts_suite *suite);
abts_suite *test_crash(abts_suite *suite);

const struct testlist {
    abts_suite *(*func)(abts_suite *suite);
} alltests[] = {
    {test_mo_idle},
    {test_mt_idle},
    {test_mo_active},
    {test_mt_active},
    {test_mo_sms},
    {test_mt_sms},
    {test_crash},
    {NULL},
};

static ogs_thread_t *nrf_thread = NULL;
static ogs_thread_t *pcrf_thread = NULL;
static ogs_thread_t *upf_thread = NULL;
static ogs_thread_t *smf_thread = NULL;
static ogs_thread_t *sgwc_thread = NULL;
static ogs_thread_t *sgwu_thread = NULL;
static ogs_thread_t *hss_thread = NULL;
ogs_socknode_t *sgsap = NULL;

static void terminate(void)
{
    ogs_msleep(50);

    test_child_terminate();

    ogs_info("MME try to terminate");
    mme_terminate();

    testvlr_sgsap_close(sgsap);

    ogs_sctp_final();
    test_epc_final();
    ogs_info("MME terminate...done");

    if (sgwc_thread) ogs_thread_destroy(sgwc_thread);
    if (smf_thread) ogs_thread_destroy(smf_thread);
    if (sgwu_thread) ogs_thread_destroy(sgwu_thread);
    if (upf_thread) ogs_thread_destroy(upf_thread);
    if (hss_thread) ogs_thread_destroy(hss_thread);
    if (pcrf_thread) ogs_thread_destroy(pcrf_thread);
    if (nrf_thread) ogs_thread_destroy(nrf_thread);

    ogs_app_terminate();
}

static void initialize(const char *const argv[])
{
    int rv;

    rv = ogs_app_initialize(NULL, argv);
    ogs_assert(rv == OGS_OK);

    if (ogs_config()->parameter.no_nrf == 0)
        nrf_thread = test_child_create("nrf", argv);
    if (ogs_config()->parameter.no_pcrf == 0)
        pcrf_thread = test_child_create("pcrf", argv);
    if (ogs_config()->parameter.no_hss == 0)
        hss_thread = test_child_create("hss", argv);

    /*
     * To avoid freeDiameter error
     *
     * ROUTING ERROR
     * 'No remaining suitable candidate to route the message to' for:
     */
    ogs_msleep(500);

    if (ogs_config()->parameter.no_upf == 0)
        upf_thread = test_child_create("upf", argv);
    if (ogs_config()->parameter.no_sgwu == 0)
        sgwu_thread = test_child_create("sgwu", argv);
    if (ogs_config()->parameter.no_sgwc == 0)
        sgwc_thread = test_child_create("sgwc", argv);


    /*
     * To avoid freeDiameter error
     *
     * ROUTING ERROR
     * 'No remaining suitable candidate to route the message to' for:
     */
    ogs_msleep(500);

    if (ogs_config()->parameter.no_smf == 0)
        smf_thread = test_child_create("smf", argv);

    test_epc_init();
    ogs_sctp_init(ogs_config()->usrsctp.udp_port);

    sgsap = testvlr_sgsap_server("127.0.0.2");
    ogs_assert(sgsap);

    rv = mme_initialize();
    ogs_assert(rv == OGS_OK);
    ogs_info("MME initialize...done");
}

int main(int argc, const char *const argv[])
{
    int i;
    abts_suite *suite = NULL;

    atexit(terminate);
    test_epc_run(argc, argv, "csfb.yaml", initialize);

    for (i = 0; alltests[i].func; i++)
        suite = alltests[i].func(suite);

    return abts_report(suite);
}
