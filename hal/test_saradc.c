/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (c) 2020 Rockchip Electronics Co., Ltd.
 */

#include "hal_bsp.h"
#include "hal_base.h"
#include "unity.h"
#include "unity_fixture.h"

#ifdef HAL_SARADC_MODULE_ENABLED

TEST_GROUP_RUNNER(HAL_SARADC) {
    int32_t ret;
    int32_t channel = 0x1;
    int32_t val;

    while (1) {
        HAL_SARADC_Start(SARADC, SARADC_INT_MOD, channel);
        HAL_DelayMs(500);
        val = HAL_SARADC_GetRaw(SARADC);
        HAL_DBG("saradc val = %d\n", val);
        HAL_SARADC_Stop(SARADC);
    }
}

#endif