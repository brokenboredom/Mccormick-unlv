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

Void adcFxn(UArg arg0, UArg arg1);
Void uartFxn(UArg arg0, UArg arg1);
Void pwmFxn(UArg arg0, UArg arg1);
Void hwiFxn(UArg arg);

Task_Struct adcStruct, uartStruct, pwmStruct;
Char adcStack[TASKSTACKSIZE], uartStack[TASKSTACKSIZE], pwmStack[TASKSTACKSIZE];

Semaphore_Struct sem0Struct, sem1Struct;
Semaphore_Handle sem0Handle, sem1Handle;

Clock_Struct clk0Struct;

uint8_t clk_instance_cnt = 0;

UInt32 sleepTickCount;

/* Period and duty in microseconds */
uint16_t   pwmPeriod = 3000;
uint16_t   duty = 0;
uint16_t   dutyInc = 100;

/* Sleep time in microseconds */
PWM_Handle pwm1 = NULL;
PWM_Params params;

/* ADC conversion result variables */
uint16_t adcValue0;
uint32_t adcValue0MicroVolt;

Display_Handle   display;

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

    /* Call driver init functions */
    Board_init();
    GPIO_init();
    PWM_init();
    ADC_init();

    /* Open the UART display for output */
    display = Display_open(Display_Type_UART, NULL);
    if(display == NULL)
    {
        while(1);
    }

    PWM_Params_init(&params);
    params.dutyUnits = PWM_DUTY_US;
    params.dutyValue = 0;
    params.periodUnits = PWM_PERIOD_US;
    params.periodValue = pwmPeriod;
    pwm1 = PWM_open(CONFIG_PWM_0, &params);
    if (pwm1 == NULL) {
        /* CONFIG_PWM_0 did not open */
        while (1);
    }

    PWM_start(pwm1);

//    /* Construct periodic clock with period of 5000/Clock_tickPeriod */
//    // Our main task clock
//    Clock_Params_init(&clkParams);
//    clkParams.period = 5000/Clock_tickPeriod;
//    clkParams.startFlag = TRUE;
//    /* Construct a periodic Clock Instance */
//    Clock_construct(&clk0Struct, (Clock_FuncPtr)clk1Fxn,
//                    5000/Clock_tickPeriod, &clkParams);
//
//    /* Construct a periodic Clock Instance with period 500000/Clock_tickPeriod system time units */
    Clock_Params_init(&clkParams);
    clkParams.period = 500000/Clock_tickPeriod;
    clkParams.startFlag = TRUE;
    /* Construct a periodic Clock Instance */
    Clock_construct(&clk0Struct, (Clock_FuncPtr)heartbeatFxn,
                    500000/Clock_tickPeriod, &clkParams);

    Task_Params_init(&taskParams);
    taskParams.stackSize = TASKSTACKSIZE;

    taskParams.priority = 1;
    taskParams.stack = &adcStack;
    Task_construct(&adcStruct, (Task_FuncPtr)adcFxn, &taskParams, NULL);

    taskParams.priority = 3;
    taskParams.stack = &uartStack;
    Task_construct(&uartStruct, (Task_FuncPtr)uartFxn, &taskParams, NULL);

    taskParams.priority = 5;
    taskParams.stack = &pwmStack;
    Task_construct(&pwmStruct, (Task_FuncPtr)pwmFxn, &taskParams, NULL);


    /* Construct a Semaphore object to be use as a resource lock, inital count 1 */
    Semaphore_Params_init(&semParams);
    Semaphore_construct(&sem0Struct, 1, &semParams);
    Semaphore_construct(&sem1Struct, 1, &semParams);

    /* Obtain instance handle */
    sem0Handle = Semaphore_handle(&sem0Struct);
    sem1Handle = Semaphore_handle(&sem1Struct);

    /* Configure the LED and button pins */
    GPIO_setConfig(CONFIG_GPIO_LED_0, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);
    GPIO_setConfig(CONFIG_GPIO_LED_1, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);
    GPIO_setConfig(CONFIG_GPIO_LED_2, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);
    GPIO_setConfig(CONFIG_GPIO_BUTTON_0, GPIO_CFG_IN_PU | GPIO_CFG_IN_INT_FALLING);

    /* Enable interrupts */
    GPIO_enableInt(CONFIG_GPIO_BUTTON_0);
    CONFIGURE_INTERRUPTS();

    /* We want to sleep for 10000 microseconds */
    sleepTickCount = 50000 / Clock_tickPeriod;

    BIOS_start();    /* Does not return */
    return(0);
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

    ADC_Handle   adc;
    ADC_Params   adcParams;
    int_fast16_t res;

    for (;;) {

        if (Semaphore_getCount(sem0Handle) == 0) {
            System_printf("Sem blocked in task1\n");
        }

        /* Get access to resource */
        Semaphore_pend(sem0Handle, BIOS_WAIT_FOREVER);

        ADC_Params_init(&adcParams);
        adc = ADC_open(CONFIG_ADC_0, &adcParams);

        if (adc == NULL) {
            System_printf("Error initializing CONFIG_ADC_0\n");
            while (1);
        }

        /* Blocking mode conversion */
        res = ADC_convert(adc, &adcValue0);

        if (res == ADC_STATUS_SUCCESS) {

            adcValue0MicroVolt = ADC_convertRawToMicroVolts(adc, adcValue0);
            Display_printf(display, 0, 0, "DEBUG: ADC Executing\n");
            Display_printf(display, 0, 0, "CONFIG_ADC_0 raw result: %d\n", adcValue0);
            Display_printf(display, 0, 0, "CONFIG_ADC_0 convert result: %d uV\n",adcValue0MicroVolt);
//            System_printf("CONFIG_ADC_0 raw result: %d\n", adcValue0);
//            System_printf("CONFIG_ADC_0 convert result: %d uV\n",
//                adcValue0MicroVolt);
        }
        else {
            System_printf("CONFIG_ADC_0 convert failed\n");
        }

        ADC_close(adc);

        Semaphore_post(sem1Handle);

        Task_sleep(sleepTickCount);

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
        //Semaphore_pend(sem1Handle, BIOS_WAIT_FOREVER);
        //Print adc conversion
        Display_print0(display, 0, 0, "DEBUG: UART Executing\n");
        Semaphore_post(sem0Handle);
        Task_sleep(sleepTickCount);
    }
}

Void pwmFxn(UArg arg0, UArg arg1)
{
    while(1)
    {
        //Calculate pwm duty cycle (applied with hwi)
        //PWM duty based off of joystick
        //duty = pwmPeriod * getADCValue[0]/41) / 100
        //duty = pwmPeriod / 3; //DEBUG: Placeholder
        //PWM_setDuty(pwm1, duty);

        duty = (duty + dutyInc);

        if (duty == pwmPeriod || (!duty)) {
            dutyInc = - dutyInc;
        }
        Task_sleep(sleepTickCount);
    }
}

/*
 *  ======== hwiFxn =======
 */
Void hwiFxn(UArg arg){
    CLEAR_INTERRUPT();
    /* Toggle an LED */
    //GPIO_toggle(CONFIG_GPIO_LED_0);
    //Apply pwm duty cycle
    PWM_setDuty(pwm1, duty);
}


