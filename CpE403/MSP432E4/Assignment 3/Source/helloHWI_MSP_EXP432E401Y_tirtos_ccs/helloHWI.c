/*
 * Copyright (c) 2015-2019, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ======== hello.c ========
 */
#include <stdint.h>
#include <stdio.h>
/* For usleep() */
#include <unistd.h>

/* XDC module Headers */
#include <xdc/std.h>
#include <xdc/runtime/Types.h>
#include <xdc/runtime/Timestamp.h>
#include <xdc/runtime/System.h>
#include <xdc/cfg/global.h>
#include <xdc/runtime/Error.h>

/* BIOS module Headers */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/hal/Hwi.h>

/* Driver Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/Board.h>
#include <ti/drivers/ADC.h>
#include <ti/drivers/PWM.h>
#include <ti/display/Display.h>
#include <ti/drivers/Timer.h>

/* Driver configuration */
#include "ti_drivers_config.h"

/* "useHwiMacros" allows for faster inline disable/enable */
#define ti_sysbios_Build_useHwiMacros
    #define HWI_INT 67
    #define CONFIGURE_INTERRUPTS()
    #define TRIGGER_INTERRUPT() \
        Hwi_post(HWI_INT);
    #define CLEAR_INTERRUPT()
    #define CHANGE_HWI(hwiHandle, hwiFxn) \
        Hwi_setFunc(hwiHandle, hwiFxn, (UArg)NULL);
#define TASKSTACKSIZE           512

/* Task prototypes */
Void adcFxn(UArg arg0, UArg arg1);
Void uartFxn(UArg arg0, UArg arg1);
Void pwmFxn(UArg arg0, UArg arg1);
Void hwiFxn(UArg arg);
void timerCallback(Timer_Handle myHandle, int_fast16_t status);

/* Task variables */
Task_Struct adcStruct, uartStruct, pwmStruct;
Char adcStack[TASKSTACKSIZE], uartStack[TASKSTACKSIZE], pwmStack[TASKSTACKSIZE];
Semaphore_Struct sem0Struct, sem1Struct;
Semaphore_Handle sem0Handle, sem1Handle;
Clock_Struct clk0Struct;

/* Timer instance counter for task tracking */
uint8_t clk_instance_cnt = 0;

UInt32 sleepTickCount;

/* PWM variables */
PWM_Handle pwm1 = NULL;
uint16_t   pwmPeriod = 4000;
uint16_t   duty = 0;

/* ADC variables */
uint16_t adcValue0;
uint32_t adcValue0MicroVolt;
int_fast16_t res;
ADC_Params   adcParams;
ADC_Handle   adc;

/* Display variables */
Display_Handle   display;

/* Timer variables */
Timer_Handle timer0;



/*
 *  ======== clk0Fxn =======
 */
// Main clock triggers every 5ms - runs main tasks
//Void clk0Fxn(UArg arg0)
//{
//    //Increment instance counter
//    clk_instance_cnt++;
//
//    //Handle adc conversion
//    if (clk_instance_cnt == 1) {
//        //Call wrapper to post semaphore
//    }
//    //Handle uart
//    else if (clk_instance_cnt == 2) {
//
//    }
//    //Calculate new pwi duty (applied elsewhere in hwi)
//    else if (clk_instance_cnt == 3) {
//
//
//        //reset instance counter
//        clk_instance_cnt = 0;
//    }
//    else {} // Never happens
//}

/*
 *  ======== heartbeatFxn =======
 */
Void heartbeatFxn(UArg arg0)
{
    GPIO_toggle(CONFIG_GPIO_LED_1);
}

/*
 *  ======== main ========
 */
int main()
{

    /* Construct BIOS objects */
    Task_Params taskParams;
    Semaphore_Params semParams;
    Clock_Params clkParams;
    PWM_Params pwmParams;
    Timer_Params params;

    /* Call driver init functions */
    Board_init();
    GPIO_init();
    PWM_init();
    ADC_init();
    Timer_init();

    /*
    * Setting up the adc for single source manual trigger
    */
    ADC_Params_init(&adcParams);
    adc = ADC_open(CONFIG_ADC_0, &adcParams);
    if (adc == NULL) {
        System_printf("Error initializing CONFIG_ADC_0\n");
        while (1);
    }

    /*
     * Setting up the timer in continuous callback mode that calls the callback
     * function every 5000 microseconds, or 5ms.
     */
    Timer_Params_init(&params);
    params.period = 5000;
    params.periodUnits = Timer_PERIOD_US;
    params.timerMode = Timer_CONTINUOUS_CALLBACK;
    params.timerCallback = timerCallback;
    timer0 = Timer_open(CONFIG_TIMER_0, &params);
    if (timer0 == NULL) {
        /* Failed to initialized timer */
        while (1) {}
    }
    if (Timer_start(timer0) == Timer_STATUS_ERROR) {
        /* Failed to start timer */
        while (1) {}
    }

    /* Open the UART display for output */
    display = Display_open(Display_Type_UART, NULL);
    if(display == NULL)
    {
        while(1);
    }

    /* Configure the PWM with period of 4000 us (250Hz)*/
    PWM_Params_init(&pwmParams);
    pwmParams.dutyUnits = PWM_DUTY_US;
    pwmParams.dutyValue = 0;
    pwmParams.periodUnits = PWM_PERIOD_US;
    pwmParams.periodValue = pwmPeriod;
    pwm1 = PWM_open(CONFIG_PWM_0, &pwmParams);
    if (pwm1 == NULL) {
        /* CONFIG_PWM_0 did not open */
        while (1);
    }
    PWM_start(pwm1);

    /* Construct a periodic Clock Instance with
     * period 500000/Clock_tickPeriod system time units.
     * This is used for the hearbeat */
    Clock_Params_init(&clkParams);
    clkParams.period = 500000/Clock_tickPeriod;
    clkParams.startFlag = TRUE;
    /* Construct a periodic Clock Instance */
    Clock_construct(&clk0Struct, (Clock_FuncPtr)heartbeatFxn,
                    500000/Clock_tickPeriod, &clkParams);

    /* Create tasks to do work
     * stacksize stays the same so declare it once for all tasks*/
    Task_Params_init(&taskParams);
    taskParams.stackSize = TASKSTACKSIZE;
    /* Task1 our ADC task with higher priority*/
    taskParams.priority = 1;
    taskParams.stack = &adcStack;
    Task_construct(&adcStruct, (Task_FuncPtr)adcFxn, &taskParams, NULL);
    /* Task2 our UART task with medium priority*/
    taskParams.priority = 3;
    taskParams.stack = &uartStack;
    Task_construct(&uartStruct, (Task_FuncPtr)uartFxn, &taskParams, NULL);
    /* Task 3 our pwm task with low priority */
    taskParams.priority = 5;
    taskParams.stack = &pwmStack;
    Task_construct(&pwmStruct, (Task_FuncPtr)pwmFxn, &taskParams, NULL);


    /* Construct Semaphore objects to be use as a resource locks
     * sem0 for clock trigger - sem1 for adc completion */
    Semaphore_Params_init(&semParams);
    Semaphore_construct(&sem0Struct, 1, &semParams);
    Semaphore_construct(&sem1Struct, 1, &semParams);
    /* Obtain instance handle */
    sem0Handle = Semaphore_handle(&sem0Struct);
    sem1Handle = Semaphore_handle(&sem1Struct);

    /* Configure the LED and button pins
     * LEDs are for debugging purposes
     * except LED1 is the heartbeat */
    GPIO_setConfig(CONFIG_GPIO_LED_0, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);
    GPIO_setConfig(CONFIG_GPIO_LED_1, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);
    GPIO_setConfig(CONFIG_GPIO_LED_2, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);
    GPIO_setConfig(CONFIG_GPIO_BUTTON_0, GPIO_CFG_IN_PU | GPIO_CFG_IN_INT_FALLING);

    /* Enable interrupts */
    GPIO_enableInt(CONFIG_GPIO_BUTTON_0);
    CONFIGURE_INTERRUPTS();

    /* DEBUG: We want to sleep for 10000 microseconds */
    //sleepTickCount = 50000 / Clock_tickPeriod;

    BIOS_start();    /* Does not return */
    return(0);
}

void timerCallback(Timer_Handle myHandle, int_fast16_t status)
{
    //DEBUG LED
    //GPIO_toggle(CONFIG_GPIO_LED_0);

    //Increment instance counter
    clk_instance_cnt++;

    //Trigger adc conversion at 5ms
    if (clk_instance_cnt == 1) {
        //post clock semaphore
        Semaphore_post(sem0Handle);
    }
    //Trigger uart display at 10ms
    else if (clk_instance_cnt == 2) {
        //Post clock semaphore
        Semaphore_post(sem1Handle);
    }
    //Calculate new pwi duty (applied elsewhere in hwi)
    else if (clk_instance_cnt == 3) {
        //Post clock semaphore
        Semaphore_post(sem0Handle);
        //reset instance counter
        clk_instance_cnt = 0;
    }
    else {} // Never happens

}

/*
 *  ======== adc0Fxn =======
 */
Void adcFxn(UArg arg0, UArg arg1)
{
//    /* wait for semaphore to be posted from Clock function */
//    Semaphore_pend(sem0Handle, BIOS_WAIT_FOREVER);
//    //Run adc conversion
//
//    //Signal resource available
//    Semaphore_post(sem1Handle);

    // DEBUG: ADC placeholder

    for (;;) {

        if (Semaphore_getCount(sem0Handle) == 0) {
            System_printf("Sem blocked in task1\n");
        }

        /* Get access to adc resource */
        Semaphore_pend(sem0Handle, BIOS_WAIT_FOREVER);

        /* Blocking mode conversion */
        res = ADC_convert(adc, &adcValue0);

        if (res == ADC_STATUS_SUCCESS) {

            adcValue0MicroVolt = ADC_convertRawToMicroVolts(adc, adcValue0);
            //Display_printf(display, 0, 0, "DEBUG: ADC Executing\n");
            //Display_printf(display, 0, 0, "CONFIG_ADC_0 raw result: %d\n", adcValue0);
            //Display_printf(display, 0, 0, "CONFIG_ADC_0 convert result: %d uV\n",adcValue0MicroVolt);
//            System_printf("CONFIG_ADC_0 raw result: %d\n", adcValue0);
//            System_printf("CONFIG_ADC_0 convert result: %d uV\n",
//                adcValue0MicroVolt);
        }
        else {
            System_printf("CONFIG_ADC_0 convert failed\n");
        }

        //ADC_close(adc);

        Semaphore_post(sem0Handle);

        //Task_sleep(sleepTickCount);

    }
}
/*
 *  ======== uart0Fxn =======
 */
Void uartFxn(UArg arg0, UArg arg1)
{
    for (;;){
        if (Semaphore_getCount(sem1Handle) == 0) {
            System_printf("Sem blocked in task2\n");
        }
        Semaphore_pend(sem1Handle, BIOS_WAIT_FOREVER);

        if (Semaphore_getCount(sem0Handle) == 0) {
            System_printf("Sem blocked in task2\n");
        }
        Semaphore_pend(sem0Handle, BIOS_WAIT_FOREVER);
        //Print adc conversion
        //Display_printf(display, 0, 0, "DEBUG: UART Executing\n");
        Display_printf(display, 0, 0, "CONFIG_ADC_0 raw result: %d\n", adcValue0);
        //Semaphore_post(sem0Handle);
        //Task_sleep(sleepTickCount);
    }
}

Void pwmFxn(UArg arg0, UArg arg1)
{
    for(;;)
    {
        if (Semaphore_getCount(sem0Handle) == 0) {
            System_printf("Sem blocked in task3\n");
        }
        Semaphore_pend(sem0Handle, BIOS_WAIT_FOREVER);
        //Calculate pwm duty cycle (applied with hwi)
        //PWM duty based off of joystick
        duty = pwmPeriod * adcValue0/41 / 100;
        //duty = pwmPeriod / 3; //DEBUG: Placeholder
        //PWM_setDuty(pwm1, duty);

//        duty = (duty + dutyInc);
//
//        if (duty == pwmPeriod || (!duty)) {
//            dutyInc = - dutyInc;
//        }
        //Task_sleep(sleepTickCount);
    }
}

/*
 *  ======== hwiFxn =======
 */
Void hwiFxn(UArg arg){
    CLEAR_INTERRUPT();
    /* Toggle an LED */
    //GPIO_toggle(CONFIG_GPIO_LED_2);

    //Apply pwm duty cycle
    PWM_setDuty(pwm1, duty);
}


