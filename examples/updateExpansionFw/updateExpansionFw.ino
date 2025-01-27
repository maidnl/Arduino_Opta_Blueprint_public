/* -------------------------------------------------------------------------- */
/* FILE NAME:   updateExpansionFw.ino
   AUTHOR:      Daniele Aimo
   EMAIL:       d.aimo@arduino.cc
   DATE:        20240115
   DESCRIPTION: This sketch updates Opta Expansions. 
                Follow instruction on the Serial Monitor.
   LICENSE:     Copyright (c) 2024 Arduino SA
                This Source Code Form is subject to the terms fo the Mozilla
                Public License (MPL), v 2.0. You can obtain a copy of the MPL
                at http://mozilla.org/MPL/2.0/.
   NOTES:       If you are using also an Opta Cellular you can update also this
                device from this sketch. In this case uncomment the  
                #define UPDATE_ARDUINO_OPTA_CELLULAR
                here below                                                    */
/* -------------------------------------------------------------------------- */

/* !!!!! if you have an Opta Cellular un-comment the following define */
//#define UPDATE_ARDUINO_OPTA_CELLULAR

#include "OptaBlue.h"
#include "utility/BossaOpta.h"
#include "fwUpdateDigital.h"
#include "fwUpdateAnalog.h"
#ifdef UPDATE_ARDUINO_OPTA_CELLULAR
#include "fwUpdateCellular.h"
#include "CellularExpansion.h"
#endif
#include "BossaArduino.h"


/* if this is defined the sketch will ask for a confirmation via serial to 
 * actually perform the fw update */
#define ASK_FOR_FW_UPDATE 

using namespace mbed;

UART BossaSerial(D14, D13, NC, NC);

/* opta_analog_fw_update contains the fw update for analog expansion 
 * opta_digital_fw_update_contains the fw update for digital expansion
 * fourth from last bit contains the fw type 
 * last 3 bytes contains the version of the FW expansion */

static uint32_t oa_fw_size = sizeof(opta_analog_fw_update) - 4;
static unsigned char oa_type = opta_analog_fw_update[oa_fw_size];
static unsigned char oa_M = opta_analog_fw_update[oa_fw_size + 1];
static unsigned char oa_m = opta_analog_fw_update[oa_fw_size + 2];
static unsigned char oa_r = opta_analog_fw_update[oa_fw_size + 3];
static unsigned int  oa_version = oa_M * 255 + oa_m * 255 + oa_r;

static uint32_t od_fw_size = sizeof(opta_digital_fw_update) - 4;
static unsigned char od_type = opta_digital_fw_update[od_fw_size];
static unsigned char od_M = opta_digital_fw_update[od_fw_size + 1];
static unsigned char od_m = opta_digital_fw_update[od_fw_size + 2];
static unsigned char od_r = opta_digital_fw_update[od_fw_size + 3];
static unsigned int  od_version = od_M * 255 + od_m * 255 + od_r;

#ifdef UPDATE_ARDUINO_OPTA_CELLULAR
static uint32_t oc_fw_size = sizeof(opta_cellular_fw_update) - 4;
static unsigned char oc_type = opta_cellular_fw_update[oc_fw_size];
static unsigned char oc_M = opta_cellular_fw_update[oc_fw_size + 1];
static unsigned char oc_m = opta_cellular_fw_update[oc_fw_size + 2];
static unsigned char oc_r = opta_cellular_fw_update[oc_fw_size + 3];
static unsigned int  oc_version = oc_M * 255 + oc_m * 255 + oc_r;
#endif

/* print the expansion type */
void printExpansionType(ExpansionType_t t) {
  if(t == EXPANSION_NOT_VALID) {
    Serial.print("Unknown!");
  }
  else if(t == EXPANSION_OPTA_DIGITAL_MEC) {
    Serial.print("Opta --- DIGITAL [Mechanical]  ---");
  }
  else if(t == EXPANSION_OPTA_DIGITAL_STS) {
    Serial.print("Opta --- DIGITAL [Solid State] ---");
  }
  else if(t == EXPANSION_DIGITAL_INVALID) {
    Serial.print("Opta --- DIGITAL [!!Invalid!!] ---");
  }
  else if(t == EXPANSION_OPTA_ANALOG) {
    Serial.print("Opta ~~~ ANALOG ~~~ ");
  }
  #ifdef UPDATE_ARDUINO_OPTA_CELLULAR
  else if (t == OptaController.getExpansionType(CellularExpansion::getProduct())) {
    Serial.print("Opta ~~~ CELLULAR ~~~ ");
  }
  #endif
  else {
    Serial.print("Unknown!");
  }
}

void printVersion(unsigned char M, unsigned char m, unsigned char r) {
   Serial.print(M);
   Serial.print('.');
   Serial.print(m);
   Serial.print('.');
   Serial.print(r);
   Serial.println();
}

/* verify if the device is updatable */
bool isUpdatable(int device) {
   bool rv = false;
   uint8_t M,m,r;
   Serial.print("\nDevice n. ");
   Serial.print(device);
   Serial.print(" type ");
   uint8_t type = OptaController.getExpansionType(device);
   printExpansionType(type);

   if(OptaController.getFwVersion(device,M,m,r)) {
      Serial.print(" Current FW version: ");
      printVersion(M,m,r);
      unsigned int current_version = M * 255 + m * 255 + r;
      if(EXPANSION_OPTA_DIGITAL_MEC == type || EXPANSION_OPTA_DIGITAL_STS == type) {
         if(od_version > current_version ) {
            rv = true;
         }
      } else if(EXPANSION_OPTA_ANALOG == type) {
         if(oa_version > current_version ) {
            rv = true;
         }
      } 
      #ifdef UPDATE_ARDUINO_OPTA_CELLULAR
      else if(OptaController.getExpansionType(CellularExpansion::getProduct()) == type) {
         if(oc_version > current_version ) {
            rv = true;
         }
      }
      #endif
   } else {
      Serial.println("WARNING: unable to get current FW version!");
   }

   if(rv) {
      Serial.println("Device is Updatable");
   }
   else {
      Serial.println("Device is already updated to the last FW version");
   }
  
   return rv;
}

#ifdef UPDATE_ARDUINO_OPTA_CELLULAR
/* look for an Opta Cellular and if present send the command to disable 
   the modem*/
static void find_opta_cellular() {
   Serial.println("Looking for Opta Cellular...");
   for (int i = 0; i < OptaController.getExpansionNum(); i++) {
      CellularExpansion ce = OptaController.getExpansion(i);
      if(ce) {
        Serial.print("  - OPTA CELLULAR FOUND @ index ");
        Serial.println(i);
        /* disable the modem */
        Serial.println("  - Disable modem")
        ce.ctrlModem(false);
        break;
      }
   }
}
#endif

#ifdef ASK_FOR_FW_UPDATE
bool ask_for_confirmation(int i) {

   Serial.println("Expansion " + String(i) + " will be now updated... Proceed? [Y/n]");
   bool first = true;;
   char ans = 'Y';
   while(!Serial.available()) {
      
   }
   while(Serial.available()) {
      if(first) {
         ans = (char)Serial.read();
         first = false;
      }else {
         Serial.read();
      }
   }
   if(ans == 'Y' || ans == 'y') {
      Serial.println("Starting FW Update");
      return true;
   }
   else {
     Serial.println("FW will NOT be updated!");
     return false;
   }
}
#endif

void updateTask() {
   static unsigned long start = millis();
   if(millis() - start > 500) {
      start = millis();
      if(OptaController.getExpansionNum() > 0) {
         for(int i = 0; i < OptaController.getExpansionNum(); i++) {
            if(isUpdatable(i)) {
               unsigned char *fw = nullptr;
               uint32_t sz = 0;

               uint8_t type = OptaController.getExpansionType(i);
               if(EXPANSION_OPTA_DIGITAL_MEC == type || EXPANSION_OPTA_DIGITAL_STS == type) {
                  fw = (unsigned char *)opta_digital_fw_update;
                  sz = od_fw_size;
               } else if(EXPANSION_OPTA_ANALOG == type) {
                  fw = (unsigned char *)opta_analog_fw_update;
                  sz = oa_fw_size;
               } 
               #ifdef UPDATE_ARDUINO_OPTA_CELLULAR
               else if (type == OptaController.getExpansionType(CellularExpansion::getProduct())) {
                  fw = (unsigned char *)opta_cellular_fw_update;
                  sz = oc_fw_size;
               }
               #endif
               
               if(sz == 0 || fw == nullptr) {
                  continue;
               }

               Serial.print("REBOOTING expansion: ");

               if(BOSSA.begin(BossaSerial,&OptaController,i)) {
                  Serial.println("BOSSA correctly initialized");
                  #ifdef ASK_FOR_FW_UPDATE
                  if(ask_for_confirmation(i)) {
                     if(BOSSA.flash(fw,sz)) {
                        Serial.println("UPDATE successfully performed... reset board");
                        BOSSA.reset();
                     }
                     else {
                        BOSSA.reset();
                     }
                  }
                  else {
                     BOSSA.reset();
                  }
                  #else
                  if(BOSSA.flash(fw,sz)) {
                        Serial.println("UPDATE successfully performed... reset board");
                     BOSSA.reset();
                  }
                  #endif
               }
               else {
                  Serial.println("FAILED to initialize BOSSA...");
                  BOSSA.reset();
               }
            }
         }
      }
      else {
         Serial.println("No expansion detected... connect an expansion");
      }
   }
}



/* -------------------------------------------------------------------------- */
/*                                 SETUP                                      */
/* -------------------------------------------------------------------------- */
void setup() {
/* -------------------------------------------------------------------------- */    
  #ifdef UPDATE_ARDUINO_OPTA_CELLULAR
  OptaController.registerCustomExpansion(CellularExpansion::getProduct(),
                                         CellularExpansion::makeExpansion,
                                         CellularExpansion::startUp);
  #endif

  Serial.begin(115200);
  while(!Serial) {
  }

  Serial.println("\n**** OPTA Expansions Updater sketch ****\n");

  Serial.print("This update will update\n  - OPTA ** DIGITAL ** to version: ");
  printVersion(od_M, od_m, od_r);

  Serial.print("  - OPTA ** ANALOG ** to version: ");
  printVersion(oa_M, oa_m, oa_r);
  
  #ifdef UPDATE_ARDUINO_OPTA_CELLULAR
  Serial.print("  - OPTA ** CELLULAR ** to version: ");
  printVersion(oc_M, oc_m, oc_r);
  #endif

  delay(1000);

  OptaController.begin();

  delay(2000); 

  #ifdef UPDATE_ARDUINO_OPTA_CELLULAR
  find_opta_cellular();
  #endif
}

/* -------------------------------------------------------------------------- */
/*                                  LOOP                                      */
/* -------------------------------------------------------------------------- */
void loop() {
/* -------------------------------------------------------------------------- */    
  /* update function will be called each OPTA_CONTROLLER_UPDATE_RATE ms */
  OptaController.update();
  updateTask();
}


