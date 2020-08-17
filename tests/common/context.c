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

#include "test-config-private.h"
#include "test-common.h"

static test_context_t self;

static OGS_POOL(test_ue_pool, test_ue_t);
static OGS_POOL(test_sess_pool, test_sess_t);
static OGS_POOL(test_bearer_pool, test_bearer_t);

static int context_initialized = 0;

void test_context_init(void)
{
    ogs_assert(context_initialized == 0);

    /* Initialize AMF context */
    memset(&self, 0, sizeof(test_context_t));

    ogs_pool_init(&test_ue_pool, ogs_config()->pool.ue);
    ogs_pool_init(&test_sess_pool, ogs_config()->pool.sess);
    ogs_pool_init(&test_bearer_pool, ogs_config()->pool.bearer);

    context_initialized = 1;
}

void test_context_final(void)
{
    ogs_assert(context_initialized == 1);

    ogs_pool_final(&test_ue_pool);
    ogs_pool_final(&test_bearer_pool);
    ogs_pool_final(&test_sess_pool);

    context_initialized = 0;
}

test_context_t *test_self(void)
{
    return &self;
}

static int test_context_prepare(void)
{
    return OGS_OK;
}

static int test_context_validation(void)
{
    if (test_self()->e_served_tai[0].list2.num) {
        memcpy(&test_self()->e_tai, 
            &test_self()->e_served_tai[0].list2.tai[0], sizeof(ogs_5gs_tai_t));
    } else if (test_self()->e_served_tai[0].list0.tai[0].num) {
        test_self()->e_tai.tac =
            test_self()->e_served_tai[0].list0.tai[0].tac[0];
        memcpy(&test_self()->e_tai.plmn_id,
                &test_self()->e_served_tai[0].list0.tai[0].plmn_id,
                OGS_PLMN_ID_LEN);
    }

    if (test_self()->nr_served_tai[0].list2.num) {
        memcpy(&test_self()->nr_tai, 
            &test_self()->nr_served_tai[0].list2.tai[0], sizeof(ogs_5gs_tai_t));
    } else if (test_self()->nr_served_tai[0].list0.tai[0].num) {
        test_self()->nr_tai.tac =
            test_self()->nr_served_tai[0].list0.tai[0].tac[0];
        memcpy(&test_self()->nr_tai.plmn_id,
                &test_self()->nr_served_tai[0].list0.tai[0].plmn_id,
                OGS_PLMN_ID_LEN);
    }

    memcpy(&test_self()->nr_cgi.plmn_id, &test_self()->nr_tai.plmn_id,
            OGS_PLMN_ID_LEN);
    test_self()->nr_cgi.cell_id = 0x40001;

    return OGS_OK;
}

int test_context_parse_config(void)
{
    int rv;
    yaml_document_t *document = NULL;
    ogs_yaml_iter_t root_iter;

    document = ogs_config()->document;
    ogs_assert(document);

    rv = test_context_prepare();
    if (rv != OGS_OK) return rv;

    ogs_yaml_iter_init(&root_iter, document);
    while (ogs_yaml_iter_next(&root_iter)) {
        const char *root_key = ogs_yaml_iter_key(&root_iter);
        ogs_assert(root_key);
        if (!strcmp(root_key, "amf")) {
            ogs_yaml_iter_t amf_iter;
            ogs_yaml_iter_recurse(&root_iter, &amf_iter);
            while (ogs_yaml_iter_next(&amf_iter)) {
                const char *amf_key = ogs_yaml_iter_key(&amf_iter);
                ogs_assert(amf_key);
                if (!strcmp(amf_key, "tai")) {
                    int num_of_list0 = 0;
                    ogs_5gs_tai0_list_t *list0 = NULL;
                    ogs_5gs_tai2_list_t *list2 = NULL;

                    ogs_assert(self.num_of_nr_served_tai <=
                            OGS_MAX_NUM_OF_SERVED_TAI);
                    list0 =
                        &self.nr_served_tai[self.num_of_nr_served_tai].list0;
                    ogs_assert(list0);
                    list2 =
                        &self.nr_served_tai[self.num_of_nr_served_tai].list2;
                    ogs_assert(list2);

                    ogs_yaml_iter_t tai_array, tai_iter;
                    ogs_yaml_iter_recurse(&amf_iter, &tai_array);
                    do {
                        const char *mcc = NULL, *mnc = NULL;
                        ogs_uint24_t tac[OGS_MAX_NUM_OF_TAI];
                        int num_of_tac = 0;

                        if (ogs_yaml_iter_type(&tai_array) ==
                                YAML_MAPPING_NODE) {
                            memcpy(&tai_iter, &tai_array,
                                    sizeof(ogs_yaml_iter_t));
                        } else if (ogs_yaml_iter_type(&tai_array) ==
                            YAML_SEQUENCE_NODE) {
                            if (!ogs_yaml_iter_next(&tai_array))
                                break;
                            ogs_yaml_iter_recurse(&tai_array,
                                    &tai_iter);
                        } else if (ogs_yaml_iter_type(&tai_array) ==
                                YAML_SCALAR_NODE) {
                            break;
                        } else
                            ogs_assert_if_reached();

                        while (ogs_yaml_iter_next(&tai_iter)) {
                            const char *tai_key = ogs_yaml_iter_key(&tai_iter);
                            ogs_assert(tai_key);
                            if (!strcmp(tai_key, "plmn_id")) {
                                ogs_yaml_iter_t plmn_id_iter;

                                ogs_yaml_iter_recurse(&tai_iter, &plmn_id_iter);
                                while (ogs_yaml_iter_next(&plmn_id_iter)) {
                                    const char *plmn_id_key =
                                        ogs_yaml_iter_key(&plmn_id_iter);
                                    ogs_assert(plmn_id_key);
                                    if (!strcmp(plmn_id_key, "mcc")) {
                                        mcc = ogs_yaml_iter_value(
                                                &plmn_id_iter);
                                    } else if (!strcmp(plmn_id_key, "mnc")) {
                                        mnc = ogs_yaml_iter_value(
                                                &plmn_id_iter);
                                    }
                                }
                            } else if (!strcmp(tai_key, "tac")) {
                                ogs_yaml_iter_t tac_iter;
                                ogs_yaml_iter_recurse(&tai_iter, &tac_iter);
                                ogs_assert(ogs_yaml_iter_type(&tac_iter) !=
                                    YAML_MAPPING_NODE);

                                do {
                                    const char *v = NULL;

                                    ogs_assert(num_of_tac <=
                                            OGS_MAX_NUM_OF_TAI);
                                    if (ogs_yaml_iter_type(&tac_iter) ==
                                            YAML_SEQUENCE_NODE) {
                                        if (!ogs_yaml_iter_next(&tac_iter))
                                            break;
                                    }

                                    v = ogs_yaml_iter_value(&tac_iter);
                                    if (v) {
                                        tac[num_of_tac].v = atoi(v);
                                        num_of_tac++;
                                    }
                                } while (
                                    ogs_yaml_iter_type(&tac_iter) ==
                                        YAML_SEQUENCE_NODE);
                            } else
                                ogs_warn("unknown key `%s`", tai_key);
                        }

                        if (mcc && mnc && num_of_tac) {
                            if (num_of_tac == 1) {
                                ogs_plmn_id_build(
                                    &list2->tai[list2->num].plmn_id,
                                    atoi(mcc), atoi(mnc), strlen(mnc));
                                list2->tai[list2->num].tac.v = tac[0].v;

                                list2->num++;
                                if (list2->num > 1)
                                    list2->type = OGS_TAI2_TYPE;
                                else
                                    list2->type = OGS_TAI1_TYPE;
                            } else if (num_of_tac > 1) {
                                int i;
                                ogs_plmn_id_build(
                                    &list0->tai[num_of_list0].plmn_id,
                                    atoi(mcc), atoi(mnc), strlen(mnc));
                                for (i = 0; i < num_of_tac; i++) {
                                    list0->tai[num_of_list0].tac[i].v =
                                        tac[i].v;
                                }

                                list0->tai[num_of_list0].num = num_of_tac;
                                list0->tai[num_of_list0].type = OGS_TAI0_TYPE;

                                num_of_list0++;
                            }
                        } else {
                            ogs_warn("Ignore tai : mcc(%p), mnc(%p), "
                                    "num_of_tac(%d)", mcc, mnc, num_of_tac);
                        }
                    } while (ogs_yaml_iter_type(&tai_array) ==
                            YAML_SEQUENCE_NODE);

                    if (list2->num || num_of_list0) {
                        self.num_of_nr_served_tai++;
                    }
                } else if (!strcmp(amf_key, "plmn")) {
                    ogs_yaml_iter_t plmn_array, plmn_iter;
                    ogs_yaml_iter_recurse(&amf_iter, &plmn_array);
                    do {
                        const char *mnc = NULL, *mcc = NULL;
                        ogs_assert(self.num_of_plmn_support <=
                                OGS_MAX_NUM_OF_PLMN);

                        if (ogs_yaml_iter_type(&plmn_array) ==
                                YAML_MAPPING_NODE) {
                            memcpy(&plmn_iter, &plmn_array,
                                    sizeof(ogs_yaml_iter_t));
                        } else if (ogs_yaml_iter_type(&plmn_array) ==
                            YAML_SEQUENCE_NODE) {
                            if (!ogs_yaml_iter_next(&plmn_array))
                                break;
                            ogs_yaml_iter_recurse(&plmn_array,
                                    &plmn_iter);
                        } else if (ogs_yaml_iter_type(&plmn_array) ==
                            YAML_SCALAR_NODE) {
                            break;
                        } else
                            ogs_assert_if_reached();

                        while (ogs_yaml_iter_next(&plmn_iter)) {
                            const char *plmn_key =
                                ogs_yaml_iter_key(&plmn_iter);
                            ogs_assert(plmn_key);
                            if (!strcmp(plmn_key, "plmn_id")) {
                                ogs_yaml_iter_t plmn_id_iter;

                                ogs_yaml_iter_recurse(&plmn_iter,
                                        &plmn_id_iter);
                                while (ogs_yaml_iter_next(&plmn_id_iter)) {
                                    const char *plmn_id_key =
                                        ogs_yaml_iter_key(&plmn_id_iter);
                                    ogs_assert(plmn_id_key);
                                    if (!strcmp(plmn_id_key, "mcc")) {
                                        mcc = ogs_yaml_iter_value(
                                                &plmn_id_iter);
                                    } else if (!strcmp(plmn_id_key, "mnc")) {
                                        mnc = ogs_yaml_iter_value(
                                                &plmn_id_iter);
                                    }
                                }

                                if (mcc && mnc) {
                                    ogs_plmn_id_build(
                                        &self.plmn_support[
                                            self.num_of_plmn_support].
                                                plmn_id,
                                        atoi(mcc), atoi(mnc), strlen(mnc));
                                }
                            } else if (!strcmp(plmn_key, "s_nssai")) {
                                ogs_yaml_iter_t s_nssai_array, s_nssai_iter;
                                ogs_yaml_iter_recurse(&plmn_iter,
                                        &s_nssai_array);
                                do {
                                    ogs_s_nssai_t *s_nssai = NULL;
                                    const char *sst = NULL, *sd = NULL;
                                    ogs_assert(
                                        self.plmn_support[
                                            self.num_of_plmn_support].
                                                num_of_s_nssai <=
                                            OGS_MAX_NUM_OF_S_NSSAI);
                                    s_nssai = &self.plmn_support[
                                            self.num_of_plmn_support].s_nssai[
                                                self.plmn_support[
                                                    self.num_of_plmn_support].
                                                        num_of_s_nssai];
                                    ogs_assert(s_nssai);

                                    if (ogs_yaml_iter_type(&s_nssai_array) ==
                                            YAML_MAPPING_NODE) {
                                        memcpy(&s_nssai_iter, &s_nssai_array,
                                                sizeof(ogs_yaml_iter_t));
                                    } else if (ogs_yaml_iter_type(
                                                &s_nssai_array) ==
                                                YAML_SEQUENCE_NODE) {
                                        if (!ogs_yaml_iter_next(&s_nssai_array))
                                            break;
                                        ogs_yaml_iter_recurse(&s_nssai_array,
                                                &s_nssai_iter);
                                    } else if (ogs_yaml_iter_type(
                                                &s_nssai_array) ==
                                                YAML_SCALAR_NODE) {
                                        break;
                                    } else
                                        ogs_assert_if_reached();

                                    while (ogs_yaml_iter_next(&s_nssai_iter)) {
                                        const char *s_nssai_key =
                                            ogs_yaml_iter_key(&s_nssai_iter);
                                        ogs_assert(s_nssai_key);
                                        if (!strcmp(s_nssai_key, "sst")) {
                                            sst = ogs_yaml_iter_value(
                                                    &s_nssai_iter);
                                        } else if (!strcmp(
                                                    s_nssai_key, "sd")) {
                                            sd = ogs_yaml_iter_value(
                                                    &s_nssai_iter);
                                        }
                                    }

                                    if (sst) {
                                        s_nssai->sst = atoi(sst);
                                        if (sd)
                                            s_nssai->sd =
                                                ogs_uint24_from_string(
                                                        (char*)sd);
                                        else
                                            s_nssai->sd.v =
                                                OGS_S_NSSAI_NO_SD_VALUE;

                                        s_nssai->mapped_hplmn_sst = 0;
                                        s_nssai->mapped_hplmn_sd.v =
                                                OGS_S_NSSAI_NO_SD_VALUE;

                                        self.plmn_support[
                                            self.num_of_plmn_support].
                                                num_of_s_nssai++;
                                    }

                                } while (ogs_yaml_iter_type(&s_nssai_array) ==
                                        YAML_SEQUENCE_NODE);
                            } else
                                ogs_warn("unknown key `%s`", plmn_key);
                        }

                        if (self.plmn_support[
                                self.num_of_plmn_support].num_of_s_nssai &&
                                mcc && mnc) {
                            self.num_of_plmn_support++;
                        } else {
                            ogs_warn("Ignore plmn : "
                                    "s_nssai(%d) mcc(%s), mnc(%s)",
                                    self.plmn_support[
                                    self.num_of_plmn_support].num_of_s_nssai,
                                    mcc, mnc);
                            self.plmn_support[
                                self.num_of_plmn_support].num_of_s_nssai = 0;
                        }
                    } while (ogs_yaml_iter_type(&plmn_array) ==
                            YAML_SEQUENCE_NODE);
                } else if (!strcmp(amf_key, "sbi")) {
                    /* handle config in sbi library */
                } else
                    ogs_warn("unknown key `%s`", amf_key);
            }
        } else if (!strcmp(root_key, "mme")) {
            ogs_yaml_iter_t mme_iter;
            ogs_yaml_iter_recurse(&root_iter, &mme_iter);
            while (ogs_yaml_iter_next(&mme_iter)) {
                const char *mme_key = ogs_yaml_iter_key(&mme_iter);
                ogs_assert(mme_key);
                if (!strcmp(mme_key, "tai")) {
                    int num_of_list0 = 0;
                    ogs_eps_tai0_list_t *list0 = NULL;
                    ogs_eps_tai2_list_t *list2 = NULL;

                    ogs_assert(self.num_of_e_served_tai <=
                            OGS_MAX_NUM_OF_SERVED_TAI);
                    list0 = &self.e_served_tai[self.num_of_e_served_tai].list0;
                    ogs_assert(list0);
                    list2 = &self.e_served_tai[self.num_of_e_served_tai].list2;
                    ogs_assert(list2);

                    ogs_yaml_iter_t tai_array, tai_iter;
                    ogs_yaml_iter_recurse(&mme_iter, &tai_array);
                    do {
                        const char *mcc = NULL, *mnc = NULL;
                        uint16_t tac[OGS_MAX_NUM_OF_TAI];
                        int num_of_tac = 0;

                        if (ogs_yaml_iter_type(&tai_array) ==
                                YAML_MAPPING_NODE) {
                            memcpy(&tai_iter, &tai_array,
                                    sizeof(ogs_yaml_iter_t));
                        } else if (ogs_yaml_iter_type(&tai_array) ==
                            YAML_SEQUENCE_NODE) {
                            if (!ogs_yaml_iter_next(&tai_array))
                                break;
                            ogs_yaml_iter_recurse(&tai_array,
                                    &tai_iter);
                        } else if (ogs_yaml_iter_type(&tai_array) ==
                                YAML_SCALAR_NODE) {
                            break;
                        } else
                            ogs_assert_if_reached();

                        while (ogs_yaml_iter_next(&tai_iter)) {
                            const char *tai_key = ogs_yaml_iter_key(&tai_iter);
                            ogs_assert(tai_key);
                            if (!strcmp(tai_key, "plmn_id")) {
                                ogs_yaml_iter_t plmn_id_iter;

                                ogs_yaml_iter_recurse(&tai_iter, &plmn_id_iter);
                                while (ogs_yaml_iter_next(&plmn_id_iter)) {
                                    const char *plmn_id_key =
                                        ogs_yaml_iter_key(&plmn_id_iter);
                                    ogs_assert(plmn_id_key);
                                    if (!strcmp(plmn_id_key, "mcc")) {
                                        mcc = ogs_yaml_iter_value(
                                                &plmn_id_iter);
                                    } else if (!strcmp(plmn_id_key, "mnc")) {
                                        mnc = ogs_yaml_iter_value(
                                                &plmn_id_iter);
                                    }
                                }
                            } else if (!strcmp(tai_key, "tac")) {
                                ogs_yaml_iter_t tac_iter;
                                ogs_yaml_iter_recurse(&tai_iter, &tac_iter);
                                ogs_assert(ogs_yaml_iter_type(&tac_iter) !=
                                    YAML_MAPPING_NODE);

                                do {
                                    const char *v = NULL;

                                    ogs_assert(num_of_tac <=
                                            OGS_MAX_NUM_OF_TAI);
                                    if (ogs_yaml_iter_type(&tac_iter) ==
                                            YAML_SEQUENCE_NODE) {
                                        if (!ogs_yaml_iter_next(&tac_iter))
                                            break;
                                    }

                                    v = ogs_yaml_iter_value(&tac_iter);
                                    if (v) {
                                        tac[num_of_tac] = atoi(v);
                                        num_of_tac++;
                                    }
                                } while (
                                    ogs_yaml_iter_type(&tac_iter) ==
                                        YAML_SEQUENCE_NODE);
                            } else
                                ogs_warn("unknown key `%s`", tai_key);
                        }

                        if (mcc && mnc && num_of_tac) {
                            if (num_of_tac == 1) {
                                ogs_plmn_id_build(
                                    &list2->tai[list2->num].plmn_id,
                                    atoi(mcc), atoi(mnc), strlen(mnc));
                                list2->tai[list2->num].tac = tac[0];

                                list2->num++;
                                if (list2->num > 1)
                                    list2->type = OGS_TAI2_TYPE;
                                else
                                    list2->type = OGS_TAI1_TYPE;
                            } else if (num_of_tac > 1) {
                                int i;
                                ogs_plmn_id_build(
                                    &list0->tai[num_of_list0].plmn_id,
                                    atoi(mcc), atoi(mnc), strlen(mnc));
                                for (i = 0; i < num_of_tac; i++) {
                                    list0->tai[num_of_list0].tac[i] = tac[i];
                                }

                                list0->tai[num_of_list0].num = num_of_tac;
                                list0->tai[num_of_list0].type = OGS_TAI0_TYPE;

                                num_of_list0++;
                            }
                        } else {
                            ogs_warn("Ignore tai : mcc(%p), mnc(%p), "
                                    "num_of_tac(%d)", mcc, mnc, num_of_tac);
                        }
                    } while (ogs_yaml_iter_type(&tai_array) ==
                            YAML_SEQUENCE_NODE);

                    if (list2->num || num_of_list0) {
                        self.num_of_e_served_tai++;
                    }
                }
            }
        }
    }

    rv = test_context_validation();
    if (rv != OGS_OK) return rv;

    return OGS_OK;
}

void test_ue_set_mobile_identity_suci(test_ue_t *test_ue,
    ogs_nas_5gs_mobile_identity_suci_t *mobile_identity_suci,
    uint16_t mobile_identity_suci_length)
{
    ogs_nas_5gs_mobile_identity_t mobile_identity;

    ogs_assert(test_ue);
    ogs_assert(mobile_identity_suci);
    ogs_assert(mobile_identity_suci_length);

    test_ue->mobile_identity_suci_length = mobile_identity_suci_length;
    memcpy(&test_ue->mobile_identity_suci, mobile_identity_suci,
            mobile_identity_suci_length);

    mobile_identity.length = test_ue->mobile_identity_suci_length;
    mobile_identity.buffer = &test_ue->mobile_identity_suci;

    if (test_ue->suci)
        ogs_free(test_ue->suci);
    test_ue->suci = ogs_nas_5gs_suci_from_mobile_identity(&mobile_identity);
    if (test_ue->supi)
        ogs_free(test_ue->supi);
    test_ue->supi = ogs_supi_from_suci(test_ue->suci);
    if (test_ue->imsi)
        ogs_free(test_ue->imsi);
    test_ue->imsi = ogs_id_get_value(test_ue->supi);
}

static void test_ue_set_mobile_identity_imsi(test_ue_t *test_ue)
{
    ogs_assert(test_ue);
    ogs_assert(test_ue->imsi);

    test_ue->mobile_identity_imsi.odd_even = OGS_NAS_MOBILE_IDENTITY_ODD;
    test_ue->mobile_identity_imsi.type = OGS_NAS_MOBILE_IDENTITY_IMSI;
    test_ue->mobile_identity_imsi.digit1 = test_ue->imsi[0] - '0';
    test_ue->mobile_identity_imsi.digit2 = test_ue->imsi[1] - '0';
    test_ue->mobile_identity_imsi.digit3 = test_ue->imsi[2] - '0';
    test_ue->mobile_identity_imsi.digit4 = test_ue->imsi[3] - '0';
    test_ue->mobile_identity_imsi.digit5 = test_ue->imsi[4] - '0';
    test_ue->mobile_identity_imsi.digit6 = test_ue->imsi[5] - '0';
    test_ue->mobile_identity_imsi.digit7 = test_ue->imsi[6] - '0';
    test_ue->mobile_identity_imsi.digit8 = test_ue->imsi[7] - '0';
    test_ue->mobile_identity_imsi.digit9 = test_ue->imsi[8] - '0';
    test_ue->mobile_identity_imsi.digit10 = test_ue->imsi[9] - '0';
    test_ue->mobile_identity_imsi.digit11 = test_ue->imsi[10] - '0';
    test_ue->mobile_identity_imsi.digit12 = test_ue->imsi[11] - '0';
    test_ue->mobile_identity_imsi.digit13 = test_ue->imsi[12] - '0';
    test_ue->mobile_identity_imsi.digit14 = test_ue->imsi[13] - '0';
    test_ue->mobile_identity_imsi.digit15 = test_ue->imsi[14] - '0';
}

test_ue_t *test_ue_add_by_suci(
    ogs_nas_5gs_mobile_identity_suci_t *mobile_identity_suci,
    uint16_t mobile_identity_suci_length)
{
    test_ue_t *test_ue = NULL;

    ogs_assert(mobile_identity_suci);
    ogs_assert(mobile_identity_suci_length);

    ogs_pool_alloc(&test_ue_pool, &test_ue);
    ogs_assert(test_ue);
    memset(test_ue, 0, sizeof *test_ue);

    memcpy(&test_ue->e_tai, &test_self()->e_tai, sizeof(ogs_eps_tai_t));
    memcpy(&test_ue->e_cgi.plmn_id, &test_ue->e_tai.plmn_id, OGS_PLMN_ID_LEN);

    memcpy(&test_ue->nr_tai, &test_self()->nr_tai, sizeof(ogs_5gs_tai_t));
    memcpy(&test_ue->nr_cgi.plmn_id, &test_ue->nr_tai.plmn_id, OGS_PLMN_ID_LEN);

    ogs_nas_from_plmn_id(
            &mobile_identity_suci->nas_plmn_id, &test_ue->e_tai.plmn_id);

    test_ue_set_mobile_identity_suci(
            test_ue, mobile_identity_suci, mobile_identity_suci_length);

    test_ue_set_mobile_identity_imsi(test_ue);

    ogs_list_add(&self.test_ue_list, test_ue);

    return test_ue;
}

void test_ue_remove(test_ue_t *test_ue)
{
    ogs_assert(test_ue);

    ogs_list_remove(&self.test_ue_list, test_ue);

    if (test_ue->suci)
        ogs_free(test_ue->suci);
    if (test_ue->supi)
        ogs_free(test_ue->supi);
    if (test_ue->imsi)
        ogs_free(test_ue->imsi);

    test_sess_remove_all(test_ue);

    ogs_pool_free(&test_ue_pool, test_ue);
}

void test_ue_remove_all(void)
{
    test_ue_t *test_ue = NULL, *next = NULL;;

    ogs_list_for_each_safe(&self.test_ue_list, next, test_ue)
        test_ue_remove(test_ue);
}

test_sess_t *test_sess_add_by_apn(test_ue_t *test_ue, char *apn)
{
    test_sess_t *sess = NULL;

    ogs_assert(test_ue);
    ogs_assert(apn);

    ogs_pool_alloc(&test_sess_pool, &sess);
    ogs_assert(sess);
    memset(sess, 0, sizeof *sess);

    sess->apn = ogs_strdup(apn);
    ogs_assert(sess->apn);
    sess->pti = 1; /* Default PTI : 1 */

    test_bearer_add(sess); /* Add default bearer */

    sess->test_ue = test_ue;

    ogs_list_add(&test_ue->sess_list, sess);

    return sess;
}

void test_sess_remove(test_sess_t *sess)
{
    test_ue_t *test_ue = NULL;

    ogs_assert(sess);
    test_ue = sess->test_ue;
    ogs_assert(test_ue);

    ogs_list_remove(&test_ue->sess_list, sess);

    if (sess->dnn)
        ogs_free(sess->dnn);

    test_bearer_remove_all(sess);

    ogs_pool_free(&test_sess_pool, sess);
}

void test_sess_remove_all(test_ue_t *test_ue)
{
    test_sess_t *sess = NULL, *next = NULL;;

    ogs_assert(test_ue);

    ogs_list_for_each_safe(&test_ue->sess_list, next, sess)
        test_sess_remove(sess);
}

test_bearer_t *test_bearer_add(test_sess_t *sess)
{
    test_bearer_t *bearer = NULL;

    ogs_assert(sess);

    ogs_pool_alloc(&test_bearer_pool, &bearer);
    ogs_assert(bearer);
    memset(bearer, 0, sizeof *bearer);

    bearer->sess = sess;

    ogs_list_add(&sess->bearer_list, bearer);

    return bearer;
}

void test_bearer_remove(test_bearer_t *bearer)
{
    ogs_assert(bearer);
    ogs_assert(bearer->sess);

    ogs_list_remove(&bearer->sess->bearer_list, bearer);

    ogs_pool_free(&test_bearer_pool, bearer);
}

void test_bearer_remove_all(test_sess_t *sess)
{
    test_bearer_t *bearer = NULL, *next_bearer = NULL;

    ogs_assert(sess);
    ogs_list_for_each_safe(&sess->bearer_list, next_bearer, bearer)
        test_bearer_remove(bearer);
}

test_bearer_t *test_bearer_find_by_sess_ebi(test_sess_t *sess, uint8_t ebi)
{
    test_bearer_t *bearer = NULL;

    ogs_assert(sess);

    ogs_list_for_each(&sess->bearer_list, bearer)
        if (ebi == bearer->ebi) return bearer;

    return NULL;
}

test_bearer_t *test_bearer_find_by_ue_ebi(test_ue_t *test_ue, uint8_t ebi)
{
    test_sess_t *sess = NULL;
    test_bearer_t *bearer = NULL;

    ogs_assert(test_ue);

    ogs_list_for_each(&test_ue->sess_list, sess) {
        bearer = test_bearer_find_by_sess_ebi(sess, ebi);
        if (bearer) return bearer;

    }

    return NULL;
}
