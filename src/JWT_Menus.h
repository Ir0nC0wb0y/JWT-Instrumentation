#pragma once
#include <ItemCommand.h>
#include <ItemValue.h>
#include <ItemSubMenu.h>
#include <ItemBack.h>
#include <ItemWidget.h>
#include <ItemToggle.h>
#include <LcdMenu.h>
#include <MenuScreen.h>
#include <widget/WidgetRange.h>

extern LcdMenu menu;

double measure_quad = 0.0;
float scale1_val = 0.0;
float scale2_val = 0.0;
float pres_val = 0;

// Backlight Control
  extern LiquidCrystal_I2C lcd;
  void toggleBacklight(bool isOn);

#include <JWT_Menu_FileLogger.h>

// Calibrate Scale 1
  // From Main:
    extern bool menu_cal_scale1;
    extern long  cal_scale1_tare;
    extern float cal_scale1_scale;
    extern expFilter scale1_filter;
  void Menu_Scale1_Start();
  void Menu_Scale1_Tare();
  void Menu_Scale1_Scale();
  void Menu_Scale1_Accept();
  void Menu_Scale1_Reset();
  void Menu_Scale1_Cancel();
  void Combine_Scale1_testWeight(int digit1, int digit2, int digit3, int digit4, int digit5);
  long  menu_cal_scale1_tare        =   0;
  float menu_cal_scale1_testWeight  = 0.0;
  float menu_cal_scale1_scaleFactor = 0.0;
  float menu_cal_scale1_measure_val = 0.0;
  bool menu_cal_scale1_testWeight_set = false;
  expFilter menu_cal_scale1_measure;
    #define CAL_MEASURE_FILTER_WEIGHT .75
    bool menu_cal_scale1_measure_first = false;
  // Quadratic LSS Regression
    #include "JWT_Menu_Quad.h"

// Calibrate Scale 2
  // From Main:
    extern bool menu_cal_scale2;
    extern long  cal_scale2_tare;
    extern float cal_scale2_scale;
    extern expFilter scale2_filter;
  long  menu_cal_scale2_tare        =   0;
  float menu_cal_scale2_testWeight  = 0.0;
  float menu_cal_scale2_scaleFactor = 0.0;
  float menu_cal_scale2_measure_val = 0.0;
  bool menu_cal_scale2_testWeight_set = false;
  expFilter menu_cal_scale2_measure;
    bool menu_cal_scale2_measure_first = false;
  void Menu_Scale2_Start();
  void Menu_Scale2_Tare();
  void Menu_Scale2_Scale();
  void Menu_Scale2_Accept();
  void Menu_Scale2_Reset();
  void Menu_Scale2_Cancel();
  void Combine_Scale2_testWeight(int digit1, int digit2, int digit3, int digit4, int digit5);

#define MENU_POLL_TIME 1000

extern MenuScreen* settingsScreen;
extern MenuScreen* dataLoggerScreen;
extern MenuScreen* calibrateScale1Screen;
extern MenuScreen* calibrateScale2Screen;
extern MenuScreen* calibratePitotScreen;

MENU_SCREEN(mainScreen, mainItems,
  ITEM_VALUE("Quad1 ", measure_quad, "%.0f"),
  ITEM_VALUE("Scale1", scale1_val, "%.0f"),
  ITEM_VALUE("Scale2", scale2_val, "%.0f"),
  ITEM_VALUE("dPres ",  pres_val, "%.1f"),
  ITEM_SUBMENU("Settings", settingsScreen),
  ITEM_SUBMENU("Data Logger", dataLoggerScreen)
);

MENU_SCREEN(settingsScreen, settingsItems,
  ITEM_BACK(),
  ITEM_TOGGLE("Backlight", true, toggleBacklight),
  ITEM_SUBMENU("Cal Scale1 Quad", calibrateScale1QuadScreen),
  ITEM_SUBMENU("Calibrate Scale1", calibrateScale1Screen),
  ITEM_SUBMENU("Calibrate Scale2", calibrateScale2Screen),
  ITEM_SUBMENU("Calibrate Pitot", calibratePitotScreen)
);



MENU_SCREEN(calibrateScale1Screen, calibrateScale1Items,
  ITEM_COMMAND("Tare", Menu_Scale1_Tare),
  ITEM_WIDGET(
    "TestWeight",
    [](int cal_scale1_testWeight_digit1, int cal_scale1_testWeight_digit2, int cal_scale1_testWeight_digit3, int cal_scale1_testWeight_digit4, int cal_scale1_testWeight_digit5) { Combine_Scale1_testWeight(cal_scale1_testWeight_digit1, cal_scale1_testWeight_digit2, cal_scale1_testWeight_digit3, cal_scale1_testWeight_digit4, cal_scale1_testWeight_digit5); },
    WIDGET_RANGE(0, 1, 0, 9, "%d", 1),
    WIDGET_RANGE(0, 1, 0, 9, "%d", 1),
    WIDGET_RANGE(0, 1, 0, 9, "%d", 1),
    WIDGET_RANGE(0, 1, 0, 9, "%d", 1),
    WIDGET_RANGE(0, 1, 0, 9, ".%d", 1)),
  ITEM_VALUE("Scaling   ", menu_cal_scale1_scaleFactor, "%.2f"),
  ITEM_VALUE("Measure   ", menu_cal_scale1_measure_val, "%.1f"),
  ITEM_COMMAND("Accept", Menu_Scale1_Accept),
  ITEM_COMMAND("Remeasure", Menu_Scale1_Scale),
  ITEM_COMMAND("Reset", Menu_Scale1_Reset),
  ITEM_COMMAND("Cancel", Menu_Scale1_Cancel)
);

MENU_SCREEN(calibrateScale2Screen, calibrateScale2Items,
  ITEM_COMMAND("Tare", Menu_Scale2_Tare),
  ITEM_WIDGET(
    "TestWeight",
    [](int cal_scale2_testWeight_digit1, int cal_scale2_testWeight_digit2, int cal_scale2_testWeight_digit3, int cal_scale2_testWeight_digit4, int cal_scale2_testWeight_digit5) { Combine_Scale2_testWeight(cal_scale2_testWeight_digit1, cal_scale2_testWeight_digit2, cal_scale2_testWeight_digit3, cal_scale2_testWeight_digit4, cal_scale2_testWeight_digit5); },
    WIDGET_RANGE(0, 1, 0, 9, "%d", 1),
    WIDGET_RANGE(0, 1, 0, 9, "%d", 1),
    WIDGET_RANGE(0, 1, 0, 9, "%d", 1),
    WIDGET_RANGE(0, 1, 0, 9, "%d", 1),
    WIDGET_RANGE(0, 1, 0, 9, ".%d", 1)),
  ITEM_VALUE("Scaling   ", menu_cal_scale2_scaleFactor, "%.2f"),
  ITEM_VALUE("Measure   ", menu_cal_scale2_measure_val, "%.1f"),
  ITEM_COMMAND("Accept", Menu_Scale2_Accept),
  ITEM_COMMAND("Remeasure", Menu_Scale2_Scale),
  ITEM_COMMAND("Reset", Menu_Scale2_Reset),
  ITEM_COMMAND("Cancel", Menu_Scale2_Cancel)
);

MENU_SCREEN(calibratePitotScreen, calibratePitotItems,
  ITEM_BASIC("Tare"),
  ITEM_BACK("Cancel"),
  );

void toggleBacklight(bool isOn) {
    lcd.setBacklight(isOn);
}

// ################################################### Callbacks for Scale1 ###################################################

void Combine_Scale1_testWeight(int digit1, int digit2, int digit3, int digit4, int digit5) {
  menu_cal_scale1_testWeight  = (float)digit1 * 1000.0;
  menu_cal_scale1_testWeight += (float)digit2 * 100.0;
  menu_cal_scale1_testWeight += (float)digit3 * 10.0;
  menu_cal_scale1_testWeight += (float)digit4;
  menu_cal_scale1_testWeight += (float)digit5 * 0.1;
  Serial.print("---> Test Weight: ");
    Serial.println(menu_cal_scale1_testWeight);
  Menu_Scale1_Scale();
}

void Menu_Scale1_Start() {
  menu_cal_scale1 = true;
  menu_cal_scale1_measure.setWeight(CAL_MEASURE_FILTER_WEIGHT);
  menu_cal_scale1_measure_first = true;
  menu_cal_scale1_scaleFactor = cal_scale1_scale;
  //Serial.print("---> New Measure filter weight: ");
  //  Serial.println(menu_cal_scale1_measure.getWeight());
}

void Menu_Scale1_Tare() {
  if (!menu_cal_scale1) {
    Menu_Scale1_Start();
  }
  menu_cal_scale1_tare = scale1.read_average(25) >> 8;
  Serial.print("---> Scale1 Tare: ");
    Serial.println(menu_cal_scale1_tare);
}

void Menu_Scale1_Scale() {
  // Scale Factor = {Test Weight} / (Reading - Tare)
  menu_cal_scale1_testWeight_set = true;
  menu_cal_scale1_measure_first = true;
  long scale_reading = scale1.read_average(25) >> 8;
  menu_cal_scale1_scaleFactor = menu_cal_scale1_testWeight / (float)(scale_reading - menu_cal_scale1_tare);
  Serial.print("---> Scale1 Scale: ");
    Serial.println(menu_cal_scale1_scaleFactor);
}

void Menu_Scale1_Accept() {
  cal_scale1_tare = menu_cal_scale1_tare;
  cal_scale1_scale = menu_cal_scale1_scaleFactor;
  menu_cal_scale1 = false;
  menu_cal_scale1_testWeight_set = false;
  menu.setScreen(settingsScreen);
  scale1_filter.setValue(menu_cal_scale1_measure_val);
  Serial.print("---> Accepted Tare: ");
    Serial.println(cal_scale1_tare);
  Serial.print("---> Accepted Scale: ");
    Serial.println(cal_scale1_scale);
  Serial.print("Assorted booleans: 1.");
    Serial.print(menu_cal_scale1);
    Serial.print(" 2.");
    Serial.println(menu_cal_scale1_testWeight_set);
}

void Menu_Scale1_Reset() {
  cal_scale1_tare = 0;
  cal_scale1_scale = 1;
  menu_cal_scale1 = false;
  menu_cal_scale1_testWeight_set = false;
  menu.setScreen(settingsScreen);
}

void Menu_Scale1_Cancel() {
  menu_cal_scale1 = false;
  menu_cal_scale1_testWeight_set = false;
  menu.setScreen(settingsScreen);
}


// ################################################### Callbacks for Scale2 ###################################################

void Combine_Scale2_testWeight(int digit1, int digit2, int digit3, int digit4, int digit5) {
  menu_cal_scale2_testWeight  = (float)digit1 * 1000.0;
  menu_cal_scale2_testWeight += (float)digit2 * 100.0;
  menu_cal_scale2_testWeight += (float)digit3 * 10.0;
  menu_cal_scale2_testWeight += (float)digit4;
  menu_cal_scale2_testWeight += (float)digit5 * 0.1;
  Serial.print("---> Scale2 Test Weight: ");
    Serial.println(menu_cal_scale2_testWeight);
  Menu_Scale2_Scale();
}

void Menu_Scale2_Start() {
  menu_cal_scale2 = true;
  menu_cal_scale2_measure.setWeight(CAL_MEASURE_FILTER_WEIGHT);
  menu_cal_scale2_measure_first = true;
  menu_cal_scale2_scaleFactor = cal_scale2_scale;
  Serial.print("---> New Measure2 filter weight: ");
    Serial.println(menu_cal_scale1_measure.getWeight());
}

void Menu_Scale2_Tare() {
  if (!menu_cal_scale1) {
    Menu_Scale2_Start();
  }
  menu_cal_scale2_tare = scale2.read_average(25) >> 8;
  Serial.print("---> Scale2 Tare: ");
    Serial.println(menu_cal_scale2_tare);
}

void Menu_Scale2_Scale() {
  // Scale Factor = {Test Weight} / (Reading - Tare)
  menu_cal_scale2_measure_first = true;
  menu_cal_scale2_testWeight_set = true;
  long scale_reading = scale2.read_average(25) >> 8;
  menu_cal_scale2_scaleFactor = menu_cal_scale2_testWeight / (float)(scale_reading - menu_cal_scale2_tare);
  Serial.print("---> Scale2 Scale: ");
    Serial.println(menu_cal_scale2_scaleFactor);
}

void Menu_Scale2_Accept() {
  cal_scale2_tare = menu_cal_scale2_tare;
  cal_scale2_scale = menu_cal_scale2_scaleFactor;
  menu_cal_scale2 = false;
  menu_cal_scale2_testWeight_set = false;
  scale2_filter.setValue(menu_cal_scale2_measure_val);
  menu.setScreen(settingsScreen);
  Serial.print("---> Accepted Tare: ");
    Serial.println(cal_scale2_tare);
  Serial.print("---> Accepted Scale: ");
    Serial.println(cal_scale2_scale);
  Serial.print("Assorted booleans: 1.");
    Serial.print(menu_cal_scale2);
    Serial.print(" 2.");
    Serial.println(menu_cal_scale2_testWeight_set);
}

void Menu_Scale2_Reset() {
  cal_scale2_tare = 0;
  cal_scale2_scale = 1;
  menu_cal_scale2 = false;
  menu_cal_scale2_testWeight_set = false;
  menu.setScreen(settingsScreen);
}

void Menu_Scale2_Cancel() {
  menu_cal_scale2 = false;
  menu_cal_scale2_testWeight_set = false;
  menu.setScreen(settingsScreen);
}