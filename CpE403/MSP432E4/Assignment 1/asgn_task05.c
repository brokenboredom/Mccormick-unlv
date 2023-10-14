// Initialization and execution of Heartbeat

/* DriverLib Includes */
#include <ti/devices/msp432e4/driverlib/driverlib.h>

/* Standard Includes */
#include <stdint.h>
#include <stdbool.h>

/* Display Include via console */
#include "uartstdio.h"

//*****************************************************************************
//
// Flag to tell the watchdog interrupt handler whether or not to clear the
// interrupt (feed the watchdog).
//
//*****************************************************************************
volatile bool g_bFeedWatchdog = true;

uint32_t systemClock;

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
}

int main(void)
{

    /* Configure the system clock for 120 MHz */
    systemClock = MAP_SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN |
                                          SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480),
                                          120000000);

    //
    // Enable the peripherals used by this example.
    //
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_WDOG0);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);

    //
    // Enable processor interrupts.
    //
    MAP_IntMasterEnable();

    //
    // Set GPIO PN0 as an output.  This drives an LED on the board that will
    // toggle when a watchdog interrupt is processed.
    //
    MAP_GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_0);
    MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_0, 0);

    //
    // Enable the watchdog interrupt.
    //
    MAP_IntEnable(INT_WATCHDOG);

    //
    // Set the period of the watchdog timer to 1 second.
    //
    MAP_WatchdogReloadSet(WATCHDOG0_BASE, systemClock);

    //
    // Enable reset generation from the watchdog timer.
    //
    MAP_WatchdogResetEnable(WATCHDOG0_BASE);

    //
    // Enable the watchdog timer.
    //
    MAP_WatchdogEnable(WATCHDOG0_BASE);

   /* Wait loop */
        while(1)
        {
        }

}
