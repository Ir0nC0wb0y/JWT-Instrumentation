#pragma once
//#include <ItemBack.h>
#include <ItemCommand.h>
//#include <ItemRange.h>
#include <ItemSubMenu.h>
#include <ItemToggle.h>
#include <ItemValue.h>
#include <ItemWidget.h>
#include <LcdMenu.h>
#include <MenuScreen.h>
#include <widget/WidgetRange.h>
#include <curveFitting.h>

extern MenuScreen* settingsScreen;
extern MenuScreen* calibrateScale1QuadScreen;
extern MenuScreen* calibrateScale1QuadValueScreen;


// Scale 1
  // base values
    double scale1_quad = 0;
    long cal_scale1_quad_tare = 0;
    double cal_scale1_quad_coeffs[4] = {0.0, 0.0, 1.0, 0.0};
  // menu values
    expFilter menu_cal_scale1_quad_measure;
    bool   menu_cal_scale1_quad               = false;
    bool   menu_cal_scale1_quad_measure_first = false;
    long   menu_cal_scale1_quad_tare          = 0;
    float  menu_cal_scale1_quad_weight_temp   = 0;
    int    menu_cal_scale1_quad_item          = 0;
    double menu_cal_scale1_quad_val           = 0;
    int    menu_cal_scale1_quad_item_weight[4];
    double menu_cal_scale1_quad_weight[4] = {0.0, 257.0, 893.0, 2345.0};
    double menu_cal_scale1_quad_value[4];
    double menu_cal_scale1_quad_coeffs[4] = {0.0, 0.0, 1.0, 0.0};
  // Callback functions
  void Menu_Scale1_Quad_Start();
  void Menu_Scale1_Quad_Tare();
  void Menu_Scale1_Quad_Calc();
  void Menu_Scale1_Quad_Accept();
  void Menu_Scale1_Quad_Reset();
  void Menu_Scale1_Quad_Cancel();
  void calibrateScale1QuadValueItem(int item);
  void Combine_Scale1_WeightArray(int place, int digit1, int digit2, int digit3, int digit4);
  void Menu_Scale1_Quad_Value_Accept();
  void Menu_Scale1_Quad_Value_Cancel();


MENU_SCREEN(calibrateScale1QuadScreen, calibrateScale1QuadItems,
  ITEM_COMMAND("Tare", Menu_Scale1_Quad_Tare),
  ITEM_SUBMENU("Enter Values", calibrateScale1QuadValueScreen),
  ITEM_VALUE("Weight1", menu_cal_scale1_quad_weight[1], "%.0f"),
  ITEM_VALUE("Value1 ", menu_cal_scale1_quad_value[1], "%.0f"),
  ITEM_VALUE("Weight2", menu_cal_scale1_quad_weight[2], "%.0f"),
  ITEM_VALUE("Value2 ", menu_cal_scale1_quad_value[2], "%.0f"),
  ITEM_VALUE("Weight3", menu_cal_scale1_quad_weight[3], "%.0f"),
  ITEM_VALUE("Value3 ", menu_cal_scale1_quad_value[3], "%.0f"),
  ITEM_COMMAND("Calculate", Menu_Scale1_Quad_Calc),
  ITEM_VALUE("Measure: ", menu_cal_scale1_quad_val, "%.0f"),
  ITEM_COMMAND("Accept", Menu_Scale1_Quad_Accept),
  ITEM_COMMAND("Reset", Menu_Scale1_Quad_Reset),
  ITEM_COMMAND("Cancel", Menu_Scale1_Quad_Cancel)
);

MENU_SCREEN(calibrateScale1QuadValueScreen, calibrateScale1QuadValueItems,
  ITEM_WIDGET(
    "Item",
    [](int quad_value_item) { calibrateScale1QuadValueItem(quad_value_item); },
    WIDGET_RANGE(1, 1, 1, 3, "%d", 0)),
  ITEM_WIDGET(
    "TestWeight",
    [](int cal_scale1_testWeight_digit1, int cal_scale1_testWeight_digit2, int cal_scale1_testWeight_digit3, int cal_scale1_testWeight_digit4) { Combine_Scale1_WeightArray(2,cal_scale1_testWeight_digit1, cal_scale1_testWeight_digit2, cal_scale1_testWeight_digit3, cal_scale1_testWeight_digit4); },
    WIDGET_RANGE(0, 1, 0, 9, "%d", 0),
    WIDGET_RANGE(0, 1, 0, 9, "%d", 0),
    WIDGET_RANGE(0, 1, 0, 9, "%d", 0),
    WIDGET_RANGE(0, 1, 0, 9, "%d", 0)),
  ITEM_VALUE("Value1    ", menu_cal_scale1_quad_val, "%.0f"),
  ITEM_COMMAND("Accept", Menu_Scale1_Quad_Value_Accept),
  ITEM_COMMAND("Cancel", Menu_Scale1_Quad_Value_Cancel)
);

void Menu_Scale1_Quad_Start() {
  menu_cal_scale1_quad = true;
  menu_cal_scale1_quad_measure.setWeight(CAL_MEASURE_FILTER_WEIGHT);
  menu_cal_scale1_quad_measure_first = true;
  menu_cal_scale1_quad_weight[0] = 0.0;
  menu_cal_scale1_quad_value[0]  = 0.0;
  menu_cal_scale1_quad_item      = 1;
  Serial.println("---> Cal Quad START");
}

void Menu_Scale1_Quad_Tare() {
  if (!menu_cal_scale1_quad) {
    Menu_Scale1_Quad_Start();
  }
  menu_cal_scale1_quad_tare = scale1.read_average(25) >> 8;
  Serial.print("---> Scale1 Quad Tare: ");
    Serial.println(menu_cal_scale1_quad_tare);
}

void Menu_Scale1_Quad_Calc() {
  fitCurve(3, 4, menu_cal_scale1_quad_value, menu_cal_scale1_quad_weight, 4, menu_cal_scale1_quad_coeffs);
  menu_cal_scale1_quad_measure_first = true;
  Serial.print("---> Calc Coefficients. a: ");
    Serial.print(menu_cal_scale1_quad_coeffs[0]);
    Serial.print(" b: ");
    Serial.print(menu_cal_scale1_quad_coeffs[1]);
    Serial.print(" c: ");
    Serial.print(menu_cal_scale1_quad_coeffs[2]);
    Serial.print(" d: ");
    Serial.print(menu_cal_scale1_quad_coeffs[3]);
    Serial.println();
}

void Menu_Scale1_Quad_Accept() {
  cal_scale1_quad_coeffs[0] = menu_cal_scale1_quad_coeffs[0];
  cal_scale1_quad_coeffs[1] = menu_cal_scale1_quad_coeffs[1];
  cal_scale1_quad_coeffs[2] = menu_cal_scale1_quad_coeffs[2];

  menu_cal_scale1_quad = false;
  Serial.print("---> Quad ACCEPTED Coefficients. a: ");
    Serial.print(cal_scale1_quad_coeffs[0], 4);
    Serial.print(" b: ");
    Serial.print(cal_scale1_quad_coeffs[1], 4);
    Serial.print(" c: ");
    Serial.print(cal_scale1_quad_coeffs[2], 4);
    Serial.print(" d: ");
    Serial.print(cal_scale1_quad_coeffs[3], 4);
    Serial.println();
  
  menu.setScreen(settingsScreen);
}

void Menu_Scale1_Quad_Reset() {
  cal_scale1_quad_tare = 0;
  cal_scale1_quad_coeffs[0] = 0;
  cal_scale1_quad_coeffs[1] = 0;
  cal_scale1_quad_coeffs[2] = 1;
  cal_scale1_quad_coeffs[3] = 0;

  menu_cal_scale1_quad_tare = 0;
  menu_cal_scale1_quad_coeffs[0] = 0;
  menu_cal_scale1_quad_coeffs[1] = 0;
  menu_cal_scale1_quad_coeffs[2] = 1;
  menu_cal_scale1_quad_coeffs[3] = 0;

  menu_cal_scale1_quad = false;
  menu.setScreen(settingsScreen);
  Serial.println("---> Quad Calibration RESET");
}

void Menu_Scale1_Quad_Cancel() {
  menu_cal_scale1_quad = false;
  menu.setScreen(settingsScreen);
  Serial.println("---> Quad Calibration Cancel");
}

void calibrateScale1QuadValueItem(int item) {
  menu_cal_scale1_quad_item = item;

  // Collect digits of the default item weight
  //cal_scale1_testWeight_digit1 = (int)floor(menu_cal_scale1_quad_weight[item] / 1000);
  //cal_scale1_testWeight_digit2 = (int)floor(menu_cal_scale1_quad_weight[item] / 100) - (int)floor(menu_cal_scale1_quad_weight[item] / 1000)*10;
  //cal_scale1_testWeight_digit3 = (int)floor(menu_cal_scale1_quad_weight[item] / 10) - (int)floor(menu_cal_scale1_quad_weight[item] / 100)*10;
  //cal_scale1_testWeight_digit4 = (int)floor(menu_cal_scale1_quad_weight[item] / 1) - (int)floor(menu_cal_scale1_quad_weight[item] / 10)*10;
}

void Combine_Scale1_WeightArray(int place, int digit1, int digit2, int digit3, int digit4) {
  menu_cal_scale1_quad_weight_temp = digit1 * 1000;
  menu_cal_scale1_quad_weight_temp += digit2 * 100;
  menu_cal_scale1_quad_weight_temp += digit3 * 10;
  menu_cal_scale1_quad_weight_temp += digit4;
}

void Menu_Scale1_Quad_Value_Accept() {
  menu_cal_scale1_quad_weight[menu_cal_scale1_quad_item] = (double)menu_cal_scale1_quad_weight_temp;
  menu_cal_scale1_quad_value[menu_cal_scale1_quad_item]  = (double)menu_cal_scale1_quad_val;
  menu.setScreen(calibrateScale1QuadScreen);
  Serial.print("---> Quad Item ");
    Serial.print(menu_cal_scale1_quad_item);
    Serial.print(" Accepted. Weight: ");
    Serial.print(menu_cal_scale1_quad_weight[menu_cal_scale1_quad_item]);
    Serial.print(" Value: ");
    Serial.println(menu_cal_scale1_quad_value[menu_cal_scale1_quad_item]);
}

void Menu_Scale1_Quad_Value_Cancel() {
  menu_cal_scale1_quad_weight_temp = 0;
  menu_cal_scale1_measure_val = 0;
  menu.setScreen(calibrateScale1QuadScreen);
}