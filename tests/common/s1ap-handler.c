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
    char buf[OGS_ADDRSTRLEN];
    int i, j;

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
