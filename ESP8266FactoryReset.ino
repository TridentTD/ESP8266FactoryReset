// Coding By TridenTD  (Ven.Phaisarn Techajaruwong)
// FB : ESP8266 IoT 
// @7/5/2559 Buddhist Era
//
// File : ESP8266FactoryReset.ino
// Note :
//   1.Upload to ESP8266
//   2.Waiting to reset
//


#include <ESP8266WiFi.h>
#include <FS.h>

#define FLASH_EEPROM_SIZE 4096
extern "C" {
#include "user_interface.h"
#include "spi_flash.h"
}
extern "C" uint32_t _SPIFFS_start;
extern "C" uint32_t _SPIFFS_end;
extern "C" uint32_t _SPIFFS_page;
extern "C" uint32_t _SPIFFS_block;

#define RTC_BASE 65 // system doc says user area starts at 64, but it does not work (?)
void saveToRTC(byte Par1)
{
  byte buf[3] = {0xAA, 0x55, 0};
  buf[2] = Par1;
  system_rtc_mem_write(RTC_BASE, buf, 3);
}

void EraseFlash()
{
  uint32_t _sectorStart = (ESP.getSketchSize() / SPI_FLASH_SEC_SIZE) + 1;
  uint32_t _sectorEnd = _sectorStart + (ESP.getFlashChipRealSize() / SPI_FLASH_SEC_SIZE);

  for (uint32_t _sector = _sectorStart; _sector < _sectorEnd; _sector++)
  {
    noInterrupts();
    if (spi_flash_erase_sector(_sector) == SPI_FLASH_RESULT_OK)
    {
      interrupts();
      Serial.print(F("[FLASH] Erase Sector: "));
      Serial.println(_sector);
      delay(10);
    }
    interrupts();
  }
}

void ZeroFillFlash()
{
  // this will fill the SPIFFS area with a 64k block of all zeroes.
  uint32_t _sectorStart = ((uint32_t)&_SPIFFS_start - 0x40200000) / SPI_FLASH_SEC_SIZE;
  uint32_t _sectorEnd = _sectorStart + 16 ; //((uint32_t)&_SPIFFS_end - 0x40200000) / SPI_FLASH_SEC_SIZE;
  uint8_t* data = new uint8_t[FLASH_EEPROM_SIZE];

  uint8_t* tmpdata = data;
  for (int x = 0; x < FLASH_EEPROM_SIZE; x++)
  {
    *tmpdata = 0;
    tmpdata++;
  }


  for (uint32_t _sector = _sectorStart; _sector < _sectorEnd; _sector++)
  {
    // write sector to flash
    noInterrupts();
    if (spi_flash_erase_sector(_sector) == SPI_FLASH_RESULT_OK)
      if (spi_flash_write(_sector * SPI_FLASH_SEC_SIZE, reinterpret_cast<uint32_t*>(data), FLASH_EEPROM_SIZE) == SPI_FLASH_RESULT_OK)
      {
        interrupts();
        Serial.print(F("FLASH: Zero Fill Sector: "));
        Serial.println(_sector);
        delay(10);
      }
  }
  interrupts();
  delete [] data;
}

void ESP8266FactoryReset(){
  WiFi.disconnect(true);

  Serial.println("");
  Serial.println("Factory Reseting!");
  Serial.println("[SPIFFS] Please wait for formating SPIFFS.....");
  SPIFFS.format();
  Serial.println("[SPIFFS] formated!");
  Serial.println("-------------------------------");
  EraseFlash();
  ZeroFillFlash();
  saveToRTC(0);
  Serial.println("-------------------------------");
  Serial.println("[FLASH] has cleaned!");
  Serial.println("Your ESP8266 now is like new one.");
}
//////////////////////////////////////////////////////////////////////////////

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  ESP8266FactoryReset();
}

void loop() {
  // put your main code here, to run repeatedly:

}
//////////////////////////////////////////////////////////////////////////////
