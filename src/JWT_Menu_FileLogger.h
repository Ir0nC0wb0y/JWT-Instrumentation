#pragma once
#include <ItemBack.h>
#include <ItemBool.h>
#include <ItemCommand.h>
#include <ItemSubMenu.h>
#include <ItemWidget.h>
#include <widget/WidgetList.h>
#include <LcdMenu.h>
#include <MenuScreen.h>

extern LcdMenu menu;

extern MenuScreen* mainScreen;
extern MenuScreen* dataLoggerScreen;
extern MenuScreen* dataLoggeFilenameScreen;

std::vector<const char*> fanspeed = {"_", "low_", "med_", "high_"};
std::vector<const char*> pinChars = {" ", "_", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z"};;

bool start_file_logging = false;
bool stop_file_logging = false;
unsigned long menu_file_write_period = file_write_period;
extern char* Filename_buffer;
void dataLoggerStart();
void dataLoggerStop();
void dataLoggerFilenameAccept();
void dataLoggerFilenameCancel();
void dataLoggerFilenameConcat(const uint8_t char1, // could be as many as 5 characters
                              const uint8_t char2 = 0,
                              const uint8_t char3 = 0,
                              const uint8_t char4 = 0,
                              const uint8_t char5 = 0,
                              const uint8_t char6 = 0,
                              const uint8_t char7 = 0,
                              const uint8_t char8 = 0,
                              const uint8_t char9 = 0,
                              const uint8_t char10 = 0,
                              const uint8_t char11 = 0,
                              const uint8_t char12 = 0,
                              const uint8_t char13 = 0);
void dataLoggerEJECT();
void dataLoggerPeriod(unsigned long period);
extern void SD_Initialization();
void dataLoggerINITSD();


MENU_SCREEN(dataLoggerScreen, dataLoggerItems,
  ITEM_BACK(),
  ITEM_VALUE("Logging",FileLogging_display,"%s"),
  ITEM_SUBMENU("Filename", dataLoggeFilenameScreen),
  ITEM_WIDGET(
        "Write Per [ms]", [](int menu_period) {dataLoggerPeriod(menu_period);},
        WIDGET_RANGE((int)file_write_period, 250, 250, 5000, "%i", 0)),
  ITEM_COMMAND("Start", dataLoggerStart),
  ITEM_COMMAND("EJECT", dataLoggerEJECT)
);

MENU_SCREEN(dataLoggeFilenameScreen, dataLoggeFilenameItems,
  ITEM_BASIC("Filename:"),
  ITEM_WIDGET(
    "",
    [](const uint8_t d1, const uint8_t d2, const uint8_t d3, const uint8_t d4) {dataLoggerFilenameConcat(d1, d2, d3, d4) ; },
    WIDGET_LIST(fanspeed, 0, "%s", 0, true),
    WIDGET_LIST(pinChars, 0, "%s", 0, true),
    WIDGET_LIST(pinChars, 0, "%s", 0, true),
    WIDGET_LIST(pinChars, 0, "%s", 0, true)),
  ITEM_COMMAND("Accept", dataLoggerFilenameAccept),
  ITEM_COMMAND("Cancel", dataLoggerFilenameCancel)
);

void dataLoggerStart() {
  if (strlen(Filename_buffer) == 0) {
    Serial.println("Filename not set, not starting log");
    return;
  }
  start_file_logging = true;
  // Clear Filename buffer
  menu.setScreen(mainScreen);
}

void dataLoggerStop() {
  stop_file_logging = true;
  menu.setScreen(mainScreen);
}

void dataLoggerFilenameAccept() {
  menu.setScreen(dataLoggerScreen);
}

void dataLoggerFilenameCancel() {
  menu.setScreen(mainScreen);
}

void dataLoggerFilenameConcat(const uint8_t char1, // could be as many as 5 characters
                              const uint8_t char2,
                              const uint8_t char3,
                              const uint8_t char4,
                              const uint8_t char5,
                              const uint8_t char6,
                              const uint8_t char7,
                              const uint8_t char8,
                              const uint8_t char9,
                              const uint8_t char10,
                              const uint8_t char11,
                              const uint8_t char12,
                              const uint8_t char13) {

  // Clear existing
  for(int i = 0; i < sizeof(Filename_buffer); i++) {
    Filename_buffer[i] = '\0';
  }

  // Concat all values where character =/= 0
  if (char1 != 0) {
    strcat(Filename_buffer, fanspeed[char1]);
  }
  if (char2 != 0) {
    strcat(Filename_buffer, pinChars[char2]);
  }
  if (char3 != 0) {
    strcat(Filename_buffer, pinChars[char3]);
  }
  if (char4 != 0) {
    strcat(Filename_buffer, pinChars[char4]);
  }
  if (char5 != 0) {
    strcat(Filename_buffer, pinChars[char5]);
  }
  if (char6 != 0) {
    strcat(Filename_buffer, pinChars[char6]);
  }
  if (char7 != 0) {
    strcat(Filename_buffer, pinChars[char7]);
  }
  if (char8 != 0) {
    strcat(Filename_buffer, pinChars[char8]);
  }
  if (char9 != 0) {
    strcat(Filename_buffer, pinChars[char9]);
  }
  if (char10 != 0) {
    strcat(Filename_buffer, pinChars[char10]);
  }
  if (char11 != 0) {
    strcat(Filename_buffer, pinChars[char11]);
  }
  if (char12 != 0) {
    strcat(Filename_buffer, pinChars[char12]);
  }
  if (char13 != 0) {
    strcat(Filename_buffer, pinChars[char13]);
  }
  Serial.print("---> Entered filename: ");
    Serial.println(Filename_buffer);
}


void dataLoggerEJECT() {
  SD.end();
  Serial.println("Ejected SD Card");
  mainScreen -> removeLastItem();
  delay(500);
  digitalWrite(LED_BUILTIN,LOW);
  menu.setScreen(mainScreen);
  mainScreen -> addItem(ITEM_COMMAND("INIT SD",dataLoggerINITSD));
}

void dataLoggerPeriod(unsigned long period) {
  menu_file_write_period = (unsigned long)period;
}

void dataLoggerINITSD() {
  SD_Initialization();
  mainScreen -> removeLastItem();
  mainScreen -> addItem(ITEM_SUBMENU("Data Logger", dataLoggerScreen));
}