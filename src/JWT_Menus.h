#pragma once
#include <ItemCommand.h>
#include <ItemValue.h>
#include <ItemSubMenu.h>
#include <ItemBack.h>
#include <ItemWidget.h>
#include <LcdMenu.h>
#include <MenuScreen.h>
#include <widget/WidgetRange.h>
//#include "expFilter.h"

extern LcdMenu menu;

float scale1_val = 0.0;
float scale2_val = 0.0;
float pres_val = 0;

// Calibrate Scale 1
  // From Main:
    extern long  cal_scale1_tare;
    extern float cal_scale1_scale;
    extern expFilter scale1_filter;
  void Menu_Scale1_Tare();
  void Menu_Scale1_Scale();
  void Menu_Scale1_Accept();
  void Combine_Scale1_testWeight(int digit1, int digit2, int digit3, int digit4, int digit5);
  long  menu_cal_scale1_tare        =   0;
  float menu_cal_scale1_testWeight  = 0.0;
  float menu_cal_scale1_scaleFactor = 0.0;
  float menu_cal_scale1_measure     = 0.0;
// Calibrate Scale 2
  // From Main:
    extern long  cal_scale2_tare;
    extern float cal_scale2_scale;
  float menu_cal_scale2_testWeight  = 0.0;
  float menu_cal_scale2_scaleFactor = 0.0;
  float menu_cal_scale2_measure     = 0.0;

#define MENU_POLL_TIME 1000

extern MenuScreen* settingsScreen;
extern MenuScreen* dataLoggerScreen;
extern MenuScreen* calibrateScale1Screen;
extern MenuScreen* calibrateScale2Screen;
extern MenuScreen* calibratePitotScreen;

MENU_SCREEN(mainScreen, mainItems,
    ITEM_VALUE("Scale1", scale1_val, "%.0f"),
    ITEM_VALUE("Scale2", scale2_val, "%.0f"),
    ITEM_VALUE("dPres ",  pres_val, "%.1f"),
    ITEM_SUBMENU("Settings", settingsScreen),
    ITEM_SUBMENU("Data Logger", dataLoggerScreen));

MENU_SCREEN(settingsScreen, settingsItems,
    ITEM_BACK(),
    ITEM_SUBMENU("Calibrate Scale1", calibrateScale1Screen),
    ITEM_SUBMENU("Calibrate Scale2", calibrateScale2Screen),
    ITEM_SUBMENU("Calibrate Pitot", calibratePitotScreen)
    );

MENU_SCREEN(dataLoggerScreen, dataLoggerItems,
    ITEM_BACK(),
    ITEM_BASIC("Filename"),
    ITEM_BASIC("Start"),
    ITEM_BASIC("Stop")
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
    //ITEM_VALUE("TestWeight",  cal_scale1_testWeight, "%.1f"),
    ITEM_VALUE("Scaling   ", menu_cal_scale1_scaleFactor, "%.2f"),
    ITEM_VALUE("Measure   ",     menu_cal_scale1_measure, "%.1f"),
    ITEM_COMMAND("Accept", Menu_Scale1_Accept),
    ITEM_COMMAND("Remeasure", Menu_Scale1_Scale),
    ITEM_BACK("Cancel"),
    );

MENU_SCREEN(calibrateScale2Screen, calibrateScale2Items,
    ITEM_BASIC("Tare"),
    ITEM_VALUE("TestWeight",  menu_cal_scale2_testWeight, "%.1f"),
    ITEM_VALUE("Scaling   ", menu_cal_scale2_scaleFactor, "%.2f"),
    ITEM_VALUE("Measure   ",     menu_cal_scale2_measure, "%.1f"),
    ITEM_BASIC("Accept"),
    ITEM_BASIC("Remeasure"),
    ITEM_BACK("Cancel"),
    );

MENU_SCREEN(calibratePitotScreen, calibratePitotItems,
    ITEM_BASIC("Tare"),
    ITEM_BACK("Cancel"),
    );

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

void Menu_Scale1_Tare() {
  menu_cal_scale1_tare = scale1.read_average(25);
  Serial.print("---> Scale1 Tare: ");
    Serial.println(menu_cal_scale1_tare);
}

void Menu_Scale1_Scale() {
  // Scale Factor = {Test Weight} / (Reading - Tare)
  long scale_reading = scale1.read_average(25);
  menu_cal_scale1_scaleFactor = menu_cal_scale1_testWeight / (float)(scale_reading - menu_cal_scale1_tare);
  Serial.print("---> Scale1 Scale: ");
    Serial.println(cal_scale1_scale);
}

void Menu_Scale1_Accept() {
  cal_scale1_tare = menu_cal_scale1_tare;
  cal_scale1_scale = menu_cal_scale1_scaleFactor;
  menu.setScreen(settingsScreen);
  scale1_filter.setValue(0);
}