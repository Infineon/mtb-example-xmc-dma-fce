/******************************************************************************
* File Name:   main.c
*
* Description: This is the source code for the XMC MCU: DMA FCE Example for 
*              ModusToolbox. The code example demonstrates how to use DMA to 
*              feed the Flexible CRC Engine (FCE) to calculate the CRC-32 on a 
*              fictitious frame of data.
*
* Related Document: See README.md
*
******************************************************************************
*
* Copyright (c) 2021, Infineon Technologies AG
* All rights reserved.                        
*                                             
* Boost Software License - Version 1.0 - August 17th, 2003
* 
* Permission is hereby granted, free of charge, to any person or organization
* obtaining a copy of the software and accompanying documentation covered by
* this license (the "Software") to use, reproduce, display, distribute,
* execute, and transmit the Software, and to prepare derivative works of the
* Software, and to permit third-parties to whom the Software is furnished to
* do so, all subject to the following:
* 
* The copyright notices in the Software and this entire statement, including
* the above license grant, this restriction and the following disclaimer,
* must be included in all copies of the Software, in whole or in part, and
* all derivative works of the Software, unless such copies or derivative
* works are solely in the form of machine-executable object code generated by
* a source language processor.
* 
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
* SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
* FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
* ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
* DEALINGS IN THE SOFTWARE.
*                                                                              
*****************************************************************************/

#include "cybsp.h"
#include "cy_utils.h"
#include "xmc_fce.h"
#include "xmc_dma.h"

/*******************************************************************************
* Defines
*******************************************************************************/
/* Declarations for LED toggle timing */
#define TICKS_PER_SECOND   1000
#define TICKS_WAIT_MS      500

/* DMA Channel 2 */
#define GPDMA_CHANNEL_NUM  2

/* Data Length of the fictitious data */
#define DATA_SIZE          256UL

/* Expected crc32 of the fictitious data */
#define EXPECTED_CRC       0x99f69cd9


/*******************************************************************************
* Variables
*******************************************************************************/
/* Test data for which crc32 should be calculated */
const uint32_t data[DATA_SIZE] =
{
        0x54EC0525,  0x447135B2,  0x6EED86F9,  0x10B9EE76,  0x9C5C8F55,  0x44C8F0A0,  0x9F5C41D3,  0x4A6C8EF3,
        0x3CB43AC2,  0x9A9E6CA4,  0xA296D5A0,  0x2BE2EF95,  0x7501B963,  0x2906AD29,  0xE695DF1E,  0xA1E3C9E8,
        0x907339C8,  0xF205C7A7,  0xD6DE4D18,  0xB6A9D7D5,  0x79F98F63,  0x467F7A6E,  0x11B5B005,  0x69B97842,
        0xD1B9129C,  0xDDBA1E26,  0x42E421EF,  0xA8BEDDE7,  0x33995D90,  0xC639D14D,  0xAC58E5ED,  0x5D71FE17,
        0xEE1CDF08,  0x3D5E0D81,  0xACB04FA7,  0x7FD4CE83,  0x6BE85D52,  0x02EB1A5A,  0x24CC89DC,  0x37F18392,
        0x56C886E7,  0x9C8BCC4C,  0xBB3475DA,  0x0424F800,  0x404A14F4,  0x41D7B9A4,  0x8F97D2B4,  0x16D928C5,
        0x2C059085,  0xC1BACF2D,  0xE3A582FE,  0x48ED877C,  0xC2BE6BBB,  0x3F83C7EF,  0x752CE9CB,  0x0D1E01CA,
        0x02ECDF82,  0x3D45F86B,  0xB359203D,  0x611A60E6,  0x3FE3B879,  0x34E81B39,  0x5B761930,  0x869EC1A5,
        0xBF58005D,  0x8B9C20F5,  0x058D22A3,  0x11AB9E24,  0xCC534712,  0xA268FEB4,  0x4EA7DE22,  0x34DC0414,
        0xF430CB9D,  0x9069905B,  0x3A9487E0,  0x930C11BA,  0x5D9D6FF3,  0x9C3B5B26,  0xD099A501,  0xE55FC72E,
        0x76C54E09,  0x11243967,  0x3B952BD2,  0x0617F2FC,  0x2A14FA99,  0x42B1C525,  0xF9EEF4A4,  0xA090F1C3,
        0xFA2480BD,  0x548E7924,  0x2623CE99,  0x544299F4,  0xB902DA0E,  0x71725F30,  0x52016AFB,  0x95386417,
        0x9AD099AB,  0x12F6E52D,  0x618D04C7,  0xD4CCD8A3,  0x01AB1763,  0x8893836D,  0x3A2D270E,  0x999C0DDF,
        0x06ED62BE,  0x887F8DE5,  0x20AA3875,  0x22637F43,  0x85419B21,  0x2E54855D,  0xC6D0D493,  0x38E8F00C,
        0x2F463851,  0xA5930018,  0xC108CEA8,  0x9D20AE8A,  0x4EA42837,  0x3DB7AE94,  0xDA364F9B,  0x256F506A,
        0x78273ADE,  0x6AC3CE93,  0x0B79A658,  0x6EE19398,  0xA3C157A3,  0x0F90187B,  0xEB4B6797,  0xAF834C40,
        0xCBD6BCBB,  0xC103BC69,  0x5F2E65CC,  0x472AAD0B,  0x06911251,  0x87692D91,  0x999ECA09,  0xA8E1BF50,
        0x68642547,  0x0B8B4F7E,  0xB0731848,  0x4ED787A3,  0xA9AF212E,  0x8D105838,  0xD4A1C0A3,  0xDD03DE66,
        0x5675CCE7,  0x2B0A99A5,  0x4C2A3071,  0x0635D4F3,  0xD5EBA468,  0x77B2CCE7,  0xB9B2722F,  0x0457FCFA,
        0xC963D641,  0x2401AFCA,  0x5F29B15E,  0x616A97D1,  0x82415FA4,  0xE7C0E4A8,  0x2D7138E2,  0xA4DFF120,
        0x1301A423,  0xEAB3A9B0,  0x432DCEFC,  0xDABD708B,  0x832BD129,  0xD7FA5CA1,  0x607CF76C,  0x8D9EA21D,
        0x2DBE329B,  0x7D6FE0B1,  0xCFF21250,  0xEE449A83,  0x70B84C66,  0x99882725,  0x2487F50A,  0x11DD0B02,
        0x66C457B4,  0xA310227D,  0xF0EF426F,  0x557A1C16,  0x2B42E34D,  0xB2ADF3CB,  0x11430900,  0xA768BC11,
        0xFB364887,  0x150B6F3C,  0x3D60CEDD,  0x7C54E784,  0xB1AC29AC,  0xC90BA26B,  0xDCD7775C,  0xFD9E7FF8,
        0xD9CD97D1,  0x4858F921,  0x08FEDA90,  0x2F7FDE9E,  0x035323C9,  0x833E9235,  0x7E183785,  0x8DE27D17,
        0x35E6C361,  0xEAD03BA8,  0x3227D1B3,  0x13A0BEDF,  0xF77C1EDD,  0x61BDC1F3,  0x5270B33B,  0xCD86C8AE,
        0xF4ED9D50,  0x365A767D,  0x0B048CBA,  0x8571E3CF,  0xBE7C1D76,  0x677D748F,  0x90C664C4,  0x4A2B89C6,
        0x90748331,  0x7D6AE589,  0xFD8746D1,  0x504B41DA,  0x4A98B9D2,  0xD7073E19,  0xF8AE5049,  0xBDF25050,
        0x660B5035,  0x0FFA4172,  0x49CF273D,  0x3C75F234,  0x199FC2A7,  0x114A73A9,  0x446A88BE,  0xCD397931,
        0x4C9AC32B,  0x1FD7D8EB,  0x39672BA1,  0xE352ED93,  0xBACA029F,  0xD88B5BAE,  0xE8A1ECB5,  0xBF2DF783,
        0xDDC77DFC,  0x5EA152A1,  0xE08BC11C,  0x923ECDC6,  0x75A7E9EF,  0xABAB7297,  0x7EDBC951,  0x38CE6F2A,
        0x6E8392A8,  0x88576DB6,  0x4124D40E,  0xD58AD161,  0xD667DC29,  0xC3BB310B,  0xB46DED73,  0xFFFB622D
};

/* CRC engine configuration */
const XMC_FCE_t crc_engine =
{
  .kernel_ptr = FCE_KE0,                /* FCE Kernel Pointer */
  .fce_cfg_update.config_xsel = true,   /* Enables output inversion */
  .fce_cfg_update.config_refin = true,  /* Enables byte-wise reflection */
  .fce_cfg_update.config_refout = true, /* Enables bit-wise reflection */
  .seedvalue = 0xffffffffU              /* CRC seed value to be used */
};

/* DMA channel configuration */
const XMC_DMA_CH_CONFIG_t dma_ch_config =
{
  {
    .enable_interrupt = true,                                          /* Interrupts enabled ? */
    .dst_transfer_width = XMC_DMA_CH_TRANSFER_WIDTH_32,                /* Destination transfer width */
    .src_transfer_width = XMC_DMA_CH_TRANSFER_WIDTH_32,                /* Source transfer width */
    .dst_address_count_mode = XMC_DMA_CH_ADDRESS_COUNT_MODE_NO_CHANGE, /* Destination address count mode */
    .src_address_count_mode = XMC_DMA_CH_ADDRESS_COUNT_MODE_INCREMENT, /* Source address count mode */
    .dst_burst_length = XMC_DMA_CH_BURST_LENGTH_8,                     /* Destination burst length */
    .src_burst_length = XMC_DMA_CH_BURST_LENGTH_8,                     /* Source burst length */
    .enable_src_gather = false,                                        /* Source gather enabled? */
    .enable_dst_scatter = false,                                       /* Destination scatter enabled? */
    .transfer_flow = XMC_DMA_CH_TRANSFER_FLOW_M2M_DMA,                 /* Transfer flow */
  },
  .src_addr = (uint32_t)&data[0],                                      /* Source address */
  .dst_addr = (uint32_t)&(FCE_KE0->IR),                                /* Destination address */
  .block_size = DATA_SIZE,                                             /* Block size */
  .transfer_type = XMC_DMA_CH_TRANSFER_TYPE_SINGLE_BLOCK,              /* Transfer type */
  .priority = XMC_DMA_CH_PRIORITY_0,                                   /* Priority level */
  .src_handshaking = XMC_DMA_CH_SRC_HANDSHAKING_SOFTWARE,              /* Source handshaking */
  .dst_handshaking = XMC_DMA_CH_DST_HANDSHAKING_SOFTWARE               /* Destination handshaking */
};

/* Variable to indicate the transfer complete status of the DMA */
volatile bool transfer_done = false;


/*******************************************************************************
* Function Name: SysTick_Handler
********************************************************************************
* Summary:
* Function called by system timer every millisecond.
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void SysTick_Handler(void)
{
    static uint32_t ticks = 0;

    ticks++;
    if (TICKS_WAIT_MS == ticks)
    {
        /* Toggle LED */
        XMC_GPIO_ToggleOutput(CYBSP_USER_LED_PORT, CYBSP_USER_LED_PIN);
        ticks = 0;
    }
}


/*******************************************************************************
* Function Name: GPDMA0_0_IRQHandler
********************************************************************************
* Summary:
* This is the interrupt handler function for the GPDMA block transfer complete
* event
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void GPDMA0_0_IRQHandler(void)
{
    transfer_done = true;

    /* Clear the DMA interrupt */
    XMC_DMA_CH_ClearEventStatus(XMC_DMA0, GPDMA_CHANNEL_NUM, XMC_DMA_CH_EVENT_BLOCK_TRANSFER_COMPLETE);
}


/*******************************************************************************
* Function Name: main
********************************************************************************
* Summary:
* This is the main function. This function performs
*  - initial setup of device
*  - initialize GPDMA peripheral
*  - initialize FCE engine
*  - checks if the calculated CRC matches the expected one. If CRC matches,
*    USER LED is turned ON. If there is CRC mismatch, USER LED blinks at
*    2 Hz using system timer
*
* Parameters:
*  none
*
* Return:
*  int
*
*******************************************************************************/
int main(void)
{
    cy_rslt_t result;

    /* Initialize the device and board peripherals */
    result = cybsp_init();
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Initialize and enable the GPDMA peripheral */
    XMC_DMA_Init(XMC_DMA0);

    /* Initialize the DMA channel 2 with provided channel configuration */
    XMC_DMA_CH_Init(XMC_DMA0, GPDMA_CHANNEL_NUM, &dma_ch_config);

    /* Enable DMA event */
    XMC_DMA_CH_EnableEvent(XMC_DMA0, GPDMA_CHANNEL_NUM, XMC_DMA_CH_EVENT_BLOCK_TRANSFER_COMPLETE);

    /* Set the interrupt priority for DMA event and enable the DMA interrupt */
    NVIC_SetPriority(GPDMA0_0_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 63, 0));
    NVIC_EnableIRQ(GPDMA0_0_IRQn);

    /* Initialize the FCE engine */
    XMC_FCE_Init(&crc_engine);

    /* Update the expected CRC-32 */
    XMC_FCE_UpdateCRCCheck(&crc_engine, EXPECTED_CRC);

    /* Set length of message */
    XMC_FCE_UpdateLength(&crc_engine, DATA_SIZE);

    /* Enable auto checking of the calculated CRC */
    XMC_FCE_EnableOperation(&crc_engine, XMC_FCE_CFG_CONFIG_CCE);

    /* Enable the DMA channel to initiate transfer */
    XMC_DMA_CH_Enable(XMC_DMA0, GPDMA_CHANNEL_NUM);

    /* Wait till DMA transfer is complete */
    while (transfer_done == false);

    if (XMC_FCE_GetEventStatus(&crc_engine, XMC_FCE_STS_MISMATCH_CRC) == false)
    {
        /* Turn ON the user LED */
        XMC_GPIO_SetOutputHigh(CYBSP_USER_LED_PORT, CYBSP_USER_LED_PIN);
    }
    else
    {
        /* Toggle user LED using system timer to indicate CRC mismatch */
        SysTick_Config(SystemCoreClock / TICKS_PER_SECOND);
    }

    while(1);
}

/* [] END OF FILE */
