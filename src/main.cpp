#include <Arduino.h>
//#include <HX711.h>
#include <ms4525do.h>
#include <Adafruit_NAU7802.h>
#include <TCA9548.h>
#include <Wire.h>
#include <ArduinoJson.h>

// Libraries for menu system
#define MENU_INPUT_KEYBOARD
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
//#include "JWT_Sensor_NAU7802.h"

// SD Card
  //#define REQUIRE_SERIAL
  #include <SPI.h>
  #include <SD.h>
  #define SPI_PIN_MISO 16  // AKA SPI RX
  #define SPI_PIN_MOSI 19  // AKA SPI TX
  #define SPI_PIN_CS   17
  #define SPI_PIN_SCK  18
  void SD_Testing();
  void printDirectory(File dir, int numTabs);
  File root;
  // Data Logging
    //extern char* Filename_buffer;
    bool FileLogging = false;
    void HandleSDWrite();
    File logFile;
    //#define FILE_WRITE_PERIOD 250
    unsigned long file_write_period = 250;
    unsigned long file_write_last = 0;
    unsigned long file_write_first = 0;
    char* Filename_buffer = new char[25];
    const char* FileLogging_display = "No";
    const char* file_show;



// I2C
  #define I2C0_PIN_SCL 5 // Bus 0
  #define I2C0_PIN_SDA 4 // Bus 0
  #define I2C1_PIN_SCL 3 // Bus 1
  #define I2C1_PIN_SDA 2 // Bus 1

// PCA9548
  TCA9548 MP(0x70, &Wire1);
  uint8_t channels = 0;
  void setupPCA9548();
  void MultiplexI2CScan();

// NAU7802
  expFilter NAU7802_filter1;
  expFilter NAU7802_filter2;
  Adafruit_NAU7802 nau1;
  Adafruit_NAU7802 nau2;
  void setupSensor_NAU7802_loadcell1();
  void printSensor_NAU7802_loadcell1(bool force = false);
  void setupSensor_NAU7802_loadcell2();
  void printSensor_NAU7802_loadcell2(bool force = false);
  unsigned long nau_print1_last = 0;
  unsigned long nau_print2_last = 0;
  #define NAU7802_PRINT_TIME 1000
  #define LOADCELL1_CHANNEL 1
  #define LOADCELL2_CHANNEL 3
  #define LOADCELL_FILTER_WEIGHT .375
  bool scale1_connected = false;
  bool scale2_connected = false;

  //Scale Calibration Vars
    //int32_t  cal_scale1_tare  = 0;
    //float cal_scale1_scale = 1.0;
    //int32_t  cal_scale2_tare  = 0;
    //float cal_scale2_scale = 1.0;
  // Load Cell Config
    struct loadcell_config {
      int32_t tare;
      float   scale;
    };
    loadcell_config config_lc1;
    loadcell_config config_lc2;
    void loadcell_config_readFromFile(const char* filename, loadcell_config &config);
    void loadcell_config_save2File(const char* filename, loadcell_config &config);
    #define FILENAME_CONFIG_LC1 "/config/lc1.cfg"
    #define FILENAME_CONFIG_LC2 "/config/lc2.cfg"

// Pressure Sensor (Pitot)
  bfs::Ms4525do pres;
  bool pres_connected = false;
  expFilter pres_filter;
  unsigned long pres_display_last = 0;
  float press_tare = 0.0;
  float press_airDensity = 0.0;
  float windspeed = 0.0;
  float press_display = 0.0;
  const char* press_display_name = "dPress";
  #define PRES_DISPLAY_TIME 500
  #define PRES_FILTER_WEIGHT .9
  void pressure_setup();
  void pressure_handle();

// Encoder
  #define ENCODER_PIN_SW  21
  #define ENCODER_PIN_CLK 20
  #define ENCODER_PIN_DT  22

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
  bool menu_cal_scale1 = false;
  bool menu_cal_scale2 = false;
  uint16_t val_polling = 0;

void setup() {

  #ifdef REQUIRE_SERIAL
    while (!Serial) {
      delay(1);  // wait for serial port to connect. Needed for native USB port only
    }
  #endif

  Serial.begin(115200);
  Serial.println("Starting sketch");

  SD_Initialization();
  
  Wire.setSDA(I2C0_PIN_SDA);
  Wire.setSCL(I2C0_PIN_SCL);
  Wire.setClock(400000);
  Wire.begin();

  // PCA9548
    Wire1.setSDA(I2C1_PIN_SDA);
    Wire1.setSCL(I2C1_PIN_SCL);
    Wire1.setClock(400000);
    Wire1.begin();
    setupPCA9548();
    //MultiplexI2CScan();

  // NAU7802
    setupSensor_NAU7802_loadcell1();
    setupSensor_NAU7802_loadcell2();
    NAU7802_filter1.setWeight(LOADCELL_FILTER_WEIGHT);
    NAU7802_filter2.setWeight(LOADCELL_FILTER_WEIGHT);
    printSensor_NAU7802_loadcell1(true);
    printSensor_NAU7802_loadcell2(true);
    loadcell_config_readFromFile(FILENAME_CONFIG_LC1, config_lc1);
    loadcell_config_readFromFile(FILENAME_CONFIG_LC2, config_lc2);

  // Pressure Sensor
  pressure_setup();


  // Menu
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

  printSensor_NAU7802_loadcell1();
  printSensor_NAU7802_loadcell2();

  pressure_handle();

  menu.poll(MENU_POLL_TIME);
  //renderer.updateTimer(); //turns off backlight, undesireable
  //delay(500);

  HandleSDWrite();

}

/*
char* ResetCharBuffer(char* buffer) {
  for (int i = 0; i < sizeof(buffer); i++) {
    buffer[i] = '\0';
  }
  return buffer;
}
*/

void SD_Initialization() {
  // SD Card Init
  pinMode(LED_BUILTIN,OUTPUT);
  bool sdInitialized = false;
  SPI.setRX(SPI_PIN_MISO);
  SPI.setTX(SPI_PIN_MOSI);
  SPI.setSCK(SPI_PIN_SCK);
  sdInitialized = SD.begin(SPI_PIN_CS);
  if (!sdInitialized) {
    Serial.println("SD initialization failed!");
    return;
  }
  digitalWrite(LED_BUILTIN,HIGH);
  Serial.println("SD initialization done.");
  root = SD.open("/");
  printDirectory(root, 0);
  Serial.println();
  //SD_Testing();
  Serial.println();
  Serial.println();
  for (int i = 0; i < sizeof(Filename_buffer); i++) {
    Filename_buffer[i] = '\0';
  }
}

void HandleSDWrite() {
  if (start_file_logging) {
    // clear start file flag
    start_file_logging = false;
    // open file
    Serial.println("Filename Buffer: ");
      Serial.println(Filename_buffer);
    
    //char* Filename_extension = new char[30];
    //Serial.print("Filename extension: ");
    //  Serial.println(Filename_extension);
    
    //strcpy(Filename_extension, Filename_buffer);
    //Serial.print("Filename extension: ");
    //  Serial.println(Filename_extension);
    
    bool valid_file = false;
    int filename_version = 0;
    //strcat(Filename_extension, ".txt");
    char* Filename_iteration = new char[40];
    //strcpy(Filename_iteration, Filename_extension);
    while (!valid_file) {
      // clear filename
      for (int i = 0; i < sizeof(Filename_iteration); i++) {
        Filename_iteration[i] = '\0';
      }

      strcpy(Filename_iteration, Filename_buffer);
      strcat(Filename_iteration, ".txt");
      // create filename with extension
      if (filename_version != 0) {
        strcat(Filename_iteration, ".");
        // convert filename_version to char*
        char str_buffer[3];
        sprintf(str_buffer, "%d", filename_version);
        strcat(Filename_iteration, str_buffer);
        
      }
      // check if filename exists
      Serial.print("Searching filename: ");
        Serial.println(Filename_iteration);
      if (SD.exists(Filename_iteration)) {
        filename_version++;
      } else {
        logFile = SD.open(Filename_iteration, FILE_WRITE);
        // Set FileLogging flag
        FileLogging = true;
        Serial.print("Opened file ");
          Serial.println(Filename_iteration);
        valid_file = true;
        break; //break out of loop
      }

      // if no valid file after 16 attempts, fail logger
      if (filename_version > 15) {
        FileLogging = false;
        Serial.print("Logger failed to open file ");
          Serial.println(Filename_iteration);
        break;
      }
    }

    // print header
    logFile.print("Time [ms], Scale1, Scale2, ");
    logFile.println(press_display_name);
    
    file_write_last = millis()-2*file_write_period;

    // Change Menu
      // Remove Filename Menu and "Start" Command
      dataLoggerScreen -> removeLastItem();
      dataLoggerScreen -> removeLastItem();
      dataLoggerScreen -> removeLastItem();
      delay(10);
      // Add "Stop" Command
      file_show = logFile.fullName();
      Serial.print("Filename for menu: ");
        Serial.println(file_show);
      dataLoggerScreen -> addItem(ITEM_VALUE("File",file_show,"%s"));
      dataLoggerScreen -> addItem(ITEM_COMMAND("Stop", dataLoggerStop));
      dataLoggerScreen -> addItem(ITEM_COMMAND("EJECT", dataLoggerEJECT));
  }

  if (stop_file_logging) {
    // clear stop file flag
    stop_file_logging = false;
    // close file
    logFile.close();
    Serial.println("Stopped logging, Closed file");
    FileLogging = false;

    // Change Menu
      // Remove Filename (1) display
      // Remove Stop Command
      dataLoggerScreen -> removeLastItem();
      dataLoggerScreen -> removeLastItem();
      dataLoggerScreen -> removeLastItem();
      delay(10);
      // Add Filename Menu
      dataLoggerScreen -> addItem(ITEM_SUBMENU("Filename", dataLoggeFilenameScreen));
      // Add Start Command
      dataLoggerScreen -> addItem(ITEM_COMMAND("Start", dataLoggerStart));
      dataLoggerScreen -> addItem(ITEM_COMMAND("EJECT", dataLoggerEJECT));

      FileLogging_display = "No";
  }

  if (FileLogging) {
    if (FileLogging_display != "Yes") {
      FileLogging_display = "Yes";
    }
    if (millis() - file_write_last >= file_write_period) {
      if (file_write_first == 0) {
        file_write_first = millis();
        logFile.print("0");
      } else {
        logFile.print(millis()-file_write_first);
      }
      // Write Line
      //logFile.print(millis());
        logFile.print(", ");
        logFile.print(scale1_val);
        logFile.print(", ");
        logFile.print(scale2_val);
        logFile.print(", ");
        logFile.println(press_display);
      file_write_last = file_write_last + file_write_period;
    }
  }
}

void printDirectory(File dir, int numTabs) {
  while (true) {

    File entry = dir.openNextFile();
    if (!entry) {
      // no more files
      break;
    }
    for (uint8_t i = 0; i < numTabs; i++) {
      Serial.print('\t');
    }
    Serial.print(entry.name());
    if (entry.isDirectory()) {
      Serial.println("/");
      printDirectory(entry, numTabs + 1);
    } else {
      // files have sizes, directories do not
      Serial.print("\t\t");
      Serial.print(entry.size(), DEC);
      time_t cr = entry.getCreationTime();
      time_t lw = entry.getLastWrite();
      struct tm* tmstruct = localtime(&cr);
      Serial.printf("\tCREATION: %d-%02d-%02d %02d:%02d:%02d", (tmstruct->tm_year) + 1900, (tmstruct->tm_mon) + 1, tmstruct->tm_mday, tmstruct->tm_hour, tmstruct->tm_min, tmstruct->tm_sec);
      tmstruct = localtime(&lw);
      Serial.printf("\tLAST WRITE: %d-%02d-%02d %02d:%02d:%02d\n", (tmstruct->tm_year) + 1900, (tmstruct->tm_mon) + 1, tmstruct->tm_mday, tmstruct->tm_hour, tmstruct->tm_min, tmstruct->tm_sec);
    }
    entry.close();
  }
}

void loadcell_config_readFromFile(const char* filename, loadcell_config &config) {
  Serial.print("Attempting to read config from: ");
    Serial.println(filename);
  // make sure the config directory exists
  if (!SD.exists("/config")) {
    if (SD.mkdir("/config")) {
      Serial.println("Directory /config created.");
    } else {
      Serial.println("Failed to create directory /config");
    }
    Serial.println("Directory does not exist, exiting");
    return;
  }

  // Open file for reading
  File file = SD.open(filename);

  // Allocate a temporary JsonDocument
  JsonDocument doc;

  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, file);
  if (error) {
    Serial.println(F("Failed to read file, using default configuration"));
    config.tare = 0;
    config.scale = 1.0;
  } else {
    // Copy values from the JsonDocument to the Config
    config.tare = doc["tare"];
    config.scale = doc["scale"];
  }

  Serial.print("Config Tare Value:  ");
    Serial.println(config.tare);
  Serial.print("Config Scale Value: ");
    Serial.println(config.scale);

  // Close the file (Curiously, File's destructor doesn't close the file)
  file.close();
}

void loadcell_config_save2File(const char* filename, loadcell_config &config) {
  Serial.print("Attempting to save config at: ");
    Serial.println(filename);

  // Delete existing file, otherwise the configuration is appended to the file
  if (SD.exists(filename)) {
    SD.remove(filename);
    Serial.println("Old config file exists, deleted");
  }

  // Open file for writing
  if (!SD.exists("/config")) {
    if (SD.mkdir("/config")) {
      Serial.println("Directory /config created.");
    } else {
      Serial.println("Failed to create directory /config");
    }
  }
  File file = SD.open(filename, FILE_WRITE);
  if (!file) {
    Serial.println(F("Failed to create file"));
    return;
  }

  // Allocate a temporary JsonDocument
  JsonDocument doc;

  // Set the values in the document
  //doc["hostname"] = config.hostname;
  //doc["port"] = config.port;
  doc["tare"] = config.tare;
  doc["scale"] = config.scale;

  // Serialize JSON to file
  if (serializeJson(doc, file) == 0) {
    Serial.println(F("Failed to write to file"));
  }

  // Close the file
  file.close();

  // dump contents of file
  File dataFile = SD.open(filename);

  // if the file is available, write to it:
  if (dataFile) {
    while (dataFile.available()) {
      Serial.write(dataFile.read());
    }
    dataFile.close();
    Serial.println();
  }
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening datalog.txt");
  }

}

void setupSensor_NAU7802_loadcell1() {
  //bool found7802 = false;
  MP.selectChannel(LOADCELL1_CHANNEL);
  if (nau1.begin(&Wire1)) {
    //found7802 = true;
    scale1_connected = true;
    Serial.print("... Success!");
      Serial.println();
  } else {
    Serial.println("...FAIL");
  }
  //if (!found7802) {
  //  Serial.println("Failed to find NAU7802");
  //  while (1) delay(10);  // Don't proceed.
  //}

  if (scale1_connected) {
    nau1.setLDO(NAU7802_4V5);
    Serial.print("LDO voltage set to ");
    switch (nau1.getLDO()) {
      case NAU7802_4V5:  Serial.println("4.5V"); break;
      case NAU7802_4V2:  Serial.println("4.2V"); break;
      case NAU7802_3V9:  Serial.println("3.9V"); break;
      case NAU7802_3V6:  Serial.println("3.6V"); break;
      case NAU7802_3V3:  Serial.println("3.3V"); break;
      case NAU7802_3V0:  Serial.println("3.0V"); break;
      case NAU7802_2V7:  Serial.println("2.7V"); break;
      case NAU7802_2V4:  Serial.println("2.4V"); break;
      case NAU7802_EXTERNAL:  Serial.println("External"); break;
    }

    nau1.setGain(NAU7802_GAIN_128);
    Serial.print("Gain set to ");
    switch (nau1.getGain()) {
      case NAU7802_GAIN_1:  Serial.println("1x"); break;
      case NAU7802_GAIN_2:  Serial.println("2x"); break;
      case NAU7802_GAIN_4:  Serial.println("4x"); break;
      case NAU7802_GAIN_8:  Serial.println("8x"); break;
      case NAU7802_GAIN_16:  Serial.println("16x"); break;
      case NAU7802_GAIN_32:  Serial.println("32x"); break;
      case NAU7802_GAIN_64:  Serial.println("64x"); break;
      case NAU7802_GAIN_128:  Serial.println("128x"); break;
    }

    nau1.setRate(NAU7802_RATE_40SPS);
    Serial.print("Conversion rate set to ");
    switch (nau1.getRate()) {
      case NAU7802_RATE_10SPS:  Serial.println("10 SPS"); break;
      case NAU7802_RATE_20SPS:  Serial.println("20 SPS"); break;
      case NAU7802_RATE_40SPS:  Serial.println("40 SPS"); break;
      case NAU7802_RATE_80SPS:  Serial.println("80 SPS"); break;
      case NAU7802_RATE_320SPS:  Serial.println("320 SPS"); break;
    }

    // Take 10 readings to flush out readings
    //for (uint8_t i=0; i<10; i++) {
    //  while (! nau.available()) delay(1);
    //  nau.read();
    //}

    // SINGLE CHANNEL ONLY!!!
    // enable use of PGA stabilizer caps (Cfilter) on VIN2
    nau1.setPGACap(true);
  }
}

void printSensor_NAU7802_loadcell1(bool force) {
  if (scale1_connected) {
    // Multiplex Ops & Sensor Ops
    MP.selectChannel(LOADCELL1_CHANNEL);
    while (! nau1.available()) {
      delay(1);
    }
    int32_t measure_raw = nau1.read() >> 4;

    // Convert measurement to weight
    int32_t measure_tared = measure_raw - config_lc1.tare;
    float weight = config_lc1.scale * (float)measure_tared;
    if (force) {
      NAU7802_filter1.setValue(weight);
    } else {
      NAU7802_filter1.filter(weight);
    }
    scale1_val = NAU7802_filter1.getValue();

    // Menu Ops
    if (menu_cal_scale1 && menu_cal_scale1_testWeight_set) {
      float menu_measure = (float)(measure_raw - menu_cal_scale1_tare) * menu_cal_scale1_scaleFactor;
      if (menu_cal_scale1_measure_first) {
        menu_cal_scale1_measure.setValue(menu_measure);
        menu_cal_scale1_measure_first = false;
        Serial.print(",");
      } else {
        menu_cal_scale1_measure.filter(menu_measure);
        Serial.print(".");
      }
      menu_cal_scale1_measure_val = menu_cal_scale1_measure.getValue();
      if (millis() - nau_print1_last >= NAU7802_PRINT_TIME){
        Serial.print("---> Scale1 Measure: ");
          Serial.println(menu_cal_scale1_measure_val);
        nau_print1_last = millis();
      }
    }

    // Serial Output
    if (millis() - nau_print1_last >= NAU7802_PRINT_TIME && !menu_cal_scale1 && !menu_cal_scale2 && !menu_cal_press) {
      Serial.print("Loadcell1 Reading: ");
        Serial.print(scale1_val);
      //Serial.print(" \t print time: ");
      //  Serial.print(millis() - nau_print1_last);
      if (millis() - nau_print1_last > NAU7802_PRINT_TIME * 2) {
        nau_print1_last = millis();
      } else {
        nau_print1_last = nau_print1_last + NAU7802_PRINT_TIME;
      }
      Serial.println();
    }
  }
}

void setupSensor_NAU7802_loadcell2() {
  //bool found7802 = false;
  MP.selectChannel(LOADCELL2_CHANNEL);
  if (nau2.begin(&Wire1)) {
    //found7802 = true;
    Serial.print("... Success!");
      Serial.println();
  } else {
    Serial.println("...FAIL");
  }
  //if (!found7802) {
  //  Serial.println("Failed to find NAU7802");
  //  while (1) delay(10);  // Don't proceed.
  //}

  if (scale2_connected) {
    nau2.setLDO(NAU7802_4V5);
    Serial.print("LDO voltage set to ");
    switch (nau2.getLDO()) {
      case NAU7802_4V5:  Serial.println("4.5V"); break;
      case NAU7802_4V2:  Serial.println("4.2V"); break;
      case NAU7802_3V9:  Serial.println("3.9V"); break;
      case NAU7802_3V6:  Serial.println("3.6V"); break;
      case NAU7802_3V3:  Serial.println("3.3V"); break;
      case NAU7802_3V0:  Serial.println("3.0V"); break;
      case NAU7802_2V7:  Serial.println("2.7V"); break;
      case NAU7802_2V4:  Serial.println("2.4V"); break;
      case NAU7802_EXTERNAL:  Serial.println("External"); break;
    }

    nau2.setGain(NAU7802_GAIN_128);
    Serial.print("Gain set to ");
    switch (nau2.getGain()) {
      case NAU7802_GAIN_1:  Serial.println("1x"); break;
      case NAU7802_GAIN_2:  Serial.println("2x"); break;
      case NAU7802_GAIN_4:  Serial.println("4x"); break;
      case NAU7802_GAIN_8:  Serial.println("8x"); break;
      case NAU7802_GAIN_16:  Serial.println("16x"); break;
      case NAU7802_GAIN_32:  Serial.println("32x"); break;
      case NAU7802_GAIN_64:  Serial.println("64x"); break;
      case NAU7802_GAIN_128:  Serial.println("128x"); break;
    }

    nau2.setRate(NAU7802_RATE_40SPS);
    Serial.print("Conversion rate set to ");
    switch (nau2.getRate()) {
      case NAU7802_RATE_10SPS:  Serial.println("10 SPS"); break;
      case NAU7802_RATE_20SPS:  Serial.println("20 SPS"); break;
      case NAU7802_RATE_40SPS:  Serial.println("40 SPS"); break;
      case NAU7802_RATE_80SPS:  Serial.println("80 SPS"); break;
      case NAU7802_RATE_320SPS:  Serial.println("320 SPS"); break;
    }

    // Take 10 readings to flush out readings
    //for (uint8_t i=0; i<10; i++) {
    //  while (! nau.available()) delay(1);
    //  nau.read();
    //}

    // SINGLE CHANNEL ONLY!!!
    // enable use of PGA stabilizer caps (Cfilter) on VIN2
    nau2.setPGACap(true);
  }
}

void printSensor_NAU7802_loadcell2(bool force) {
  if (scale1_connected) {
    // Multiplex Ops & Sensor Ops
    MP.selectChannel(LOADCELL2_CHANNEL);
    while (! nau2.available()) {
      delay(1);
    }
    int32_t measure_raw = nau2.read() >> 4;

    // Convert ADC measurement to weight
    long measure_tared = measure_raw - config_lc2.tare;
    float weight = config_lc2.scale * (float)measure_tared;
    if (force) {
      NAU7802_filter2.setValue((float)weight);
    } else {
      NAU7802_filter2.filter((float)weight);
    }
    scale2_val = NAU7802_filter2.getValue();

    // Menu Ops
    if (menu_cal_scale2 && menu_cal_scale2_testWeight_set) {
      float menu_measure = (float)(measure_raw - menu_cal_scale2_tare) * menu_cal_scale2_scaleFactor;
      if (menu_cal_scale2_measure_first) {
        menu_cal_scale2_measure.setValue(menu_measure);
        menu_cal_scale2_measure_first = false;
        Serial.print(",");
      } else {
        menu_cal_scale2_measure.filter(menu_measure);
        Serial.print(".");
      }
      menu_cal_scale2_measure_val = menu_cal_scale2_measure.getValue();
      if (millis() - nau_print2_last >= NAU7802_PRINT_TIME){
        Serial.print("---> Scale2 Measure: ");
          Serial.println(menu_cal_scale2_measure_val);
        nau_print2_last = millis();
      }
    }

    // Serial Output
    if (millis() - nau_print2_last >= NAU7802_PRINT_TIME && !menu_cal_scale1 && !menu_cal_scale2 && !menu_cal_press) {
      Serial.print("Loadcell2 Reading: ");
        Serial.print(scale2_val);
      //Serial.print(" \t print time: ");
      //  Serial.print(millis() - nau_print2_last);
      if (millis() - nau_print2_last > NAU7802_PRINT_TIME * 2) {
        nau_print2_last = millis();
      } else {
        nau_print2_last = nau_print2_last + NAU7802_PRINT_TIME;
      }
      Serial.println();
    }
  }
}

void setupPCA9548() {
  Serial.print("Connection to PCA9548...");
  if (MP.begin()) {
    Serial.println("Success!");
  } else {
    Serial.println("Fail!");
    Serial.println("Halting for PCA9548");
    while(1) {
      delay(1);
    }
  }
}

void MultiplexI2CScan() {
  channels = MP.channelCount();
  //Serial.print("CHAN:\t");
  //Serial.println(channels);

  for (int chan = 0; chan < channels; chan++ ) {
    Serial.print("Selecting channel ");
      Serial.println(chan);
    MP.disableAllChannels();
    MP.selectChannel(chan);
    //  adjust address range to your needs.
    bool channel_good = false;
    for (uint8_t addr = 8; addr < 120; addr++) {
      //if (addr % 10 == 0) Serial.println();
      bool isconnected = MP.isConnected(addr);
      if (isconnected && addr != 0x70) {
        //MP.enableChannel(chan);
        Serial.print("Found device on ");
        Serial.print(addr, HEX);
        Serial.println();
        channel_good = true;
      }
    }
    //Serial.print("Channel Mask: ");
    //  Serial.println(MP.getChannelMask());
    Serial.println();
  }

  Serial.println("done...");
  Serial.println();
}

void pressure_setup() {
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
    Serial.println("Pitot Sensor success!");
  }
}

void pressure_handle() {
  if (pres_connected) {
    if (pres.Read()) {
      pres_filter.filter(pres.pres_pa()-press_tare);
      pres_val = pres_filter.getValue();
      if (press_airDensity > 0) {
        windspeed = sqrt(2*abs(pres_val)/press_airDensity);
        press_display = windspeed;
        press_display_name = "Speed";

      } else {
        press_display = pres_val;
        press_display_name = "dPress";
      }

      #ifdef PRES_DISPLAY_TIME
        if (millis() - pres_display_last >= PRES_DISPLAY_TIME) {
          
          Serial.print("Pressure reading: ");
            Serial.println(pres_val);
          Serial.print("Windspeed: ");
            Serial.print(windspeed);
          pres_display_last = millis();
        }
      #endif
    }
  }

  if (press_airDensity > 0) {
    if (press_display_name != "Speed") {
      press_display_name = "Speed";
      mainScreen -> removeItemAt(2);
      mainScreen -> addItemAt(2,ITEM_VALUE(press_display_name,  press_display, "%.1f"));
      Serial.print("Set press display name to ");
        Serial.println(press_display_name);
    }
  } else {
    if (press_display_name != "dPress") {
      press_display_name = "dPress";
      mainScreen -> removeItemAt(2);
      mainScreen -> addItemAt(2,ITEM_VALUE(press_display_name,  press_display, "%.1f"));
      Serial.print("Set press display name to ");
        Serial.println(press_display_name);
    }
  }
}