/**
 * LED blink demo using the ESP32 ULP processor. 
 * 
 * Mahesh Viswanathan, Jan 2020, mahesh@leakspotter.com
 * 
 * GPIO2, aka RTC_GPIO12, is toggled high and low. An LED connected to it will blink.
 * Bonus: on some ESP32 boards, the on-board (blue) LED is on GPIO16. If GPIO2 is tied to GPIO16,
 * it reduces the component count and an external LED isn't required. As GPIO16 is not in 
 * the RTC register, the on-board LED cannot otherwise be used by the ULP.
 * 
 * sources synthesized:
 * https://github.com/CoretechR/ESP32-Handheld/tree/master/Memo
 * 
 */

#include "esp_sleep.h"
#include "driver/rtc_io.h"
#include "esp32/ulp.h"
#include "driver/adc.h"

#include "ulpmain.h"
#include "ulptool.h"

#include "esp_wifi.h"
#include "esp_bt.h"

extern const uint8_t ulp_main_bin_start[] asm("_binary_ulp_main_bin_start");
extern const uint8_t ulp_main_bin_end[]   asm("_binary_ulp_main_bin_end");
gpio_num_t ulp_blink_pin = GPIO_NUM_2; // this is also RTC_IO12

/**
 * Save power. from https://desire.giesecke.tk/index.php/2018/02/15/deep-sleep-needs-wifi-switched-off-for-low-current-consumption/
 * To achieve maximum power saving during deep sleep it is apparently necessary to switch off Bluetooth and WiFi before calling esp_deep_sleep_start();
 */
static void power_things_off() {
  // esp_wifi_stop(); // this causes endless rebooting. why?
  esp_bt_controller_disable();  
  adc_power_off();  
}

/**
 * Get the ULP ready to work.
 */
static void init_ulp_program() {
  esp_err_t err = ulptool_load_binary(0, ulp_main_bin_start, (ulp_main_bin_end - ulp_main_bin_start) / sizeof(uint32_t));
  ESP_ERROR_CHECK(err);

  rtc_gpio_init(ulp_blink_pin);
  rtc_gpio_pulldown_dis(ulp_blink_pin); // disable VCOM pulldown (saves 80µA). what's this about?
  rtc_gpio_set_direction(ulp_blink_pin, RTC_GPIO_MODE_OUTPUT_ONLY);

  /* Set ULP wake up period to 500ms */
  ulp_set_wakeup_period(0, 500 * 1000);
}

void enterSleep(){
  Serial.printf("Powering things off, starting ulp, entering deep sleep\n\n");
  power_things_off();
  /* Start the ULP program */
  ESP_ERROR_CHECK( ulp_run((&ulp_entry - RTC_SLOW_MEM) / sizeof(uint32_t)));
  ESP_ERROR_CHECK( esp_sleep_enable_ulp_wakeup() );
  esp_deep_sleep_start();
}

void print_wakeup_cause(esp_sleep_wakeup_cause_t cause) {
  Serial.print("Wakeup cause: ");
  if(cause == ESP_SLEEP_WAKEUP_UNDEFINED) Serial.println("ESP_SLEEP_WAKEUP_UNDEFINED"); //normal reset
  else if(cause == ESP_SLEEP_WAKEUP_ALL) Serial.println("ESP_SLEEP_WAKEUP_ALL");
  else if(cause == ESP_SLEEP_WAKEUP_EXT0) Serial.println("ESP_SLEEP_WAKEUP_EXT0");
  else if(cause == ESP_SLEEP_WAKEUP_EXT1) Serial.println("ESP_SLEEP_WAKEUP_EXT1"); //button A&B
  else if(cause == ESP_SLEEP_WAKEUP_TIMER) Serial.println("ESP_SLEEP_WAKEUP_TIMER"); //scheduled timer wakeup
  else if(cause == ESP_SLEEP_WAKEUP_TOUCHPAD) Serial.println("ESP_SLEEP_WAKEUP_TOUCHPAD");
  else if(cause == ESP_SLEEP_WAKEUP_ULP) Serial.println("ESP_SLEEP_WAKEUP_ULP");
  else if(cause == ESP_SLEEP_WAKEUP_GPIO) Serial.println("ESP_SLEEP_WAKEUP_GPIO");
  else if(cause == ESP_SLEEP_WAKEUP_UART) Serial.println("ESP_SLEEP_WAKEUP_UART"); 
}

/**
 * Entry point
 */
void setup(){
  esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();
  pinMode(0, OUTPUT); // don't change, -10µA???

  Serial.begin(115200);
  delay(100);
  print_wakeup_cause(cause);

  if (cause != ESP_SLEEP_WAKEUP_ULP) {
    // initialize ulp only if the boot cause wasn't the ulp.
    init_ulp_program();
  }
  enterSleep();
}

void loop(void){
  Serial.println("You should never get here.");
}
