#include <Arduino.h>
#include <HX711.h>
#include <ms4525do.h>

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

// SD Card
  #define REQUIRE_SERIAL
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
    #define FILE_WRITE_PERIOD 250
    unsigned long file_write_last = 0;
    char* Filename_buffer = new char[25];


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
  void Weight_Scale2();
  expFilter scale1_filter;
  expFilter scale2_filter;
  unsigned long scale1_display_last = 0;
  unsigned long scale2_display_last = 0;
  #define SCALE_DISPLAY_TIME 1000
  #define SCALE_FILTER_WEIGHT .9
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

  // SD Card Init
    bool sdInitialized = false;
    SPI.setRX(SPI_PIN_MISO);
    SPI.setTX(SPI_PIN_MOSI);
    SPI.setSCK(SPI_PIN_SCK);
    sdInitialized = SD.begin(SPI_PIN_CS);
    if (!sdInitialized) {
      Serial.println("SD initialization failed!");
      return;
    }
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


  scale1.begin(SCALE1_PIN_DOUT, SCALE1_PIN_SCK);
  scale2.begin(SCALE2_PIN_DOUT, SCALE2_PIN_SCK);
  scale1_filter.setWeight(SCALE_FILTER_WEIGHT);
  scale2_filter.setWeight(SCALE_FILTER_WEIGHT);
  bool scale1_read = false;
  bool scale2_read = false;
  while (!scale1_read || !scale2_read) {
    if (scale1.is_ready()) {
      scale1_filter.setValue((float)(scale1.read() >> 8));
      scale1_read = true;
    }
    if (scale2.is_ready()) {
      scale2_filter.setValue((float)(scale2.read() >> 8));
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
  Weight_Scale2();

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

  HandleSDWrite();

}

void Weight_Scale1() {
  if (scale1.is_ready()) {
    long measure_raw = scale1.read() >> 8;
    long measure_tared = measure_raw - cal_scale1_tare;
    float weight = cal_scale1_scale * (float)measure_tared;
    scale1_filter.filter(weight);
    scale1_val = scale1_filter.getValue();

    // Calculate quadratic regression
    long measure_quad_raw = measure_raw - cal_scale1_quad_tare;
    measure_quad = (float)cal_scale1_quad_coeffs[0] * (float)measure_quad_raw * (float)measure_quad_raw * (float)measure_quad_raw +
                   (float)cal_scale1_quad_coeffs[1] * (float)measure_quad_raw * (float)measure_quad_raw +
                   (float)cal_scale1_quad_coeffs[2] * (float)measure_quad_raw +
                   (float)cal_scale1_quad_coeffs[3];

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
      if (millis() - scale1_display_last >= SCALE_DISPLAY_TIME){
        Serial.print("---> Scale1 Measure: ");
          Serial.println(menu_cal_scale1_measure_val);
        scale1_display_last = millis();
      }
    }

    if (menu_cal_scale1_quad) {
      long menu_quad_measure_raw = measure_raw - menu_cal_scale1_quad_tare;
      float menu_quad_measure = (float)menu_cal_scale1_quad_coeffs[0] * (float)menu_quad_measure_raw * (float)menu_quad_measure_raw * (float)menu_quad_measure_raw +
                                (float)menu_cal_scale1_quad_coeffs[1] * (float)menu_quad_measure_raw * (float)menu_quad_measure_raw +
                                (float)menu_cal_scale1_quad_coeffs[2] * (float)menu_quad_measure_raw +
                                (float)menu_cal_scale1_quad_coeffs[3];
      if (menu_cal_scale1_quad_measure_first) {
        menu_cal_scale1_quad_measure.setValue(menu_quad_measure);
        menu_cal_scale1_quad_measure_first = false;
        Serial.print(",");
      } else {
        menu_cal_scale1_quad_measure.filter(menu_quad_measure);
        Serial.print(".");
      }
      menu_cal_scale1_quad_val = menu_cal_scale1_quad_measure.getValue();
      if (millis() - scale1_display_last >= SCALE_DISPLAY_TIME){
        Serial.print("---> Quad1 Measure: ");
          Serial.println(menu_cal_scale1_quad_val);
        scale1_display_last = millis();
      }
    }

    #ifdef SCALE_DISPLAY_TIME
      if (millis() - scale1_display_last >= SCALE_DISPLAY_TIME && !menu_cal_scale1 && !menu_cal_scale2) {
        Serial.print("HX711 scale1 reading: ");
        Serial.println(scale1_val);
        scale1_display_last = millis();
      }
    #endif
  }
}

void Weight_Scale2() {
  if (scale2.is_ready()) {
    long measure_raw = scale2.read() >> 8;
    long measure_tared = measure_raw - cal_scale2_tare;
    float weight = cal_scale2_scale * (float)measure_tared;
    scale2_filter.filter(weight);
    scale2_val = scale2_filter.getValue();
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
      if (millis() - scale2_display_last >= SCALE_DISPLAY_TIME){
        Serial.print("---> Scale2 Measure: ");
          Serial.println(menu_cal_scale2_measure_val);
        scale2_display_last = millis();
      }
    }
    #ifdef SCALE_DISPLAY_TIME
      if (millis() - scale2_display_last >= SCALE_DISPLAY_TIME && !menu_cal_scale1 && !menu_cal_scale2 && !menu_cal_scale1_quad) {
        Serial.print("HX711 scale2 reading: ");
        Serial.println(scale2_val);
        scale2_display_last = millis();
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

char* ResetCharBuffer(char* buffer) {
  for (int i = 0; i < sizeof(buffer); i++) {
    buffer[i] = '\0';
  }
  return buffer;
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
    logFile.println("Time [ms], LoadCell1, LoadCell2, WindSpeed");
    
    file_write_last = millis();
  }

  if (stop_file_logging) {
    // clear stop file flag
    stop_file_logging = false;
    // close file
    logFile.close();
    Serial.println("Stopped logging, Closed file");
  }

  if (FileLogging) {
    if (millis() - file_write_last >= FILE_WRITE_PERIOD) {
      // Write Line
      logFile.print(millis());
        logFile.print(", ");
        logFile.print(scale1_val);
        logFile.print(", ");
        logFile.print(scale2_val);
        logFile.print(", ");
        logFile.println(pres_val);
      file_write_last = millis();
    }
  }
}

void SD_Testing() {
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  File myFile = SD.open("test.txt", FILE_WRITE);

  // if the file opened okay, write to it:
  if (myFile) {
    Serial.print("Writing to test.txt...");
    myFile.println("testing 1, 2, 3.");
    // close the file:
    myFile.close();
    Serial.println("done.");
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }

  // re-open the file for reading:
  myFile = SD.open("test.txt");
  if (myFile) {
    Serial.println("test.txt:");

    // read from the file until there's nothing else in it:
    while (myFile.available()) {
      Serial.write(myFile.read());
    }
    // close the file:
    myFile.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
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