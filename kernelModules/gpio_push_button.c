/*
 * gpio_push_button.c
 * author: Venkata Naga Ravikiran Bulusu
 *
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/jiffies.h>

#define BUTTON_GPIO_PIN  17 + GPIO_DYNAMIC_BASE // 17 + 512   // GPIO pin connected to the push button

extern unsigned long volatile jiffies;
unsigned long last_jiffie = 0;
static unsigned int irq_number; // holding the IRQ number

// IRQ handler for button press
static irqreturn_t gpio_irq_handler(int irq, void *dev_id)
{
    unsigned long flags;
    unsigned long diff = jiffies - last_jiffie;

    // Debounce: ignore if the press is too close to the last one
    if (diff < 20) {
        return IRQ_HANDLED;
    }
    last_jiffie = jiffies;

    local_irq_save(flags);
    pr_info("%s(): Button pressed!\n", __func__);
    local_irq_restore(flags);

    return IRQ_HANDLED;
}

static int __init mod_init(void)
{
    int ret;

    pr_info("%s(): Initializing the push button module\n", __func__);

    // Check if the GPIO is valid
    if (!gpio_is_valid(BUTTON_GPIO_PIN)) {
        pr_err("%s(): Invalid GPIO %d\n", __func__, BUTTON_GPIO_PIN);
        return -EIO;
    }

    // Request the GPIO
    ret = gpio_request(BUTTON_GPIO_PIN, "BUTTON_GPIO_PIN");
    if (ret) {
        pr_err("%s(): Failed to request GPIO %d\n", __func__, BUTTON_GPIO_PIN);
        return -EIO;
    }

    // Set the GPIO as an input
    gpio_direction_input(BUTTON_GPIO_PIN);

    // Get the IRQ number for our GPIO
    irq_number = gpio_to_irq(BUTTON_GPIO_PIN);
    if (irq_number < 0) {
        pr_err("%s(): Failed to get IRQ for GPIO %d\n", __func__, BUTTON_GPIO_PIN);
        gpio_free(BUTTON_GPIO_PIN);
        return irq_number;
    }

    pr_info("%s(): Button GPIO %d mapped to IRQ %d\n", __func__, BUTTON_GPIO_PIN, irq_number);

    // Request the IRQ
    ret = request_irq(irq_number,
                      (void*)gpio_irq_handler,
                      IRQF_TRIGGER_RISING,
                      "button_gpio_irq",
                      NULL);
    if (ret) {
        pr_err("%s(): Failed to request IRQ %d\n", __func__, irq_number);
        gpio_free(BUTTON_GPIO_PIN);
        return ret;
    }

    pr_info("%s(): Module loaded successfully\n", __func__);
    return 0;
}

static void __exit mod_exit(void)
{
    pr_info("%s(): Exiting the push button module\n", __func__);
    free_irq(irq_number, NULL); // Free the IRQ line
    gpio_free(BUTTON_GPIO_PIN); // Free the GPIO
}

module_init(mod_init);
module_exit(mod_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ravi");
MODULE_DESCRIPTION("Push button example using GPIO and IRQ");
