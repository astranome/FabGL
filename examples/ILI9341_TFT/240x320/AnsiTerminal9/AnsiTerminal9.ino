 /*
  Created by Fabrizio Di Vittorio (fdivitto2013@gmail.com) - www.fabgl.com
  Copyright (c) 2019-2020 Fabrizio Di Vittorio.
  All rights reserved.

  This file is part of FabGL Library.

  FabGL is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  FabGL is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with FabGL.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "fabgl.h"

fabgl::ILI9341Controller DisplayController;
//fabgl::VGAController DisplayController;
fabgl::PS2Controller PS2Controller;
fabgl::Terminal      Terminal;

#define TFT_SCK    18
#define TFT_MOSI   23
#define TFT_CS     5
#define TFT_DC     22
#define TFT_RESET  21
#define TFT_SPIBUS VSPI_HOST


// notes about GPIO 2 and 12
//    - GPIO2:  may cause problem on programming. GPIO2 must also be either left unconnected/floating, or driven Low, in order to enter the serial bootloader.
//              In normal boot mode (GPIO0 high), GPIO2 is ignored.
//    - GPIO12: should be avoided. It selects flash voltage. To use it disable GPIO12 detection setting efuses with:
//                    python espefuse.py --port /dev/cu.SLAB_USBtoUART set_flash_voltage 3.3V
//                       WARN!! Good for ESP32 with 3.3V voltage (ESP-WROOM-32). This will BRICK your ESP32 if the flash isn't 3.3V
//                       NOTE1: replace "/dev/cu.SLAB_USBtoUART" with your serial port
//                       NOTE2: espefuse.py is downloadable from https://github.com/espressif/esptool
#define UART_RX 34
#define UART_TX 2


#include "confdialog.h"


void setup()
{
  //Serial.begin(115200); delay(500); Serial.write("\n\nReset\n\n"); // DEBUG ONLY

  preferences.begin("AnsiTerminal", false);
  //preferences.clear();

  // only keyboard configured on port 0
  PS2Controller.begin(PS2Preset::KeyboardPort0);

//  DisplayController.begin();
  //DisplayController.setResolution(VGA_640x350_70HzAlt1);
  //DisplayController.setResolution(VGA_640x480_60Hz, 640, 350);
  DisplayController.begin(TFT_SCK, TFT_MOSI, TFT_DC, TFT_RESET, TFT_CS, TFT_SPIBUS);
  DisplayController.setResolution(TFT_240x320);
  DisplayController.setOrientation(fabgl::TFTOrientation::Rotate90);

  Terminal.begin(&DisplayController);
  //Terminal.setLogStream(Serial);  // debug only

  ConfDialogApp::loadConfiguration();
  Terminal.loadFont(&fabgl::FONT_6x10);
  Terminal.clear();
  Terminal.enableCursor(true);

  Terminal.write("* *  FabGL - Serial Terminal                            * *\r\n");
  Terminal.write("* *  2019-2020 by Fabrizio Di Vittorio - www.fabgl.com  * *\r\n\n");
  //Terminal.printf("Screen Size        : %d x %d\r\n", DisplayController.getScreenWidth(), DisplayController.getScreenHeight());
  Terminal.printf("Terminal Size      : %d x %d\r\n", Terminal.getColumns(), Terminal.getRows());
  Terminal.printf("Keyboard           : %s\r\n", PS2Controller.keyboard()->isKeyboardAvailable() ? "OK" : "Error");
  Terminal.printf("Terminal Type      : %s\r\n", SupportedTerminals::names()[(int)ConfDialogApp::getTermType()]);
  //Terminal.printf("Free Memory        : %d bytes\r\n", heap_caps_get_free_size(MALLOC_CAP_32BIT));
  Terminal.write("\r\nPress F12 to change terminal configuration\r\n\n");

  // look for F12 key to open configuration dialog
  Terminal.onVirtualKey = [&](VirtualKey * vk, bool keyDown) {
    if (*vk == VirtualKey::VK_F12) {
      if (!keyDown) {
        Terminal.deactivate();
        auto dlgApp = new ConfDialogApp;
        dlgApp->run(&DisplayController);
        delete dlgApp;
        Terminal.keyboard()->emptyVirtualKeyQueue();
        Terminal.activate();
      }
      *vk = VirtualKey::VK_NONE;
    }
  };
}


void loop()
{
  // the job is done using UART interrupts
  vTaskDelete(NULL);
}
