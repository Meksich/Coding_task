#include "cy_pdl.h"
#include "cyhal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"

/* LED blink timer period value */
#define PWM_FREQUENCY (200u)

#define PWM_DUTY_CYCLE (50.0f)

#define LED_BLINK_TIMER_PERIOD            (9999)

#define BASE_FREQUENCY            		  (5000)

#define CHANGED_FREQUENCY            	  (100000)

#define LED_OFF						      (1)

#define LED_ON							  (0)


void timer_init(void);
static void isr_timer(void *callback_arg, cyhal_timer_event_t event);
void clr(void);


bool timer_interrupt_flag = false;
bool led_blink_active_flag = true;
int led_blink_timer_clock_hz = 5000;
float pwm_duty_cycle = 50.0f;

uint8_t uart_read_value;

cyhal_timer_t led_blink_timer;


int main(void) {
	cyhal_pwm_t pwm_led_control;

    cy_rslt_t result;

    result = cybsp_init();
    
    if (result != CY_RSLT_SUCCESS) {
        CY_ASSERT(0);
    }

    __enable_irq();

    result = cy_retarget_io_init(CYBSP_DEBUG_UART_TX, CYBSP_DEBUG_UART_RX,
                                 CY_RETARGET_IO_BAUDRATE);

    if (result != CY_RSLT_SUCCESS) {
        CY_ASSERT(0);
    }

    result = cyhal_gpio_init(CYBSP_USER_LED, CYHAL_GPIO_DIR_OUTPUT, 
                             CYHAL_GPIO_DRIVE_STRONG, CYBSP_LED_STATE_OFF);

    if (result != CY_RSLT_SUCCESS) {
        CY_ASSERT(0);
    }

    printf("\x1b[2J\x1b[;H");

    printf("****************** "
           "PSoC 6 MCU: Coding Interview "
           "****************** \r\n\n");
    result = cyhal_pwm_init(&pwm_led_control, CYBSP_USER_LED, NULL);
        if(CY_RSLT_SUCCESS != result)
        {
            printf("API cyhal_pwm_init failed with error code: %lu\r\n", (unsigned long) result);
            CY_ASSERT(false);
        }
        /* Set the PWM output frequency and duty cycle */
        result = cyhal_pwm_set_duty_cycle(&pwm_led_control, PWM_DUTY_CYCLE, PWM_FREQUENCY);
        if(CY_RSLT_SUCCESS != result)
        {
            printf("API cyhal_pwm_set_duty_cycle failed with error code: %lu\r\n", (unsigned long) result);
            CY_ASSERT(false);
        }
        /* Start the PWM */
        result = cyhal_pwm_start(&pwm_led_control);
        if(CY_RSLT_SUCCESS != result)
        {
            printf("API cyhal_pwm_start failed with error code: %lu\r\n", (unsigned long) result);
            CY_ASSERT(false);
        }

        printf("PWM started successfully. Entering the sleep mode...\r\n");

        cyhal_pwm_stop(&pwm_led_control);

    printf("https://github.com/Meksich?tab=repositories\r\n\n");

    timer_init();
 
    printf("Press 'o' key to on LED or 'f' key "
           "to off it.\r\nTo turn on blinking press 'b' key\r\n\r\n\r\n");

    for (;;) {
        if (cyhal_uart_getc(&cy_retarget_io_uart_obj, &uart_read_value, 1) 
             == CY_RSLT_SUCCESS) {
            if (uart_read_value == 'b') {
                cyhal_timer_start(&led_blink_timer);
                clr();
                printf("LED blinking on              \r\n");
                led_blink_active_flag = 1;
            } else if (led_blink_active_flag && uart_read_value == 'c'){
            	if (led_blink_timer_clock_hz == BASE_FREQUENCY) {
            		led_blink_timer_clock_hz = CHANGED_FREQUENCY;
            	} else {
            		led_blink_timer_clock_hz = BASE_FREQUENCY;
            	}
            	clr();
            	printf("Frequency changed to %d      \r\n", led_blink_timer_clock_hz);
            	cyhal_timer_set_frequency(&led_blink_timer, led_blink_timer_clock_hz);
            } else if (uart_read_value == 'o'){
            	clr();
            	printf("LED is on                    \r\n");
            	cyhal_timer_stop(&led_blink_timer);
            	led_blink_active_flag = 0;
            	cyhal_gpio_write(CYBSP_USER_LED, LED_ON);
            } else if (uart_read_value == 'f'){
            	clr();
            	printf("LED is off                   \r\n");
            	cyhal_timer_stop(&led_blink_timer);
            	led_blink_active_flag = 0;
            	cyhal_gpio_write(CYBSP_USER_LED, LED_OFF);
            } else if (uart_read_value == 'p'){
            	clr();
            	printf("Changing intensity...       \r\n");
            	cyhal_timer_stop(&led_blink_timer);
            	led_blink_active_flag = 0;
            	cyhal_gpio_write(CYBSP_USER_LED, LED_ON);
            	cyhal_uart_read(&cy_retarget_io_uart_obj,
            			&pwm_duty_cycle, 2);
            	cyhal_pwm_start(&pwm_led_control);

            } else {
            	clr();
            	printf("Wrong data");
            }
        }

        if (timer_interrupt_flag) {
            timer_interrupt_flag = false;
            cyhal_gpio_toggle(CYBSP_USER_LED);
        }
    }
}

 void timer_init(void)
 {
    cy_rslt_t result;

    const cyhal_timer_cfg_t led_blink_timer_cfg = 
    {
        .compare_value = 0,                 /* Timer compare value, not used */
        .period = LED_BLINK_TIMER_PERIOD,   /* Defines the timer period */
        .direction = CYHAL_TIMER_DIR_UP,    /* Timer counts up */
        .is_compare = false,                /* Don't use compare mode */
        .is_continuous = true,              /* Run timer indefinitely */
        .value = 0                          /* Initial value of counter */
    };
    result = cyhal_timer_init(&led_blink_timer, NC, NULL);

    if (result != CY_RSLT_SUCCESS) {
        CY_ASSERT(0);
    }

    cyhal_timer_configure(&led_blink_timer, &led_blink_timer_cfg);

    cyhal_timer_set_frequency(&led_blink_timer, led_blink_timer_clock_hz);

    cyhal_timer_register_callback(&led_blink_timer, isr_timer, NULL);

    cyhal_timer_enable_event(&led_blink_timer, CYHAL_TIMER_IRQ_TERMINAL_COUNT,
                              7, true);

    cyhal_timer_start(&led_blink_timer);
 }

static void isr_timer(void *callback_arg, cyhal_timer_event_t event) {
    (void) callback_arg;
    (void) event;
    timer_interrupt_flag = true;
}

void clr(void) {
	printf("\x1b[1F");
}

