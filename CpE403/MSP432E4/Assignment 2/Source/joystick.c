/*****************************************************************************
*
* Copyright (C) 2013 - 2017 Texas Instruments Incorporated - http://www.ti.com/
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
*
* * Redistributions of source code must retain the above copyright
*   notice, this list of conditions and the following disclaimer.
*
* * Redistributions in binary form must reproduce the above copyright
*   notice, this list of conditions and the following disclaimer in the
*   documentation and/or other materials provided with the
*   distribution.
*
* * Neither the name of Texas Instruments Incorporated nor the names of
*   its contributors may be used to endorse or promote products derived
*   from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
* A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
* OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
* LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
* THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
******************************************************************************
*
* MSP432 empty main.c template
*
******************************************************************************/

/* DriverLib Includes */
#include <ti/devices/msp432e4/driverlib/driverlib.h>
#include <ti/devices/msp432e4/driverlib/qei.h>

/* Standard Includes */
#include <stdint.h>
#include <stdbool.h>

/* Display Include via console */
#include "uartstdio.h"

//*****************************************************************************
//
// The error routine that is called if the driver library encounters an error.
//
//*****************************************************************************
#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
}
#endif


//*****************************************************************************
//
// Flag to tell the watchdog interrupt handler whether or not to clear the
// interrupt (feed the watchdog).
//
//*****************************************************************************
volatile bool g_bFeedWatchdog = true;

uint32_t systemClock;

static uint32_t getADCValue[1];

volatile bool bgetConvStatus = false;;

void ConfigureUART(uint32_t systemClock)
{
    /* Enable the clock to GPIO port A and UART 0 */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    /* Configure the GPIO Port A for UART 0 */
    MAP_GPIOPinConfigure(GPIO_PA0_U0RX);
    MAP_GPIOPinConfigure(GPIO_PA1_U0TX);
    MAP_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    /* Configure the UART for 115200 bps 8-N-1 format */
    UARTStdioConfig(0, 115200, systemClock);
}


/* PWM ISR */
void PWM0_0_IRQHandler(void)
{
    uint32_t getIntStatus;

    getIntStatus = MAP_PWMGenIntStatus(PWM0_BASE, PWM_GEN_0, true);

    MAP_PWMGenIntClear(PWM0_BASE, PWM_GEN_0, getIntStatus);

}




//*****************************************************************************
//
// The interrupt handler for the watchdog.  This feeds the dog (so that the
// processor does not get reset) and winks the LED connected to GPIO B3.
//
//*****************************************************************************
void
WATCHDOG_IRQHandler(void)
{
    //
    // If we have been told to stop feeding the watchdog, return immediately
    // without clearing the interrupt.  This will cause the system to reset
    // next time the watchdog interrupt fires.
    //
    if(!g_bFeedWatchdog)
    {
        return;
    }

    //
    // Clear the watchdog interrupt.
    //
    MAP_WatchdogIntClear(WATCHDOG0_BASE);

    //
    // Invert the GPIO PN0 value.
    //
    MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0,
                     (MAP_GPIOPinRead(GPIO_PORTN_BASE, GPIO_PIN_0) ^
                                     GPIO_PIN_0));
    /* PWMs Positive Duty Cycle Bit 1 (PF1) 25% */
    // Motor control
    MAP_PWMPulseWidthSet(PWM0_BASE, PWM_OUT_1,
                         MAP_PWMGenPeriodGet(PWM0_BASE, PWM_GEN_0) * (getADCValue[0]/41) / 100 );


    /* PWMs Positive Duty Cycle Bit 0 (PF0) 25% */
    // LED visual
        MAP_PWMPulseWidthSet(PWM0_BASE, PWM_OUT_0,
                             MAP_PWMGenPeriodGet(PWM0_BASE, PWM_GEN_0) * (getADCValue[0]/41) / 100 );
}

int main(void)
{

    /* Configure the system clock for 120 MHz */
    systemClock = MAP_SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN |
                                          SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480),
                                          120000000);

    // Enable the peripherals used by this example.
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_WDOG0);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);

    // Enable processor interrupts.
    MAP_IntMasterEnable();

    // Set GPIO PN0 as an output.  This drives an LED on the board that will
    // toggle when a watchdog interrupt is processed.
    MAP_GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0);
    MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, 0);

    // Enable the watchdog interrupt.
    MAP_IntEnable(INT_WATCHDOG);

    // Set the period of the watchdog timer to 1 second.
    MAP_WatchdogReloadSet(WATCHDOG0_BASE, systemClock);

    // Enable reset generation from the watchdog timer.
    MAP_WatchdogResetEnable(WATCHDOG0_BASE);

    // Enable the watchdog timer.
    MAP_WatchdogEnable(WATCHDOG0_BASE);

    /* Initialize serial console */
    ConfigureUART(systemClock);

    /* Enable the clock to GPIO Port E and wait for it to be ready */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    while(!(MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOE)))
    {
    }


    //*******************************************************************************
    // ADC
    //*******************************************************************************
    /* Configure PE3 as ADC input channel */
    MAP_GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_3);

    /* Enable the clock to ADC-0 and wait for it to be ready */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    while(!(MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_ADC0)))
    {
    }


    /* Configure Sequencer 3 to sample a single analog channel : AIN0 */
    MAP_ADCSequenceStepConfigure(ADC0_BASE, 3, 0, ADC_CTL_CH0 | ADC_CTL_IE |
                                 ADC_CTL_END);

    /* Enable sample sequence 3 with a processor signal trigger.  Sequence 3
     * will do a single sample when the processor sends a signal to start the
     * conversion */
    MAP_ADCSequenceConfigure(ADC0_BASE, 3, ADC_TRIGGER_PROCESSOR, 0);

    /* Since sample sequence 3 is now configured, it must be enabled. */
    MAP_ADCSequenceEnable(ADC0_BASE, 3);

    /* Clear the interrupt status flag.  This is done to make sure the
     * interrupt flag is cleared before we sample. */
    MAP_ADCIntClear(ADC0_BASE, 3);

    /* The PWM peripheral must be enabled for use. */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);
    while(!(MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_PWM0)));

    //*******************************************************************************
    // PWM
    //*******************************************************************************
    /* Set the PWM clock to the system clock. */
    MAP_PWMClockSet(PWM0_BASE,PWM_SYSCLK_DIV_1);

    /* Enable the clock to the GPIO Port F for PWM pins */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    while(!MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF));

    MAP_GPIOPinConfigure(GPIO_PF1_M0PWM1);
    MAP_GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_1);

    /* Configure the PWM0 to count up/down without synchronization.
    * Note: Enabling the dead-band generator automatically couples the 2
    * outputs from the PWM block so we don't use the PWM synchronization. */
    MAP_PWMGenConfigure(PWM0_BASE, PWM_GEN_0, PWM_GEN_MODE_UP_DOWN
                        | PWM_GEN_MODE_DB_NO_SYNC);

    /* Set the PWM period to 250Hz.  To calculate the appropriate parameter
     * use the following equation: N = (1 / f) * SysClk.  Where N is the
     * function parameter, f is the desired frequency, and SysClk is the
     * system clock frequency.
     * In this case you get: (1 / 250Hz) * 120MHz = 480000 cycles.  Note that
     * the maximum period you can set is 2^16 - 1. */
    MAP_PWMGenPeriodSet(PWM0_BASE, PWM_GEN_0, 480000);

    /* Set PWM0 PF1 to a duty cycle of 75%.  You set the duty cycle as a
     * function of the period.  Since the period was set above, you can use the
     * PWMGenPeriodGet() function.  For this example the PWM will be high for
     * 7% of the time or 3*(480000 / 4) clock cycles. */
    MAP_PWMPulseWidthSet(PWM0_BASE, PWM_OUT_1,
                     3*MAP_PWMGenPeriodGet(PWM0_BASE, PWM_GEN_0) / 2);

    MAP_PWMDeadBandEnable(PWM0_BASE, PWM_GEN_0, 160, 160);

    //MAP_IntMasterEnable();

    /* This timer is in up-down mode.  Interrupts will occur when the
     * counter for this PWM counts to the load value (64000), when the
     * counter counts up to 64000/4 (PWM A Up), counts down to 64000/4
     * (PWM A Down), and counts to 0. */
    //MAP_PWMGenIntTrigEnable(PWM0_BASE, PWM_GEN_0,
    //                        PWM_INT_CNT_ZERO | PWM_INT_CNT_LOAD |
    //                        PWM_INT_CNT_AU | PWM_INT_CNT_AD);
   // MAP_IntEnable(INT_PWM0_0);
   // MAP_PWMIntEnable(PWM0_BASE, PWM_INT_GEN_0);

    /* Enable the PWM0 Bit 0 (PF0) and Bit 1 (PF1) output signals. */
    MAP_PWMOutputState(PWM0_BASE, PWM_OUT_1_BIT, true);

    /* Enables the counter for a PWM generator block. */
    MAP_PWMGenEnable(PWM0_BASE, PWM_GEN_0);

    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_2); // Phase
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_3); // !Sleep

    //*******************************************************************************
    // QEI
    //*******************************************************************************
    // Enable QEI Peripherals
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOL); // For PL1 and PL2
    SysCtlPeripheralEnable(SYSCTL_PERIPH_QEI0);

    //Set Pins to be PHA0 and PHB0
    GPIOPinConfigure(GPIO_PL1_PHA0);
    GPIOPinConfigure(GPIO_PL2_PHB0);

    //Set GPIO pins for QEI. PhA0 -> PL1, PhB0 ->PL2. (See documentation shown in first picture)
    GPIOPinTypeQEI(GPIO_PORTL_BASE, GPIO_PIN_1 | GPIO_PIN_2);

    //DISable peripheral and int before configuration
    QEIDisable(QEI0_BASE);
    QEIIntDisable(QEI0_BASE,QEI_INTERROR | QEI_INTDIR | QEI_INTTIMER | QEI_INTINDEX);

    // Configure quadrature encoder, use an arbitrary top limit of 3999
    QEIConfigure(QEI0_BASE, (QEI_CONFIG_CAPTURE_A_B | QEI_CONFIG_NO_RESET | QEI_CONFIG_QUADRATURE | QEI_CONFIG_NO_SWAP), 3999);
    // Divide by clock speed to get counts/sec
    QEIVelocityConfigure(QEI0_BASE, QEI_VELDIV_1, systemClock);

    // Enable the quadrature encoder.
    QEIEnable(QEI0_BASE);
    QEIVelocityEnable(QEI0_BASE);

    //Set position to a middle value so we can see if things are working
    QEIPositionSet(QEI0_BASE, 500);

    /* Loop forever while the PWM signals are generated. */
    while(1)
    {
        /* Trigger the ADC conversion. */
        MAP_ADCProcessorTrigger(ADC0_BASE, 3);

        /* Wait for conversion to be completed. */
        while(!MAP_ADCIntStatus(ADC0_BASE, 3, false))
        {
        }

        /* Clear the ADC interrupt flag. */
        MAP_ADCIntClear(ADC0_BASE, 3);

        /* Read ADC Value. */
        MAP_ADCSequenceDataGet(ADC0_BASE, 3, getADCValue);

        if(getADCValue[0] <= 2000)
        {
            // Create a condition for QEI reading
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0x1);
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0x1);
            MAP_PWMPulseWidthSet(PWM0_BASE, PWM_OUT_1,
                                 MAP_PWMGenPeriodGet(PWM0_BASE, PWM_GEN_0) * getADCValue[0]/41/100);
        }
        else if (getADCValue[0] >= 2100)
        {
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0x0);
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0x1);
            MAP_PWMPulseWidthSet(PWM0_BASE, PWM_OUT_1,
                                 MAP_PWMGenPeriodGet(PWM0_BASE, PWM_GEN_0) * getADCValue[0]/41/100);
        }
        else
        {
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0x0);
        }


        /* Display the AIN0 (PE3) digital value on the console. */
        UARTprintf("AIN0 = %4d, Duty = %4d\n", getADCValue[0], getADCValue[0]/41);
        UARTprintf("Position: %d \n, Velocity: %d", QEIPositionGet(QEI0_BASE),QEIVelocityGet(QEI0_BASE));
        UARTprintf("Encoder direction: %d \n", QEIDirectionGet(QEI0_BASE));
        SysCtlDelay (10000);
    }

}
