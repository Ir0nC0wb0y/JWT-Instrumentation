#include <Arduino.h>
#include <HX711.h>
#include <ms4525do.h>

// Libraries for menu system
//#define MENU_INPUT_KEYBOARD
#define MENU_INPUT_ROTARY
  #include <LcdMenu.h>
  #include <MenuScreen.h>
  #include <display/LiquidCrystal_I2CAdapter.h>
  #include <renderer/CharacterDisplayRenderer.h>
  #ifdef MENU_INPUT_KEYBOARD
    #include <input/KeyboardAdapter.h>
  #endif
  #ifdef MENU_INPUT_ROTARY
    #include <SimpleRotary.h>
    #include <input/SimpleRotaryAdapter.h>
  #endif

#include "expFilter.h"

// I2C
  #define I2C_PIN_SCL 5
  #define I2C_PIN_SDA 4

// HX711
  #define SCALE1_PIN_DOUT  10
  #define SCALE1_PIN_SCK   11
  #define SCALE2_PIN_DOUT  12
  #define SCALE2_PIN_SCK   13
  HX711 scale1;
  HX711 scale2;
  void Weight_Scale1();
  expFilter scale1_filter;
  expFilter scale2_filter;
  unsigned long scale1_display_last = 0;
  unsigned long scale2_display_last = 0;
  #define SCALE_DISPLAY_TIME 1000
  #define SCALE_FILTER_WEIGHT .75
  //Scale Calibration Vars
    long  cal_scale1_tare  = 0;
    float cal_scale1_scale = 1.0;
    long  cal_scale2_tare  = 0;
    float cal_scale2_scale = 1.0;
  // Scale Functions
    //void Scale1_Tare();
    //void Scale1_Scale();

// Pressure Sensor (Pitot)
  bfs::Ms4525do pres;
  bool pres_connected = false;
  expFilter pres_filter;
  unsigned long pres_display_last = 0;
  //#define PRES_DISPLAY_TIME 500
  #define PRES_FILTER_WEIGHT .9

// Encoder
  #define ENCODER_PIN_SW  21
  #define ENCODER_PIN_CLK 20
  #define ENCODER_PIN_DT  19

// LCD Menu
  #define LCD_ROWS 4
  #define LCD_COLS 20
  #include "JWT_Menus.h"
  LiquidCrystal_I2C lcd(0x27, LCD_COLS, LCD_ROWS);
  CharacterDisplayRenderer renderer(new LiquidCrystal_I2CAdapter(&lcd), LCD_COLS, LCD_ROWS);
  LcdMenu menu(renderer);
  #ifdef MENU_INPUT_KEYBOARD
    KeyboardAdapter keyboard(&menu, &Serial);
  #endif
  #ifdef MENU_INPUT_ROTARY
    SimpleRotary encoder(ENCODER_PIN_CLK, ENCODER_PIN_DT, ENCODER_PIN_SW);
    SimpleRotaryAdapter rotaryInput(&menu, &encoder);
  #endif
  uint16_t val_polling = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("Starting sketch");

  scale1.begin(SCALE1_PIN_DOUT, SCALE1_PIN_SCK);
  scale2.begin(SCALE2_PIN_DOUT, SCALE2_PIN_SCK);
  scale1_filter.setWeight(SCALE_FILTER_WEIGHT);
  scale2_filter.setWeight(SCALE_FILTER_WEIGHT);
  bool scale1_read = false;
  bool scale2_read = false;
  while (!scale1_read || !scale2_read) {
    if (scale1.is_ready()) {
      scale1_filter.setValue((float)scale1.read());
      scale1_read = true;
    }
    if (scale2.is_ready()) {
      scale2_filter.setValue((float)scale2.read());
      scale2_read = true;
    }
    delay(5);
  }

  Wire.setSDA(I2C_PIN_SDA);
  Wire.setSCL(I2C_PIN_SCL);
  Wire.setClock(400000);
  Wire.begin();
  // I2C address of 0x28, on bus 0, with a -1 to +1 PSI range
  pres.Config(&Wire, 0x28, 1.0f, -1.0f);
  // Starting communication with the pressure transducer
  if (!pres.Begin()) {
    Serial.println("Error communicating with sensor");
    //while(1){
    //  Serial.println("Pressure device not found");
    //  delay(500);
    //}
  } else {
    pres_connected = true;
    pres_filter.setWeight(PRES_FILTER_WEIGHT);
  }

  renderer.begin();
  menu.setScreen(mainScreen);

}

void loop() {
  #ifdef MENU_INPUT_KEYBOARD
    keyboard.observe();
  #endif
  #ifdef MENU_INPUT_ROTARY
    rotaryInput.observe();
  #endif
  
  Weight_Scale1();

  if (scale2.is_ready()) {
    scale2_filter.filter((float)scale2.read());
    scale2_val = scale2_filter.getValue();
    #ifdef SCALE_DISPLAY_TIME
      if (millis() - scale2_display_last >= SCALE_DISPLAY_TIME) {
        Serial.print("HX711 scale2 reading: ");
        Serial.println(scale2_val);
        scale2_display_last = millis();
      }
    #endif
  }

  if (pres_connected) {
    if (pres.Read()) {
      pres_filter.filter(pres.pres_pa());
      pres_val = pres_filter.getValue();
      #ifdef PRES_DISPLAY_TIME
        if (millis() - pres_display_last >= PRES_DISPLAY_TIME) {
          Serial.print("Pressure reading: ");
          Serial.println(pres_val);
          pres_display_last = millis();
        }
      #endif
    }
  }
  menu.poll(MENU_POLL_TIME);
  //delay(500);
}

void Weight_Scale1() {
  if (scale1.is_ready()) {
    long measurement = (float)scale1.read() - cal_scale1_tare;
    float weight = cal_scale1_scale * (float)measurement;
    scale1_filter.filter(weight);
    scale1_val = scale1_filter.getValue();
    #ifdef SCALE_DISPLAY_TIME
      if (millis() - scale1_display_last >= SCALE_DISPLAY_TIME) {
        Serial.print("HX711 scale1 reading: ");
        Serial.println(scale1_val);
        scale1_display_last = millis();
      }
    #endif
  }
}


/*
void Scale1_Tare() {
  cal_scale1_tare = scale1.read_average(25);
  Serial.print("---> Scale1 Tare: ");
    Serial.println(cal_scale1_tare);
}

void Scale1_Scale() {
  // Scale Factor = {Test Weight} / (Reading - Tare)
  long scale_reading = scale1.read_average(25);
  cal_scale1_scale = menu_cal_scale1_testWeight / (float)(scale_reading - cal_scale1_tare);
  Serial.print("---> Scale1 Scale: ");
    Serial.println(cal_scale1_scale);
}
*/