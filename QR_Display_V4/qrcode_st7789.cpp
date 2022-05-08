#include <Arduino.h>
#include "qrencode.h"
#include "qrcode_st7789.h"


QRcode_ST7789::QRcode_ST7789(Adafruit_ST7789 *display) {
    this->display = display;
}


void QRcode_ST7789::init() {
    display->setRotation(3);
    this->screenwidth = display->width();
    this->screenheight = display->height();
    display->fillScreen(ST77XX_WHITE);
    int min = screenwidth;
    if (screenheight<screenwidth)
        min = screenheight;
    multiply = min/WD;
//    if  (multiply>1)
//        multiply = 2;
    if(this->QRDEBUG){
      Serial.print(F("WD="));
      Serial.println(WD);
      Serial.print(F("Multiply="));
      Serial.println(multiply);
    }
    offsetsX = (screenwidth-(WD*multiply))/2;
    offsetsY = (screenheight-(WD*multiply))/2;
}

void QRcode_ST7789::screenwhite() {
    display->fillScreen(ST77XX_WHITE);
}

void QRcode_ST7789::screenupdate() {
    // No hay que hacer nada
}

void QRcode_ST7789::drawPixel(int x, int y, int color) {
    if(color==1) {
        color = ST77XX_BLACK;
    } else {
        color = ST77XX_WHITE;
    }
    display->fillRect(x,y,multiply,multiply,color);
//    display->drawPixel(x,y,color);
//    if (this->multiply>1) {
//        display->drawPixel(x+1,y,color);
//        display->drawPixel(x+1,y+1,color);
//        display->drawPixel(x,y+1,color);
//    }
}
