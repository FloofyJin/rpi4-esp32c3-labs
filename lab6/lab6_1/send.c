#include <stdio.h>
#include <stdlib.h>
#include <gpiod.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

#define GPIO_CHIP "gpiochip0"
#define GPIO_LINE_OFFSET 16
#define DOT_DURATION_MS 20
#define DASH_DURATION_MS (2*DOT_DURATION_MS)

struct morse_code {
    char character;
    const char *code;
};

struct morse_code MORSE_CODE_TABLE[] = {
    {'A',".-"}, {'B', "-..."}, {'C', "-.-."}, {'D', "-.."}, {'E', "."}, {'F', "..-."},
    {'G', "--."}, {'H', "...."}, {'I', ".."}, {'J', ".---"}, {'K', "-.-"}, {'L', ".-.."},
    {'M', "--"}, {'N', "-."}, {'O', "---"}, {'P', ".--."}, {'Q', "--.-"}, {'R', ".-."},
    {'S', "..."}, {'T', "-"}, {'U', "..-"}, {'V', "...-"}, {'W', ".--"}, {'X', "-..-"},
    {'Y', "-.--"}, {'Z', "--.."}, {'0', "-----"}, {'1', ".----"}, {'2', "..---"},
    {'3', "...--"}, {'4', "....-"}, {'5', "....."}, {'6', "-...."}, {'7', "--..."},
    {'8', "---.."}, {'9', "----."}
};

const char *get_morse_code(char character) {
    int table_size = sizeof(MORSE_CODE_TABLE) / sizeof(MORSE_CODE_TABLE[0]);
    for (int i = 0; i < table_size; i++) {
        if (MORSE_CODE_TABLE[i].character == character) {
            return MORSE_CODE_TABLE[i].code;
        }
    }
    return NULL;
}

void blink_dot(struct gpiod_line *line) {
    gpiod_line_set_value(line, 1);
    usleep(DOT_DURATION_MS * 1000);
    gpiod_line_set_value(line, 0);
}

void blink_dash(struct gpiod_line *line) {
    gpiod_line_set_value(line, 1);
    usleep(DASH_DURATION_MS * 1000);
    gpiod_line_set_value(line, 0);
}

void blink_morse_code(struct gpiod_line *line, const char *morse_code) {
    while (*morse_code != '\0') {
        if (*morse_code == '.') {
            blink_dot(line);
        } else if (*morse_code == '-') {
            blink_dash(line);
        }
        morse_code++;
        usleep(DOT_DURATION_MS * 1000);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s <message>\n", argv[0]);
        return 1;
    }

    const int repeat = atoi(argv[1]);

    const char *message = argv[2];
    int message_length = strlen(message);

    struct gpiod_chip *chip;
    struct gpiod_line *line;
    int ret;

    // Open the GPIO chip
    chip = gpiod_chip_open_by_name(GPIO_CHIP);
    if (!chip) {
        perror("Failed to open GPIO chip");
        return 1;
    }

    // Get the GPIO line
    line = gpiod_chip_get_line(chip, GPIO_LINE_OFFSET);
    if (!line) {
        perror("Failed to get GPIO line");
        gpiod_chip_close(chip);
        return 1;
    }

    // Request the GPIO line
    ret = gpiod_line_request_output(line, "blink_led", 0);
    if (ret < 0) {
        perror("Failed to request GPIO line");
        gpiod_chip_close(chip);
        return 1;
    }

    // char b = 'b';
    // setbuf(stdin, b);
    // printf("%c", 'b');

    // Blink the message in Morse code
    for (int j = 0; j < repeat; j++){
        for (int i = 0; i < message_length; i++) {
            char character = message[i];
            if (character == ' '){
                printf("printing: [space]\n");
                usleep(3*DOT_DURATION_MS * 1000);//might need to make 6?
                continue;
            }
            const char *morse_code = get_morse_code(toupper((unsigned char)character));
            printf("flashing: %c\n", message[i]);
            if (morse_code) {
                blink_morse_code(line, morse_code);
                usleep(DOT_DURATION_MS * 1000);
            }
        }
        // usleep(3*DOT_DURATION_MS * 1000);
    }

    // Release the GPIO line
    gpiod_line_release(line);

    // Close the GPIO chip
    gpiod_chip_close(chip);

    return 0;
}
