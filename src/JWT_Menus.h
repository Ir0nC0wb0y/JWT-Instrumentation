#pragma once
#include <ItemValue.h>
#include <ItemSubMenu.h>
#include <ItemBack.h>
#include <LcdMenu.h>
#include <MenuScreen.h>

float scale1_val = 0.0;
float scale2_val = 0.0;
float pres_val = 0;

#define MENU_POLL_TIME 1000

extern MenuScreen* settingsScreen;

MENU_SCREEN(mainScreen, mainItems,
    ITEM_VALUE("Scale1", scale1_val, "%.0f"),
    ITEM_VALUE("Scale2", scale2_val, "%.0f"),
    ITEM_VALUE("dPres",  pres_val, "%.1f"),
    ITEM_SUBMENU("Settings", settingsScreen));

MENU_SCREEN(settingsScreen, settingsItems,
    ITEM_BACK(),
    ITEM_BASIC("Backlight0"),
    ITEM_BASIC("Backlight1"),
    ITEM_BASIC("Backlight2")
    );