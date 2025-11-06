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
  extern void loadcell_config_save2File(const char* filename, loadcell_config &config);

#include <JWT_Menu_FileLogger.h>

// Calibrate Scale 1
  // From Main:
    extern bool menu_cal_scale1;
    extern loadcell_config config_lc1;
  int32_t scale1_read_times(int times = 25);
  void Menu_Scale1_Start();
  void Menu_Scale1_Tare();
  void Menu_Scale1_Scale();
  void Menu_Scale1_Accept();
  void Menu_Scale1_Reset();
  void Menu_Scale1_Cancel();
  void Combine_Scale1_testWeight(int digit1, int digit2, int digit3, int digit4, int digit5);
  void PrepareNextRun();
  int32_t  menu_cal_scale1_tare     =   0;
  float menu_cal_scale1_testWeight  = 0.0;
  float menu_cal_scale1_scaleFactor = 0.0;
  float menu_cal_scale1_measure_val = 0.0;
  bool menu_cal_scale1_testWeight_set = false;
  expFilter menu_cal_scale1_measure;
    #define CAL_MEASURE_FILTER_WEIGHT .75
    bool menu_cal_scale1_measure_first = false;
  // Quadratic LSS Regression
    //#include "JWT_Menu_Quad.h"

// Calibrate Scale 2
  // From Main:
    extern bool menu_cal_scale2;
    extern loadcell_config config_lc1;
  long  menu_cal_scale2_tare        =   0;
  float menu_cal_scale2_testWeight  = 0.0;
  float menu_cal_scale2_scaleFactor = 0.0;
  float menu_cal_scale2_measure_val = 0.0;
  bool menu_cal_scale2_testWeight_set = false;
  expFilter menu_cal_scale2_measure;
    bool menu_cal_scale2_measure_first = false;
  int32_t scale2_read_times(int times = 25);
  void Menu_Scale2_Start();
  void Menu_Scale2_Tare();
  void Menu_Scale2_Scale();
  void Menu_Scale2_Accept();
  void Menu_Scale2_Reset();
  void Menu_Scale2_Cancel();
  void Combine_Scale2_testWeight(int digit1, int digit2, int digit3, int digit4, int digit5);

// Calibrate Pressure
  float menu_cal_press_tare = 0;
  bool menu_cal_press = false;
  float menu_press_airDensity = 1.2;
  float press_read_times(int times = 80); // 80 times @ 40sps is ~2s
  void Menu_Press_SetAirDensity(float density);
  void Menu_Press_Tare();
  void Menu_Press_Accept();
  void Menu_Press_Reset();
  void Menu_Press_Cancel();


#define MENU_POLL_TIME 1000

extern MenuScreen* settingsScreen;
extern MenuScreen* dataLoggerScreen;
extern MenuScreen* calibrateScale1Screen;
extern MenuScreen* calibrateScale2Screen;
extern MenuScreen* calibratePitotScreen;

MENU_SCREEN(mainScreen, mainItems,
  //ITEM_VALUE("Quad1 ", measure_quad, "%.0f"),
  ITEM_VALUE("Scale1", scale1_val, "%.0f"),
  ITEM_VALUE("Scale2", scale2_val, "%.0f"),
  ITEM_VALUE(press_display_name,  press_display, "%.1f"),
  ITEM_VALUE("Logging",FileLogging_display,"%s"),
  ITEM_SUBMENU("Settings", settingsScreen),
  ITEM_SUBMENU("Data Logger", dataLoggerScreen)
);

MENU_SCREEN(settingsScreen, settingsItems,
  ITEM_BACK(),
  //ITEM_TOGGLE("Backlight", true, toggleBacklight),
  //ITEM_SUBMENU("Cal Scale1 Quad", calibrateScale1QuadScreen),
  ITEM_COMMAND("Prepare Next Run", PrepareNextRun),
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
  ITEM_COMMAND("Tare", Menu_Press_Tare),
  ITEM_VALUE("dPres ",  pres_val, "%.1f"),
  ITEM_WIDGET(
        "Density", [](float menu_density) {Menu_Press_SetAirDensity(menu_density);},
        WIDGET_RANGE(menu_press_airDensity, 0.001f, 1.000f, 1.300f, "%0.3f", 0)),
  ITEM_COMMAND("Accept", Menu_Press_Accept),
  ITEM_COMMAND("Reset", Menu_Press_Reset),
  ITEM_COMMAND("Cancel", Menu_Press_Cancel)
  );

void toggleBacklight(bool isOn) {
    lcd.setBacklight(isOn);
}


void PrepareNextRun() {
  // Tare scale 1
  Menu_Scale1_Tare();
  config_lc1.tare = menu_cal_scale1_tare;

  // Tare scale 2
  Menu_Scale2_Start();
  config_lc2.tare = menu_cal_scale2_tare;

  // Tare pitot Tube
  Menu_Press_Tare();
  press_tare = menu_cal_press_tare;

  // finish menu action
  menu_cal_scale1 = false;
  menu_cal_scale2 = false;
  menu_cal_press  = false;
}

// ################################################### Callbacks for Scale1 ###################################################

int32_t scale1_read_times(int times) {
  if (scale1_connected) {
    int num_measurements = 0;
    int32_t cum_measurement = 0;
    while (num_measurements < times) {
      MP.selectChannel(LOADCELL1_CHANNEL);
      while (! nau1.available()) {
        delay(1);
      }
      cum_measurement += nau1.read() >> 4;
      num_measurements++;
      Serial.print(".");
    }
    
    int32_t average_measurement = cum_measurement / num_measurements;
    Serial.print("---> Average Raw Measurement: ");
      Serial.println(average_measurement);
    return average_measurement;
  } else {
    return 0;
  }
  
}

void Combine_Scale1_testWeight(int digit1, int digit2, int digit3, int digit4, int digit5) {
  if (!menu_cal_scale1) {
    Menu_Scale1_Start();
  }
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
  if (scale1_connected) {
    menu_cal_scale1 = true;
    menu_cal_scale1_measure.setWeight(CAL_MEASURE_FILTER_WEIGHT);
    menu_cal_scale1_measure_first = true;
    menu_cal_scale1_scaleFactor = config_lc1.scale;
    //Serial.print("---> New Measure filter weight: ");
    //  Serial.println(menu_cal_scale1_measure.getWeight());
  } else {
    Menu_Scale1_Cancel();
  }
}

void Menu_Scale1_Tare() {
  if (!menu_cal_scale1) {
    Menu_Scale1_Start();
  }
  menu_cal_scale1_tare = scale1_read_times();

  Serial.print("---> Scale1 Tare: ");
    Serial.println(menu_cal_scale1_tare);
}

void Menu_Scale1_Scale() {
  // Scale Factor = {Test Weight} / (Reading - Tare)
  menu_cal_scale1_testWeight_set = true;
  menu_cal_scale1_measure_first = true;
  int32_t scale_reading = scale1_read_times();
  menu_cal_scale1_scaleFactor = menu_cal_scale1_testWeight / (float)(scale_reading - menu_cal_scale1_tare);
  Serial.print("---> Scale1 Scale: ");
    Serial.println(menu_cal_scale1_scaleFactor);
}

void Menu_Scale1_Accept() {
  config_lc1.tare = menu_cal_scale1_tare;
  config_lc1.scale = menu_cal_scale1_scaleFactor;
  menu_cal_scale1 = false;
  menu_cal_scale1_testWeight_set = false;
  menu_cal_scale1_measure.setWeight(LOADCELL_FILTER_WEIGHT);

  loadcell_config_save2File(FILENAME_CONFIG_LC1, config_lc1);

  Serial.print("---> Accepted Tare: ");
    Serial.println(config_lc1.tare);
  Serial.print("---> Accepted Scale: ");
    Serial.println(config_lc1.scale);
  //Serial.print("Assorted booleans: 1.");
  //  Serial.print(menu_cal_scale1);
  //  Serial.print(" 2.");
  //  Serial.println(menu_cal_scale1_testWeight_set);

  menu.setScreen(settingsScreen);
}

void Menu_Scale1_Reset() {
  config_lc1.tare = 0;
  config_lc1.scale = 1;
  menu_cal_scale1 = false;
  menu_cal_scale1_testWeight_set = false;
  menu_cal_scale1_measure.setWeight(LOADCELL_FILTER_WEIGHT);
  loadcell_config_save2File(FILENAME_CONFIG_LC1, config_lc1);
  menu.setScreen(settingsScreen);
}

void Menu_Scale1_Cancel() {
  menu_cal_scale1 = false;
  menu_cal_scale1_testWeight_set = false;
  menu_cal_scale1_measure.setWeight(LOADCELL_FILTER_WEIGHT);
  menu.setScreen(settingsScreen);
}


// ################################################### Callbacks for Scale2 ###################################################

int32_t scale2_read_times(int times) {
  if (scale2_connected) {
    int num_measurements = 0;
    int32_t cum_measurement = 0;
    while (num_measurements < times) {
      MP.selectChannel(LOADCELL2_CHANNEL);
      while (! nau2.available()) {
        delay(1);
      }
      cum_measurement += nau2.read() >> 4;
      num_measurements++;
      Serial.print(".");
    }
    
    int32_t average_measurement = cum_measurement / num_measurements;
    Serial.print("---> Average Raw Measurement: ");
      Serial.println(average_measurement);
    return average_measurement;
  } else {
    return 0;
  }

}

void Combine_Scale2_testWeight(int digit1, int digit2, int digit3, int digit4, int digit5) {
  if (!menu_cal_scale2) {
    Menu_Scale2_Start();
  }
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
  if (scale2_connected) {
    menu_cal_scale2 = true;
    menu_cal_scale2_measure.setWeight(CAL_MEASURE_FILTER_WEIGHT);
    menu_cal_scale2_measure_first = true;
    menu_cal_scale2_scaleFactor = config_lc2.scale;
    //Serial.print("---> New Measure2 filter weight: ");
    //  Serial.println(menu_cal_scale1_measure.getWeight());
  } else {
    Menu_Scale2_Cancel();
  }

}

void Menu_Scale2_Tare() {
  if (!menu_cal_scale2) {
    Menu_Scale2_Start();
  }
  menu_cal_scale2_tare = scale2_read_times();
  Serial.print("---> Scale2 Tare: ");
    Serial.println(menu_cal_scale2_tare);
}

void Menu_Scale2_Scale() {
  // Scale Factor = {Test Weight} / (Reading - Tare)
  menu_cal_scale2_measure_first = true;
  menu_cal_scale2_testWeight_set = true;
  long scale_reading = scale2_read_times();
  menu_cal_scale2_scaleFactor = menu_cal_scale2_testWeight / (float)(scale_reading - menu_cal_scale2_tare);
  Serial.print("---> Scale2 Scale: ");
    Serial.println(menu_cal_scale2_scaleFactor);
}

void Menu_Scale2_Accept() {
  config_lc2.tare = menu_cal_scale2_tare;
  config_lc2.scale = menu_cal_scale2_scaleFactor;
  menu_cal_scale2 = false;
  menu_cal_scale2_testWeight_set = false;
  menu_cal_scale2_measure.setWeight(LOADCELL_FILTER_WEIGHT);

  loadcell_config_save2File(FILENAME_CONFIG_LC2, config_lc2);
  
  Serial.print("---> Accepted Tare: ");
    Serial.println(config_lc2.tare);
  Serial.print("---> Accepted Scale: ");
    Serial.println(config_lc2.scale);
  //Serial.print("Assorted booleans: 1.");
  //  Serial.print(menu_cal_scale2);
  //  Serial.print(" 2.");
  //  Serial.println(menu_cal_scale2_testWeight_set);

  menu.setScreen(settingsScreen);
}

void Menu_Scale2_Reset() {
  config_lc2.tare = 0;
  config_lc2.scale = 1;
  menu_cal_scale2 = false;
  menu_cal_scale2_testWeight_set = false;
  menu_cal_scale2_measure.setWeight(LOADCELL_FILTER_WEIGHT);
  loadcell_config_save2File(FILENAME_CONFIG_LC2, config_lc2);
  menu.setScreen(settingsScreen);  
}

void Menu_Scale2_Cancel() {
  menu_cal_scale2 = false;
  menu_cal_scale2_testWeight_set = false;
  menu_cal_scale2_measure.setWeight(LOADCELL_FILTER_WEIGHT);
  menu.setScreen(settingsScreen);
}

// ################################################### Callbacks for Pressure ###################################################

void Menu_Press_SetAirDensity(float density) {
  menu_press_airDensity = density;
  Serial.print("---> Air Density Entered: ");
    Serial.println(menu_press_airDensity,3);
}

float press_read_times(int times) {
  int num_measurements = 0;
  float cum_measurement = 0;
  while (num_measurements < times) {
    while (!pres.Read()) {
      delay(1);
    }
    cum_measurement += pres.pres_pa();
    num_measurements++;
    Serial.print(".");
  }
  
  float average_measurement = cum_measurement / num_measurements;
  Serial.print("---> Average Raw Measurement: ");
    Serial.println(average_measurement);
  return average_measurement;
}

void Menu_Press_Tare() {
  if (pres_connected) {
    menu_cal_press = true;
    menu_cal_press_tare = press_read_times();

    Serial.print("---> Press Tare: ");
      Serial.println(menu_cal_press_tare);
  } else {
    Menu_Press_Cancel();
  }

}

void Menu_Press_Accept() {
  press_tare = menu_cal_press_tare;
  press_airDensity = menu_press_airDensity;
  menu_cal_press = false;
  Serial.print("---> Accepted Press Tare: ");
    Serial.println(press_tare);
  Serial.print("---> Accepted Air Density: ");
    Serial.println(press_airDensity);
  
  menu.setScreen(settingsScreen);
}

void Menu_Press_Reset() {
  press_tare = 0.0;
  menu_cal_press = false;
  press_airDensity = 0.0;
  menu.setScreen(settingsScreen);
}

void Menu_Press_Cancel() {
  menu_cal_press = false;
  menu_cal_press_tare = 0.0;
  menu.setScreen(settingsScreen);
}