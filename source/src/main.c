#include <stdio.h>
#include "stm32f30x.h"
#include "debug_trace.h"
#ifdef USE_DBGUART
#include "dev_uart.h"
#endif
#ifdef USE_STTERM
#include "stlinky.h"
#endif
#include "mod_led.h"
#include "timer_sched.h"
#include "lz4.h"

#define LED_TIMER_MS 500
#define LED_PORT GPIOC
#define LED_PIN GPIO_Pin_13

#define DBG_PIN GPIO_Pin_7
#define DBG_PORT GPIOB

volatile uint32_t glb_tmr_1ms;
volatile uint32_t glb_tmr_1000ms;
volatile uint16_t glb_cntr = 0;
volatile double test_res;
uint32_t trace_levels;

/* Create the list head for the timer */
static LIST_HEAD(obj_timer_list);

/* Block size and count for compression routine 
	The default values are set in the build.sh script
	and they are USE_BLOCK_COUNT=512 and USE_BLOCK_SIZE=8
*/
enum {
	BLOCK_COUNT = USE_BLOCK_COUNT,
	BLOCK_SIZE = 1024 * USE_BLOCK_SIZE
};

// Declare uart
#ifdef USE_DBGUART
DECLARE_UART_DEV(dbg_uart, USART1, 115200, 256, 10, 1);
#endif

#ifdef USE_OVERCLOCKING
extern uint32_t overclock_stm32f303(void);
#endif

int test_lz4();

static inline void main_loop(void)
{
	/* 1 ms timer */
	if (glb_tmr_1ms) {
		glb_tmr_1ms = 0;
		mod_timer_polling(&obj_timer_list);
	}
	if (glb_tmr_1000ms >= 1000) {
		glb_tmr_1000ms = 0;
		glb_cntr = 0;
		DBG_PORT->ODR |= DBG_PIN;
		test_lz4();
		DBG_PORT->ODR &= ~DBG_PIN;
		TRACE(("lz4: %d\n", glb_cntr));
	}
}

void led_on(void *data)
{
	LED_PORT->ODR |= LED_PIN;
}

void led_off(void *data)
{
	LED_PORT->ODR &= ~LED_PIN;
}

void led_init(void *data)
{
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = LED_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(LED_PORT, &GPIO_InitStructure);

	LED_PORT->ODR |= LED_PIN;
	TRACE(("init\n"));
}

void dbg_pin_init(void)
{
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = DBG_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(DBG_PORT, &GPIO_InitStructure);

	DBG_PORT->ODR &= ~DBG_PIN;
}

int main(void)
{
#ifdef USE_OVERCLOCKING
    SystemCoreClock = overclock_stm32f303();
#endif
	if (SysTick_Config(SystemCoreClock / 1000)) {
		/* Capture error */
		while (1);
	}

    trace_levels_set(
			0
			| TRACE_LEVEL_DEFAULT
			,1);

#ifdef USE_DBGUART
	// setup uart port
	dev_uart_add(&dbg_uart);
	// set callback for uart rx
 	dbg_uart.fp_dev_uart_cb = NULL;
 	mod_timer_add((void*) &dbg_uart, 5, (void*) &dev_uart_update, &obj_timer_list);
#endif

	/* Declare LED module and initialize it */
	DECLARE_MODULE_LED(led_module, 8, 250);
	mod_led_init(&led_module);
	mod_timer_add((void*) &led_module, led_module.tick_ms, (void*) &mod_led_update, &obj_timer_list);

	/* Declare LED */
	DECLARE_DEV_LED(def_led, &led_module, 1, NULL, &led_init, &led_on, &led_off);
	dev_led_add(&def_led);
	dev_led_set_pattern(&def_led, 0b11001100);

	dbg_pin_init();

	TRACE(("Program started\n"));

	/* main loop */
	while (1) {
		main_loop();
	}
}

int test_lz4()
{
    LZ4_stream_t lz4Stream_body;
    LZ4_stream_t* lz4Stream = &lz4Stream_body;

    int  inpBufIndex = 0;

    LZ4_initStream(lz4Stream, sizeof (*lz4Stream));

    for(int i=0; i<BLOCK_COUNT; i++) {
        char* const inpPtr = (char*) ((uint32_t)0x20000000);
        const int inpBytes = BLOCK_SIZE;
        {
            char cmpBuf[LZ4_COMPRESSBOUND(BLOCK_SIZE)];
            const int cmpBytes = LZ4_compress_fast_continue(
                lz4Stream, inpPtr, cmpBuf, inpBytes, sizeof(cmpBuf), 1);
            if(cmpBytes <= 0) {
                break;
            }
        }
        inpBufIndex = (inpBufIndex + 1) % 2;
    }
	return 0;
}