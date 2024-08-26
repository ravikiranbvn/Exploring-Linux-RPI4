/*
 * led_gpio17.c
 * author: Venkata Naga Ravikiran Bulusu
 *
 */

#include <gpiod.h>
#include <stdio.h>
#include <unistd.h>

int main() {
    const char *chipname = "gpiochip0";
    unsigned int line_num = 17; // GPIO pin number
    int val = 1; // Value to set

    struct gpiod_chip *chip = gpiod_chip_open_by_name(chipname);
    if (!chip) {
        perror("Open chip failed\n");
        return -1;
    }

    struct gpiod_line *line = gpiod_chip_get_line(chip, line_num);
    if (!line) {
        perror("Get line failed\n");
        gpiod_chip_close(chip);
        return -1;
    }

    if (gpiod_line_request_output(line, "gpio_control", val) < 0) {
        perror("Request line as output failed\n");
        gpiod_chip_close(chip);
        return -1;
    }

    gpiod_line_set_value(line, 1); // Turn on LED
    sleep(2);                      // Keep LED on for 2 seconds
    gpiod_line_set_value(line, 0); // Turn off LED

    gpiod_chip_close(chip);
    return 0;
}