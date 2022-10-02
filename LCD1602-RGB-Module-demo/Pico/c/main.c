#include "lcd1602_RGB_Module.h"
#include <math.h>
int r,g,b,t=0;


int main()
{
    uint8_t r ,g ,b ;
    float t = 0.0;
    LCD1602RGB_init(16,2);
    while(1)
    {
        r = abs((sin(t))*255);
        g = abs((sin(t+60))*255);
        b = abs((sin(t+120))*255);
        t = t + 10;
        setRGB(r,g,b);
        setCursor(0, 0);
        send_string("Waveshare");
        setCursor(0,1);
        send_string("Hello,World!");
        sleep_ms(300);
   }
    return 0;
}
