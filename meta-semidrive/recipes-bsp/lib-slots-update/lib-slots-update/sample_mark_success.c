/*
 * SEMIDRIVE Copyright Statement
 * Copyright (c) SEMIDRIVE. All rights reserved
 *
 * This software and all rights therein are owned by SEMIDRIVE, and are
 * protected by copyright law and other relevant laws, regulations and
 * protection. Without SEMIDRIVE's prior written consent and/or related rights,
 * please do not use this software or any potion thereof in any form or by any
 * means. You may not reproduce, modify or distribute this software except in
 * compliance with the License. Unless required by applicable law or agreed to
 * in writing, software distributed under the License is distributed on
 * an "AS IS" basis, WITHOUT WARRANTIES OF ANY KIND, either express or implied.
 *
 * You should have received a copy of the License along with this program.
 * If not, see <http://www.semidrive.com/licenses/>.
 */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "slots_parse.h"

const char *slot_name[] = {"A", "B"};

int mark_success(storage_id_e dev)
{
    int ret = -1;
    int slot;
    partition_device_t *ptdev = NULL;

    ret = switch_storage_dev(&ptdev, dev);

    if (0 != ret) {
        PRINTF_CRITICAL("failed to setup ptdev\n");
        return -1;
    }

    /* get the current active slot */
    slot = get_current_slot(ptdev);

    if (slot == INVALID) {
        PRINTF_CRITICAL("bad slot %d\n", slot);
        return -1;
    }

    /* mark bootup successfully if necessary */
    ret = is_slot_marked_successful(ptdev, slot);

    if (ret != 1) {
        /* set slot Retry count(default is 3) and mark slot is Bootable */
        set_active_boot_slot(ptdev, slot);
        /* mark slot as successful */
        mark_boot_successful(ptdev);
        PRINTF_CRITICAL("mark slot %s as successful\n", slot_name[slot]);
    }

    sync();
    ptdev_destroy(ptdev);
    return 0;
}

int main(int argc, char *argv[])
{
    int ret = -1;

    if ((1 == argc) || (!strcmp(argv[1], "emmc"))) {
        ret = mark_success(DEV_EMMC0);
    }
    else if (!strcmp(argv[1], "ospi")) {
        ret = mark_success(DEV_SPI_NOR0);
    }
    else {
        PRINTF_INFO("Usage: %s <option>\n", argv[0]);
        PRINTF_INFO("%s               : mark emmc active slot as successful\n",
                    argv[0]);
        PRINTF_INFO("%s ospi          : mark ospi active slot as successful\n",
                    argv[0]);
        return -1;
    }

    if (0 != ret) {
        PRINTF_CRITICAL("failed to mark ptdev\n");
        return -1;
    }

    return 0;
}