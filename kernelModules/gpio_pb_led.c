/*
 * gpio_pb_led.c
 * Author: Venkata Naga Ravikiran Bulusu
 * 
 * Description:
 * This kernel module provides an example of handling a push button connected
 * to a GPIO pin. The module configures an interrupt to detect button presses
 * and toggles an LED connected to another GPIO pin. The current status of the 
 * LED is logged in the kernel log (dmesg) after each button press.
 * 
 * connections: https://www.thetips4you.com/wp-content/uploads/2019/06/LED-and-Push-Button.png
 * interchange the GPIO pin 17 and GPIO pin 18 in the above diagram
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/jiffies.h>

#define BUTTON_GPIO_PIN  (17 + GPIO_DYNAMIC_BASE) // GPIO pin connected to the push button
#define LED_GPIO_PIN     (18 + GPIO_DYNAMIC_BASE) // GPIO pin connected to the LED

static unsigned int irq_number;    // Holds the IRQ number for the GPIO pin
static unsigned int led_state = 0; // Holds the current state of the LED (0 = OFF, 1 = ON)
static unsigned long last_jiffie = 0;

// IRQ handler for the button press
static irqreturn_t gpio_irq_handler(int irq, void *dev_id)
{
    unsigned long flags;
    unsigned long diff = jiffies - last_jiffie;

    // Debounce: Ignore if the press is too close to the last one
    if (diff < msecs_to_jiffies(200)) {
        return IRQ_HANDLED;
    }
    last_jiffie = jiffies;

    local_irq_save(flags);
    
    // Toggle the LED state
    led_state = !led_state;
    gpio_set_value(LED_GPIO_PIN, led_state);
    pr_info("Button pressed, LED is now %s\n", led_state ? "ON" : "OFF");

    local_irq_restore(flags);

    return IRQ_HANDLED;
}

static int __init mod_init(void)
{
    int ret;
    pr_info("Initializing the GPIO Button/LED Module\n");

    // Validate the GPIO pins
    if (!gpio_is_valid(BUTTON_GPIO_PIN)) {
        pr_err("Invalid BUTTON GPIO %d\n", BUTTON_GPIO_PIN);
        return -ENODEV;
    }
    if (!gpio_is_valid(LED_GPIO_PIN)) {
        pr_err("Invalid LED GPIO %d\n", LED_GPIO_PIN);
        return -ENODEV;
    }

    // Request the GPIOs
    ret = gpio_request(BUTTON_GPIO_PIN, "BUTTON_GPIO_PIN");
    if (ret) {
        pr_err("Failed to request BUTTON GPIO %d\n", BUTTON_GPIO_PIN);
        return ret;
    }

    ret = gpio_request(LED_GPIO_PIN, "LED_GPIO_PIN");
    if (ret) {
        pr_err("Failed to request LED GPIO %d\n", LED_GPIO_PIN);
        gpio_free(BUTTON_GPIO_PIN);
        return ret;
    }

    // Configure GPIO directions
    gpio_direction_input(BUTTON_GPIO_PIN);  // Button as input
    gpio_direction_output(LED_GPIO_PIN, 0); // LED as output, initially OFF

    // Get the IRQ number for the button GPIO
    irq_number = gpio_to_irq(BUTTON_GPIO_PIN);
    if (irq_number < 0) {
        pr_err("Failed to get IRQ for BUTTON GPIO %d\n", BUTTON_GPIO_PIN);
        gpio_free(BUTTON_GPIO_PIN);
        gpio_free(LED_GPIO_PIN);
        return irq_number;
    }

    pr_info("Button GPIO %d mapped to IRQ %d\n", BUTTON_GPIO_PIN, irq_number);

    // Request the IRQ, triggered on the rising edge (button press)
    ret = request_irq(irq_number,
                      gpio_irq_handler,
                      IRQF_TRIGGER_RISING,
                      "button_gpio_irq",
                      NULL);
    if (ret) {
        pr_err("Failed to request IRQ %d\n", irq_number);
        gpio_free(BUTTON_GPIO_PIN);
        gpio_free(LED_GPIO_PIN);
        return ret;
    }

    pr_info("GPIO Button/LED Module loaded successfully\n");
    return 0;
}

static void __exit mod_exit(void)
{
    pr_info("Exiting the GPIO Button/LED Module\n");

    free_irq(irq_number, NULL);         // Free the IRQ line
    gpio_set_value(LED_GPIO_PIN, 0);    // Turn off the LED
    gpio_free(BUTTON_GPIO_PIN);         // Free the Button GPIO
    gpio_free(LED_GPIO_PIN);            // Free the LED GPIO
}

module_init(mod_init);
module_exit(mod_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ravi");
MODULE_DESCRIPTION("GPIO Button/LED Example with Interrupt Handling");
