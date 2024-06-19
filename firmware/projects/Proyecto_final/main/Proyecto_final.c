/*! @mainpage Blinking
 *
 * \section genDesc General Description
 *
 * This example makes LED_1, LED_2 and LED_3 blink at different rates, using FreeRTOS tasks.
 * 
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 15/05/2024 | Document creation		                         |
 *
 * @author Albano Peñalva (albano.penalva@uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "uart_mcu.h"
#include "timer_mcu.h"
#include "analog_io_mcu.h"

//Dispositivos
#include "servo_sg90.h"
#include "neopixel_stripe.h"
#include "ble_mcu.h"
#include "led.h"
/*==================[macros and definitions]=================================*/
#define TASA_REFRESCO_LENTO 200000        //depende de la velocidad de llenado del recipiente
#define TASA_REFRESCO_RAPIDO 100000

/*==================[internal data definition]===============================*/
TaskHandle_t balanza_task_handle = NULL;
TaskHandle_t valvula_task_handle = NULL;

uint8_t base = 10;                  //en g/cm3
uint16_t pesoElectronico;           //valor medido analogicamente con una balanza 
uint32_t dosisRequerida;                //en cm3-ml 
uint32_t dosisInstantanea = 0; 
bool dosis_seteada = false; 
bool encendido = false;

uint8_t i;
uint8_t j;
char msg[30]; 
/*==================[internal functions declaration]=========================*/
void FuncUART(void* param)
{    
    
}

void read_data(uint8_t * data, uint8_t length)
{
    if(data[0] == 'O')
    {
        encendido = true;
    }

    if(data[0] == 'o')
    {
        encendido = false;
    }
    
    if ((data[0] == 'V') & (!dosis_seteada))
    {
        dosisRequerida = 0;
        i = 1;
        while (data[i] != 'A')
        {
            dosisRequerida = dosisRequerida * 10;
			dosisRequerida = dosisRequerida + (data[i] - '0');
			i++;
        }
        dosis_seteada = true;
    }

}

void controlLEDs()
{
    if ((!dosis_seteada) | (dosisInstantanea == 0 ))
    {
        NeoPixelAllOff();
    }
    else if ((dosisInstantanea > 0) & (dosisInstantanea <= (dosisRequerida*0.125)))
    {
        NeoPixelAllOff();
        NeoPixelSetPixel(0, NEOPIXEL_COLOR_RED);
    }
    else if ((dosisInstantanea > (dosisRequerida*0.125)) & (dosisInstantanea <= (dosisRequerida*0.25)))
    {
        NeoPixelAllOff();
        NeoPixelSetPixel(0, NEOPIXEL_COLOR_RED);
        NeoPixelSetPixel(1, NEOPIXEL_COLOR_RED);
    }
    else if ((dosisInstantanea > (dosisRequerida*0.25)) & (dosisInstantanea <= (dosisRequerida*0.375)))
    {
        NeoPixelAllOff();
        NeoPixelSetPixel(0, NEOPIXEL_COLOR_RED);
        NeoPixelSetPixel(1, NEOPIXEL_COLOR_RED);
        NeoPixelSetPixel(2, NEOPIXEL_COLOR_ORANGE);
    }
    else if ((dosisInstantanea > (dosisRequerida*0.375)) & (dosisInstantanea <= (dosisRequerida*0.5)))
    {
        NeoPixelAllOff();
        NeoPixelSetPixel(0, NEOPIXEL_COLOR_RED);
        NeoPixelSetPixel(1, NEOPIXEL_COLOR_RED);
        NeoPixelSetPixel(2, NEOPIXEL_COLOR_ORANGE);
        NeoPixelSetPixel(3, NEOPIXEL_COLOR_ORANGE);
    }
    else if ((dosisInstantanea > (dosisRequerida*0.5)) & (dosisInstantanea <= (dosisRequerida*0.625)))
    {
        NeoPixelAllOff();
        NeoPixelSetPixel(0, NEOPIXEL_COLOR_RED);
        NeoPixelSetPixel(1, NEOPIXEL_COLOR_RED);
        NeoPixelSetPixel(2, NEOPIXEL_COLOR_ORANGE);
        NeoPixelSetPixel(3, NEOPIXEL_COLOR_ORANGE);
        NeoPixelSetPixel(4, NEOPIXEL_COLOR_YELLOW);
    }
    else if ((dosisInstantanea > (dosisRequerida*0.625)) & (dosisInstantanea <= (dosisRequerida*0.75)))
    {
        NeoPixelAllOff();
        NeoPixelSetPixel(0, NEOPIXEL_COLOR_RED);
        NeoPixelSetPixel(1, NEOPIXEL_COLOR_RED);
        NeoPixelSetPixel(2, NEOPIXEL_COLOR_ORANGE);
        NeoPixelSetPixel(3, NEOPIXEL_COLOR_ORANGE);
        NeoPixelSetPixel(4, NEOPIXEL_COLOR_YELLOW);
        NeoPixelSetPixel(5, NEOPIXEL_COLOR_YELLOW);
    }
    else if ((dosisInstantanea > (dosisRequerida*0.75)) & (dosisInstantanea <= (dosisRequerida*0.875)))
    {
        NeoPixelAllOff();
        NeoPixelSetPixel(0, NEOPIXEL_COLOR_RED);
        NeoPixelSetPixel(1, NEOPIXEL_COLOR_RED);
        NeoPixelSetPixel(2, NEOPIXEL_COLOR_ORANGE);
        NeoPixelSetPixel(3, NEOPIXEL_COLOR_ORANGE);
        NeoPixelSetPixel(4, NEOPIXEL_COLOR_YELLOW);
        NeoPixelSetPixel(5, NEOPIXEL_COLOR_YELLOW);
        NeoPixelSetPixel(6, NEOPIXEL_COLOR_GREEN);
    }
    else if ((dosisInstantanea > (dosisRequerida*0.875)) & (dosisInstantanea < (dosisRequerida)))
    {
        NeoPixelAllOff();
        NeoPixelSetPixel(0, NEOPIXEL_COLOR_RED);
        NeoPixelSetPixel(1, NEOPIXEL_COLOR_RED);
        NeoPixelSetPixel(2, NEOPIXEL_COLOR_ORANGE);
        NeoPixelSetPixel(3, NEOPIXEL_COLOR_ORANGE);
        NeoPixelSetPixel(4, NEOPIXEL_COLOR_YELLOW);
        NeoPixelSetPixel(5, NEOPIXEL_COLOR_YELLOW);
        NeoPixelSetPixel(6, NEOPIXEL_COLOR_GREEN);
        NeoPixelSetPixel(7, NEOPIXEL_COLOR_GREEN);
    }
}

void FuncTimerA(void* param)
{
    xTaskNotifyGive(balanza_task_handle);
}

void FuncTimerB(void* param)
{
    xTaskNotifyGive(valvula_task_handle);
}

static void pesar(void* param)
{
    while (true)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        if (encendido)
        {
            AnalogInputReadSingle(CH0, &pesoElectronico);

            dosisInstantanea = -1*(pesoElectronico - 3477)/(0.74);      //Ecuacion con el recipiente tenido en cuenta       

            if (dosisInstantanea >= dosisRequerida)
            {
                dosis_seteada = false;
            }
            controlLEDs();
        }
    }
}

static void controlValvular(void* param)
{
    while (true)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        if (encendido)
        {
            if ((dosis_seteada) & (dosisInstantanea <= dosisRequerida*0.5))
            {
                ServoMove(SERVO_0, 180);     //180 grados -> valvula abierta
                UartSendString(UART_PC, "Valvula abierta \r\n");
            }
            else if ((dosis_seteada) & (dosisInstantanea > dosisRequerida*0.5) & (dosisInstantanea <= dosisRequerida))
            {
                ServoMove(SERVO_0, 0);    //0 grados -> valvula medio cerrada
                UartSendString(UART_PC, "Valvula medio abierta \r\n");
            } 
            else if ((dosisInstantanea >= dosisRequerida) & (!dosis_seteada))
            {
                ServoMove(SERVO_0, 90);    //90 grados -> valvula cerrada
                UartSendString(UART_PC, "Valvula cerrada \r\n");
            } 
        }
           
    }
}


/*==================[external functions definition]==========================*/
void app_main(void){
    neopixel_color_t color;
    ble_config_t ble_configuration = {
        "Dosimetro",
        read_data
    };

    timer_config_t timer_balanza = {
        .timer = TIMER_A,
        .period = TASA_REFRESCO_LENTO,            //La tasa de refresco debe estar en microsegundos
        .func_p = FuncTimerA,
        .param_p = NULL
    };
    TimerInit(&timer_balanza);

    timer_config_t timer_valvula = {
        .timer = TIMER_B,
        .period = TASA_REFRESCO_RAPIDO,            //La tasa de refresco debe estar en microsegundos
        .func_p = FuncTimerB,
        .param_p = NULL
    };
    TimerInit(&timer_valvula);

    analog_input_config_t conversorAD = {			
	.input = CH0,			
	.mode = ADC_SINGLE,		
	.func_p = NULL,			
	.param_p = NULL,			
	.sample_frec = 0	
    };
    AnalogInputInit(&conversorAD);
    
    serial_config_t puerto_serie = {
        .port = UART_PC,
        .baud_rate = 115200,                //Velocidad de trabajo del puerto serie
        .func_p = FuncUART,
        .param_p = NULL
    };
    UartInit(&puerto_serie);

    ServoInit(SERVO_0, GPIO_9);
    LedsInit();
    BleInit(&ble_configuration);
    NeoPixelInit(BUILT_IN_RGB_LED_PIN, 8, &color);
    
    xTaskCreate(&pesar, "BALANZA", 512, NULL, 5, &balanza_task_handle);
    xTaskCreate(&controlValvular, "VALVULA", 512, NULL, 5, &valvula_task_handle);

    TimerStart(timer_balanza.timer);
    TimerStart(timer_valvula.timer);

    while (1)
    {
        switch(BleStatus())
        {
            case BLE_OFF:
                LedOff(LED_1);
                LedOff(LED_2);
            break;
            case BLE_DISCONNECTED:
                LedOff(LED_2);
                LedOn(LED_1);
            break;
            case BLE_CONNECTED:
                LedOff(LED_1);
                LedOn(LED_2);
            break;
        }

        if (encendido)
        {
            sprintf(msg, "*RDosis actual: %ld", dosisInstantanea);
            BleSendString(msg);
            sprintf(msg, "ml\n*");
            BleSendString(msg);
        }
         
        vTaskDelay(1000/portTICK_PERIOD_MS);
    }
}
