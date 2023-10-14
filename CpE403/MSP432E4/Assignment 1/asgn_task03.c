// Initialization and execution of Switch interrupt

/* DriverLib Includes */
#include <ti/devices/msp432e4/driverlib/driverlib.h>

/* Standard Includes */
#include <stdint.h>
#include <stdbool.h>

/* Display Include via console */
#include "uartstdio.h"

uint32_t systemClock;

void GPIOJ_IRQHandler(void)
{
    uint32_t getIntStatus;

    /* Get the interrupt status from the GPIO and clear the status */
    getIntStatus = MAP_GPIOIntStatus(GPIO_PORTJ_BASE, true);

    if((getIntStatus & GPIO_PIN_0) == GPIO_PIN_0)
    {
        MAP_GPIOIntClear(GPIO_PORTJ_BASE, getIntStatus);

        /* Set our duty cycle based off of ADC buffer
        * Formula for duty cycle with adc values between
        * 0-4000 (my joystick limit): PWMPeriod * (srcBuffer/x) / 100 = 0-100%
        * srcBuffer[0]/50 would be 0-81 so we'll use that */
        MAP_PWMPulseWidthSet(PWM0_BASE, PWM_OUT_0,
                                 MAP_PWMGenPeriodGet(PWM0_BASE, PWM_GEN_0) * (srcBuffer[0]/50) / 100 );

        /* Toggle the LED for feedback*/
        MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1,
                         ~(MAP_GPIOPinRead(GPIO_PORTN_BASE, GPIO_PIN_1)));
    }
}

int main(void)
{

    /* Configure the system clock for 120 MHz */
    systemClock = MAP_SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN |
                                          SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480),
                                          120000000);

    //*******************************************************************************
    // SW1 Config
    //*******************************************************************************
    /* Configure the GPIO PN0 as output */
    MAP_GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_1);
    MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1, 0);

    /* Enable the clock to the GPIO Port J and wait for it to be ready */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);
    while(!(MAP_SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOJ)))
    {
    }

    /* Configure the GPIO PJ0 as input with internal pull up enabled. Configure
     * the PJ0 for a rising edge interrupt detection */
    MAP_GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE, GPIO_PIN_0);
    GPIOJ->PUR |= GPIO_PIN_0;
    MAP_GPIOIntTypeSet(GPIO_PORTJ_BASE, GPIO_PIN_0, GPIO_RISING_EDGE);
    MAP_GPIOIntEnable(GPIO_PORTJ_BASE, GPIO_INT_PIN_0);

    MAP_IntEnable(INT_GPIOJ);

    /* Wait loop */
        while(1)
        {
           
        }

}
