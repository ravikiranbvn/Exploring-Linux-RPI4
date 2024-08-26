/*
 * ssd1306_spi.c
 * author: Venkata Naga Ravikiran Bulusu
 *
 */

// OLED GND → Pin 6 (GND) on Raspberry Pi
// OLED VCC → Pin 1 (3.3V) on Raspberry Pi
// OLED D0 (SCK/CLK) → Pin 23 (SCLK) on Raspberry Pi
// OLED D1 (MOSI) → Pin 19 (MOSI) on Raspberry Pi
// OLED RES → Pin 18 (GPIO 24) on Raspberry Pi
// OLED DC → Pin 22 (GPIO 25) on Raspberry Pi
// OLED CS → Pin 24 (CE0) on Raspberry Pi

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/spi/spidev.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <gpiod.h> // Include libgpiod header

// Define SPI and GPIO settings
#define SPI_PATH "/dev/spidev0.0"
#define SPI_MODE SPI_MODE_0
#define SPI_BITS 8
#define SPI_SPEED 1000000

#define OLED_DC_PIN 25
#define OLED_RESET_PIN 24
#define GPIO_CHIP "gpiochip0" // Adjust if using a different GPIO chip

int spi_fd;
struct gpiod_chip *chip;
struct gpiod_line *dc_line;
struct gpiod_line *reset_line;

// Font table with 5x8 bitmaps for each ASCII character
const uint8_t font5x8[95][5] = {
    // space
    {0x00, 0x00, 0x00, 0x00, 0x00},   
    // !
    {0x00, 0x00, 0x2f, 0x00, 0x00},   
    // "
    {0x00, 0x07, 0x00, 0x07, 0x00},   
    // #
    {0x14, 0x7f, 0x14, 0x7f, 0x14},   
    // $
    {0x24, 0x2a, 0x7f, 0x2a, 0x12},   
    // %
    {0x23, 0x13, 0x08, 0x64, 0x62},   
    // &
    {0x36, 0x49, 0x55, 0x22, 0x50},   
    // '
    {0x00, 0x05, 0x03, 0x00, 0x00},   
    // (
    {0x00, 0x1c, 0x22, 0x41, 0x00},   
    // )
    {0x00, 0x41, 0x22, 0x1c, 0x00},   
    // *
    {0x14, 0x08, 0x3E, 0x08, 0x14},   
    // +
    {0x08, 0x08, 0x3E, 0x08, 0x08},   
    // ,
    {0x00, 0x00, 0xA0, 0x60, 0x00},   
    // -
    {0x08, 0x08, 0x08, 0x08, 0x08},   
    // .
    {0x00, 0x60, 0x60, 0x00, 0x00},   
    // /
    {0x20, 0x10, 0x08, 0x04, 0x02},   
    // 0
    {0x3E, 0x51, 0x49, 0x45, 0x3E},   
    // 1
    {0x00, 0x42, 0x7F, 0x40, 0x00},   
    // 2
    {0x42, 0x61, 0x51, 0x49, 0x46},   
    // 3
    {0x21, 0x41, 0x45, 0x4B, 0x31},   
    // 4
    {0x18, 0x14, 0x12, 0x7F, 0x10},   
    // 5
    {0x27, 0x45, 0x45, 0x45, 0x39},   
    // 6
    {0x3C, 0x4A, 0x49, 0x49, 0x30},   
    // 7
    {0x01, 0x71, 0x09, 0x05, 0x03},   
    // 8
    {0x36, 0x49, 0x49, 0x49, 0x36},   
    // 9
    {0x06, 0x49, 0x49, 0x29, 0x1E},   
    // :
    {0x00, 0x36, 0x36, 0x00, 0x00},   
    // ;
    {0x00, 0x56, 0x36, 0x00, 0x00},   
    // <
    {0x08, 0x14, 0x22, 0x41, 0x00},   
    // =
    {0x14, 0x14, 0x14, 0x14, 0x14},   
    // >
    {0x00, 0x41, 0x22, 0x14, 0x08},   
    // ?
    {0x02, 0x01, 0x51, 0x09, 0x06},   
    // @
    {0x32, 0x49, 0x59, 0x51, 0x3E},   
    // A
    {0x7C, 0x12, 0x11, 0x12, 0x7C},   
    // B
    {0x7F, 0x49, 0x49, 0x49, 0x36},   
    // C
    {0x3E, 0x41, 0x41, 0x41, 0x22},   
    // D
    {0x7F, 0x41, 0x41, 0x22, 0x1C},   
    // E
    {0x7F, 0x49, 0x49, 0x49, 0x41},   
    // F
    {0x7F, 0x09, 0x09, 0x09, 0x01},   
    // G
    {0x3E, 0x41, 0x49, 0x49, 0x7A},   
    // H
    {0x7F, 0x08, 0x08, 0x08, 0x7F},   
    // I
    {0x00, 0x41, 0x7F, 0x41, 0x00},   
    // J
    {0x20, 0x40, 0x41, 0x3F, 0x01},   
    // K
    {0x7F, 0x08, 0x14, 0x22, 0x41},   
    // L
    {0x7F, 0x40, 0x40, 0x40, 0x40},   
    // M
    {0x7F, 0x02, 0x0C, 0x02, 0x7F},   
    // N
    {0x7F, 0x04, 0x08, 0x10, 0x7F},   
    // O
    {0x3E, 0x41, 0x41, 0x41, 0x3E},   
    // P
    {0x7F, 0x09, 0x09, 0x09, 0x06},   
    // Q
    {0x3E, 0x41, 0x51, 0x21, 0x5E},   
    // R
    {0x7F, 0x09, 0x19, 0x29, 0x46},   
    // S
    {0x46, 0x49, 0x49, 0x49, 0x31},   
    // T
    {0x01, 0x01, 0x7F, 0x01, 0x01},   
    // U
    {0x3F, 0x40, 0x40, 0x40, 0x3F},   
    // V
    {0x1F, 0x20, 0x40, 0x20, 0x1F},   
    // W
    {0x3F, 0x40, 0x38, 0x40, 0x3F},   
    // X
    {0x63, 0x14, 0x08, 0x14, 0x63},   
    // Y
    {0x07, 0x08, 0x70, 0x08, 0x07},   
    // Z
    {0x61, 0x51, 0x49, 0x45, 0x43},   
    // [
    {0x00, 0x7F, 0x41, 0x41, 0x00},   
    // Backslash (Checker pattern)
    {0x55, 0xAA, 0x55, 0xAA, 0x55},   
    // ]
    {0x00, 0x41, 0x41, 0x7F, 0x00},   
    // ^
    {0x04, 0x02, 0x01, 0x02, 0x04},   
    // _
    {0x40, 0x40, 0x40, 0x40, 0x40},   
    // `
    {0x00, 0x03, 0x05, 0x00, 0x00},   
    // a
    {0x20, 0x54, 0x54, 0x54, 0x78},   
    // b
    {0x7F, 0x48, 0x44, 0x44, 0x38},   
    // c
    {0x38, 0x44, 0x44, 0x44, 0x20},   
    // d
    {0x38, 0x44, 0x44, 0x48, 0x7F},   
    // e
    {0x38, 0x54, 0x54, 0x54, 0x18},   
    // f
    {0x08, 0x7E, 0x09, 0x01, 0x02},   
    // g
    {0x18, 0xA4, 0xA4, 0xA4, 0x7C},   
    // h
    {0x7F, 0x08, 0x04, 0x04, 0x78},   
    // i
    {0x00, 0x44, 0x7D, 0x40, 0x00},   
    // j
    {0x40, 0x80, 0x84, 0x7D, 0x00},   
    // k
    {0x7F, 0x10, 0x28, 0x44, 0x00},   
    // l
    {0x00, 0x41, 0x7F, 0x40, 0x00},   
    // m
    {0x7C, 0x04, 0x18, 0x04, 0x78},   
    // n
    {0x7C, 0x08, 0x04, 0x04, 0x78},   
    // o
    {0x38, 0x44, 0x44, 0x44, 0x38},   
    // p
    {0xFC, 0x24, 0x24, 0x24, 0x18},   
    // q
    {0x18, 0x24, 0x24, 0x18, 0xFC},   
    // r
    {0x7C, 0x08, 0x04, 0x04, 0x08},   
    // s
    {0x48, 0x54, 0x54, 0x54, 0x20},   
    // t
    {0x04, 0x3F, 0x44, 0x40, 0x20},   
    // u
    {0x3C, 0x40, 0x40, 0x20, 0x7C},   
    // v
    {0x1C, 0x20, 0x40, 0x20, 0x1C},   
    // w
    {0x3C, 0x40, 0x30, 0x40, 0x3C},   
    // x
    {0x44, 0x28, 0x10, 0x28, 0x44},   
    // y
    {0x1C, 0xA0, 0xA0, 0xA0, 0x7C},   
    // z
    {0x44, 0x64, 0x54, 0x4C, 0x44},   
    // {
    {0x00, 0x10, 0x7C, 0x82, 0x00},   
    // |
    {0x00, 0x00, 0xFF, 0x00, 0x00},   
    // }
    {0x00, 0x82, 0x7C, 0x10, 0x00},   
    // ~ (Degrees)
    {0x00, 0x06, 0x09, 0x09, 0x06}    
};

// Function prototypes
void gpio_write(struct gpiod_line *line, int value);
void ssd1306_command(uint8_t command);
void ssd1306_data(uint8_t data);
void ssd1306_init();
void ssd1306_clear();
void ssd1306_draw_char(uint8_t x, uint8_t y, char ch);
void ssd1306_draw_string(uint8_t x, uint8_t y, const char* str);

void gpio_write(struct gpiod_line *line, int value) {
    if (gpiod_line_set_value(line, value) < 0) {
        perror("Failed to write GPIO value");
    }
}

void ssd1306_command(uint8_t command) {
    gpio_write(dc_line, 0);  // Set DC low for command
    if (write(spi_fd, &command, 1) != 1) {
        perror("Failed to write command to SPI");
    }
}

void ssd1306_data(uint8_t data) {
    gpio_write(dc_line, 1);  // Set DC high for data
    if (write(spi_fd, &data, 1) != 1) {
        perror("Failed to write data to SPI");
    } else {
        printf("Data sent: 0x%02X\n", data); // Debug print
    }
}

void ssd1306_init() {
    gpio_write(reset_line, 0);
    usleep(10000); // 10ms delay
    gpio_write(reset_line, 1);

    // Initialization sequence
    ssd1306_command(0xAE); // Display off
    ssd1306_command(0xD5); // Set display clock divide ratio/oscillator frequency
    ssd1306_command(0x80); // Default setting
    ssd1306_command(0xA8); // Set multiplex ratio
    ssd1306_command(0x3F); // 1/64 duty
    ssd1306_command(0xD3); // Set display offset
    ssd1306_command(0x00); // No offset
    ssd1306_command(0x40); // Set start line address
    ssd1306_command(0x8D); // Enable charge pump regulator
    ssd1306_command(0x14); // Enable
    ssd1306_command(0x20); // Set memory addressing mode
    ssd1306_command(0x00); // Horizontal addressing mode
    ssd1306_command(0xA1); // Set segment re-map
    ssd1306_command(0xC8); // Set COM output scan direction
    ssd1306_command(0xDA); // Set COM pins hardware configuration
    ssd1306_command(0x12); // Alternative COM pin configuration
    ssd1306_command(0x81); // Set contrast control
    ssd1306_command(0xCF); // Maximum contrast
    ssd1306_command(0xD9); // Set pre-charge period
    ssd1306_command(0xF1); // Phase 1: 15 DCLKs, Phase 2: 1 DCLK
    ssd1306_command(0xDB); // Set VCOMH deselect level
    ssd1306_command(0x40); // VCOMH = 0.77*VCC
    ssd1306_command(0xA4); // Resume to RAM content display
    ssd1306_command(0xA6); // Normal display
    ssd1306_command(0xAF); // Display on
}

void ssd1306_clear() {
    for (int i = 0; i < 1024; i++) {
        ssd1306_data(0x00);
    }
}

void ssd1306_set_cursor(uint8_t x, uint8_t y) {
    ssd1306_command(0xB0 + y); // Set page address
    ssd1306_command(((x & 0xF0) >> 4) | 0x10); // Set high column address
    ssd1306_command((x & 0x0F) | 0x00); // Set low column address
}

void ssd1306_draw_char(uint8_t x, uint8_t y, char ch) {
    if (ch < ' ' || ch > '~') {
        ch = ' '; // Default to space if character out of range
    }
    ch -= ' ';
    ssd1306_set_cursor(x, y);
    for (int i = 0; i < 5; i++) {
        ssd1306_data(font5x8[ch][i]);
    }
    ssd1306_data(0x00); // Add space after character
}

void ssd1306_draw_string(uint8_t x, uint8_t y, const char* str) {
    while (*str) {
        ssd1306_draw_char(x, y, *str++);
        x += 6; // Move cursor to the next character position
        if (x >= 128) { // Wrap around
            x = 0;
            y++;
            if (y >= 8) {
                y = 0; // Start over from top
            }
        }
        ssd1306_set_cursor(x, y);
    }
}

int main() {
    // Open SPI device
    spi_fd = open(SPI_PATH, O_RDWR);
    if (spi_fd < 0) {
        perror("Failed to open SPI device");
        return EXIT_FAILURE;
    }

    // Configure SPI
    uint8_t mode = SPI_MODE;
    uint8_t bits = SPI_BITS;
    uint32_t speed = SPI_SPEED;
    if (ioctl(spi_fd, SPI_IOC_WR_MODE, &mode) == -1 ||
        ioctl(spi_fd, SPI_IOC_WR_BITS_PER_WORD, &bits) == -1 ||
        ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) == -1) {
        perror("Failed to configure SPI");
        close(spi_fd);
        return EXIT_FAILURE;
    }

    // Initialize libgpiod
    chip = gpiod_chip_open_by_name(GPIO_CHIP);
    if (!chip) {
        perror("Failed to open GPIO chip");
        close(spi_fd);
        return EXIT_FAILURE;
    }

    dc_line = gpiod_chip_get_line(chip, OLED_DC_PIN);
    if (!dc_line) {
        perror("Failed to get DC GPIO line");
        gpiod_chip_close(chip);
        close(spi_fd);
        return EXIT_FAILURE;
    }

    reset_line = gpiod_chip_get_line(chip, OLED_RESET_PIN);
    if (!reset_line) {
        perror("Failed to get RESET GPIO line");
        gpiod_chip_close(chip);
        close(spi_fd);
        return EXIT_FAILURE;
    }

    // Set GPIO lines as outputs
    if (gpiod_line_request_output(dc_line, "ssd1306", 0) < 0 ||
        gpiod_line_request_output(reset_line, "ssd1306", 0) < 0) {
        perror("Failed to request GPIO lines as outputs");
        gpiod_chip_close(chip);
        close(spi_fd);
        return EXIT_FAILURE;
    }

    ssd1306_init();
    ssd1306_clear();

    ssd1306_draw_string(0, 0, "Hi chuchulu!!!!");

    sleep(10);

    // Cleanup
    gpiod_chip_close(chip);
    close(spi_fd);
    return EXIT_SUCCESS;
}
