/*
 * gpio_push_button.c
 * author: Venkata Naga Ravikiran Bulusu
 *
 * Hardware Setup (Raspberry Pi 4)
 * --------------------------------
 * Button GPIO: GPIO17 (BCM) = physical pin 11
 *
 * Connections:
 *   3.3V ---- push button ---- GPIO17
 *                              |
 *                            10kΩ
 *                              |
 *                             GND
 *
 * Behavior:
 * - Button press drives GPIO17 HIGH
 * - Rising edge triggers the interrupt
 * - Check output with: dmesg -w
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/jiffies.h>
#include <linux/kernel.h>

#define BUTTON_GPIO_PIN  17 + GPIO_DYNAMIC_BASE // 17 + 512   // GPIO pin connected to the push button

static unsigned int irq_number; // holding the IRQ number
static unsigned long last_jiffie;

/* IRQ handler for button press */
static irqreturn_t gpio_irq_handler(int irq, void *dev_id)
{
    if (time_before(jiffies, last_jiffie + msecs_to_jiffies(200)))
        return IRQ_HANDLED;

    last_jiffie = jiffies;
    pr_info("%s(): Button pressed!\n", __func__);

    return IRQ_HANDLED;
}

static int __init mod_init(void)
{
    int ret;

    pr_info("%s(): Initializing push button module\n", __func__);

    if (!gpio_is_valid(BUTTON_GPIO_PIN)) {
        pr_err("%s(): Invalid GPIO %d\n", __func__, BUTTON_GPIO_PIN);
        return -EINVAL;
    }

    ret = gpio_request(BUTTON_GPIO_PIN, "button_gpio");
    if (ret) {
        pr_err("%s(): Failed to request GPIO %d\n", __func__, BUTTON_GPIO_PIN);
        return ret;
    }

    ret = gpio_direction_input(BUTTON_GPIO_PIN);
    if (ret) {
        pr_err("%s(): Failed to set GPIO %d as input\n", __func__, BUTTON_GPIO_PIN);
        gpio_free(BUTTON_GPIO_PIN);
        return ret;
    }

    irq_number = gpio_to_irq(BUTTON_GPIO_PIN);
    if ((int)irq_number < 0) {
        pr_err("%s(): Failed to map GPIO %d to IRQ\n", __func__, BUTTON_GPIO_PIN);
        gpio_free(BUTTON_GPIO_PIN);
        return (int)irq_number;
    }

    pr_info("%s(): Button GPIO %d mapped to IRQ %d\n",
            __func__, BUTTON_GPIO_PIN, irq_number);

    ret = request_irq(irq_number,
                      gpio_irq_handler,
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
    pr_info("%s(): Exiting push button module\n", __func__);
    free_irq(irq_number, NULL);
    gpio_free(BUTTON_GPIO_PIN);
}

module_init(mod_init);
module_exit(mod_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ravi");
MODULE_DESCRIPTION("Push button example using GPIO and IRQ");
