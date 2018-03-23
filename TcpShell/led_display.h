#pragma once

// Display stuff with LEDs.
void led_init();
void led_thinking_on();
void led_thinking_off();
void led_error(ErrorCode error);
void led_display_clear();
void led_display_set_ip(const char* ip);
void led_display_set_message(const char* message);