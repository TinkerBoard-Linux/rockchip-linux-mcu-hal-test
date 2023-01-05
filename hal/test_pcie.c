/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (c) 2022 Rockchip Electronics Co., Ltd.
 */

#include "hal_bsp.h"
#include "hal_base.h"
#include "unity.h"
#include "unity_fixture.h"

#if defined(HAL_PCIE_MODULE_ENABLED)

static int PCIE_WaitForLinkUp(struct HAL_PCIE_HANDLE *pcie)
{
    int retries;

    for (retries = 0; retries < 100000; retries++) {
        if (HAL_PCIE_LinkUp(pcie)) {
            /*
             * We may be here in case of L0 in Gen1. But if EP is capable
             * of Gen2 or Gen3, Gen switch may happen just in this time, but
             * we keep on accessing devices in unstable link status. Given
             * that LTSSM max timeout is 24ms per period, we can wait a bit
             * more for Gen switch.
             */
            HAL_DelayMs(50);
            printf("PCIe Link up, LTSSM is 0x%lx\n", HAL_PCIE_GetLTSSM(pcie));

            return 0;
        }
        printf("PCIe Linking... LTSSM is 0x%lx\n", HAL_PCIE_GetLTSSM(pcie));
        HAL_DelayMs(20);
    }
    printf("PCIe Link failed, LTSSM is 0x%lx\n", HAL_PCIE_GetLTSSM(pcie));
    HAL_DelayMs(20);

    return -1;
}

static int PCIE_WaitForDmaFinished(struct HAL_PCIE_HANDLE *pcie, struct DMA_TABLE *cur)
{
    int us = 0xFFFFFFFF;
    int ret;

    while (us--) {
        ret = HAL_PCIE_GetDmaStatus(pcie, cur->chn, cur->dir);
        if (ret) {
            return 0;
        }
        HAL_CPUDelayUs(1);
    }

    return -1;
}

static int PCIE_Init(struct HAL_PCIE_HANDLE *pcie)
{
    int ret;

    HAL_PCIE_Init(pcie);

    ret = PCIE_WaitForLinkUp(pcie);
    if (ret) {
        return ret;
    }

    return 0;
}

/*************************** DEMO DRIVER ****************************/

/*************************** DEMO TEST ****************************/

#define TEST_PCIE_DMA_BUS_ADDR   0x3C000000
#define TEST_PCIE_DMA_LOCAL_ADDR 0x3C000000
#define TEST_PCIE_DMA_SIZE       0x1000
#define TEST_PCIE_DMA_CHAN       0

TEST_GROUP(HAL_PCIE);

TEST_SETUP(HAL_PCIE){
}

TEST_TEAR_DOWN(HAL_PCIE){
}

/* PCIe test case 0 */
TEST(HAL_PCIE, DemoSimpleTest){
    struct DMA_TABLE table;
    int ret;

    printf("PCIe Test\n");

    ret = PCIE_Init(&g_pcieDev);
    TEST_ASSERT(ret == 0);

    /* DMA write test */
    memset(&table, 0, sizeof(struct DMA_TABLE));
    table.bufSize = TEST_PCIE_DMA_SIZE;
    table.bus = TEST_PCIE_DMA_BUS_ADDR;
    table.local = TEST_PCIE_DMA_LOCAL_ADDR;
    table.chn = TEST_PCIE_DMA_CHAN;
    table.dir = DMA_TO_BUS;

    HAL_PCIE_ConfigDma(&g_pcieDev, &table);
    HAL_PCIE_StartDma(&g_pcieDev, &table);
    ret = PCIE_WaitForDmaFinished(&g_pcieDev, &table);
    TEST_ASSERT(ret == 0);
    printf("PCIe DMA wr success\n");

    /* DMA read test */
    memset(&table, 0, sizeof(struct DMA_TABLE));
    table.bufSize = TEST_PCIE_DMA_SIZE;
    table.bus = TEST_PCIE_DMA_BUS_ADDR;
    table.local = TEST_PCIE_DMA_LOCAL_ADDR;
    table.chn = TEST_PCIE_DMA_CHAN;
    table.dir = DMA_FROM_BUS;

    HAL_PCIE_ConfigDma(&g_pcieDev, &table);
    HAL_PCIE_StartDma(&g_pcieDev, &table);
    ret = PCIE_WaitForDmaFinished(&g_pcieDev, &table);
    TEST_ASSERT(ret == 0);
    printf("PCIe DMA rd success\n");
}

TEST_GROUP_RUNNER(HAL_PCIE){
    RUN_TEST_CASE(HAL_PCIE, DemoSimpleTest);
}

#endif
