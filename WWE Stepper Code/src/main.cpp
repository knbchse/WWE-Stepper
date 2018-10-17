// Wire Winding Enterprises (WWE) main code
// Author: Hamish Simmonds (The man from Levin)

#include "mbed.h"
#include "math.h"

#define LCD_DATA 1
#define LCD_INSTRUCTION 0
void Initialise();
int Interface(char *str, int maxDigits, int minBoundary, int maxBoundary);
int square(int base, int power);
void lcdCommand(unsigned char command);
void lcdPutChar(unsigned char c);
void setcol1();
void setcol2();
void setcol3();
void setcolhigh();
void lcdPutString(char *s);
int keyscan(int max);
static void lcdSetRS(int mode); //-- mode is either LCD_DATA or LCD_INSTRUCTION
static void lcdPulseEN(void);
static void lcdInit8Bit(unsigned char command);
//-- the first few commands of initialisation, are still in pseudo 8-bit data mode
DigitalOut lcdD4(PA_9), lcdD5(PA_8), lcdD6(PB_10), lcdD7(PB_4);
DigitalOut lcdEN(PC_7), lcdRS(PB_6);

DigitalIn row1(PA_7), row2(PA_6), row3(PA_5), row4(PB_9);
DigitalOut col1(PB_8), col2(PC_9), col3(PC_11), col4(PD_2);

//===[Stepper Class]===
class Stepper {
private:
    InterruptIn endStop;
    DigitalOut dirPin;      // stepper direction pin
    DigitalOut stepPin;     // stepper atep pin
    bool direction;     // direction (0-1)
    bool on;            // true-motor on, false-motor off
    unsigned int pulseCount;        // no of pulses
    unsigned int stepMode;
    float delay;          // delay between switching magnets (in us)
    Ticker t;           // ticker timer


public:
    Stepper(PinName p0, PinName p1, PinName p2, bool d, unsigned int p, unsigned int s):        // direction pin, step pin, direction, delay(us)
        dirPin(p0), stepPin(p1), endStop(p2)       // setup stepper pins
    {
        dirPin = d;         // set initial direction
        on = false;         // initailally off
        pulseCount = 0;     // reset pulse count
        stepMode = s;
        delay = p;          // set initial delay
        endStop.mode(PullUp);
        endStop.fall( this, &Stepper::stop);

    }
    void pulse(void);       // declare functions
    void run(unsigned int period);
    void stop(void);
    void toggleDirection(void);
    void toggleOn(void);
    void runPulse(unsigned int n);
    void ramp(unsigned int c);
    void distance(unsigned int d);
    void initialise(void);
    void Rotate(void);
    void setDirection(int d);

};

void Stepper::pulse(void) {
    stepPin = !stepPin;     // do half a pulse
    pulseCount++;           // count 1
}

void Stepper::run(unsigned int period) {
    if (period != 0) {
        delay = period;
    }
    endStop.mode(PullUp);
    t.attach_us(this, &Stepper::pulse, delay);      // attach the pulse to the ticker timer
    on = true;
}

void Stepper::stop(void) {
    stepPin = 0;        // turn all pins off
    pulseCount = 0;     // reset pulse count
    t.detach();         // turn of ticker
    on = false;
}

void Stepper::toggleDirection(void) {
    dirPin = !dirPin;       // toggle direction
}

void Stepper::toggleOn(void) {
    if (on == false)            // if off
        Stepper::run(0);    // turn on
    else
        Stepper::stop();        // else turn off
}

void Stepper::runPulse(unsigned int n) {
    pulseCount = 0;
    Stepper::run(0);
    while (pulseCount <= n*stepMode*50) {
        wait(.01);
    }
    Stepper::stop();
}

void Stepper::ramp(unsigned int c)  {
    }

void Stepper::distance(unsigned int d)  {
    pulseCount = 0;
    int h = 400;
    while (h > 60)  {
        Stepper::run(h);
        wait(.01);
        h--;
    }
    Stepper::run(0);

    while (pulseCount <= ((d-5)*800)) {
        wait(.1);
        }
    while (pulseCount <= (d*800))  {
        Stepper::run(h);
        wait(.00001);
        h++;
    }
    Stepper::stop();
}

void Stepper::initialise(void) {
    Stepper::setDirection(0);
    endStop.mode(PullUp);
    Stepper::run(100);
    while (endStop == 1) {
    }
    Stepper::toggleDirection();
    Stepper::stop();
}

void Stepper::setDirection(int d) {
    dirPin= d;
}

//===[End of Stepper Class]===


//===[Start of main code]===
int main()
{

    PwmOut servolifter(PA_0);
    PwmOut servocutter(PA_1);
    DigitalIn user_button (USER_BUTTON);
    DigitalIn flyStop (PC_8);
    Stepper plate_translate(PC_4, PB_13, PA_10, 0, 200, 32); // 3
    Stepper plate_rotate(PB_1, PB_2, PC_12, 1, 200, 64); // 2
    Stepper Fly(PB_12, PA_11, PC_12,  0, 200, 32);       // 1
    Stepper conveyor(PB_14, PB_15, PC_12, 1, 200, 32);     // 4
    flyStop.mode(PullUp);
    servocutter.period_us(20000); //-- 20 ms time period
    servolifter.period_us(20000); //-- 20 ms time period
    Initialise();
    servolifter.pulsewidth_us(2350);//-- pulse width of 1 ms; 0 degrees
    servocutter.pulsewidth_us(2350);//-- pulse width of 1 ms; 0 degrees
    char str1[] = "Number of coils:";
    int numberOfCoils = Interface(str1, 3, 100, 500);
    char str3[] = "Number of bobbin";
    int bobbins = Interface(str3, 2, 1, 99);
    char str2[] =  " Gauge of wire :";
    char defaultstr[] = "  Default : 25  ";
    lcdCommand(0x01); //-- display clear
    wait_us(2000); //-- needs a 2msec delay !!
    lcdCommand(0x06); //-- cursor increments
    lcdCommand(0x0f); //-- display on, cursor(blinks) on
    lcdPutString(str2);
    wait(1);
    lcdCommand(0xc0);
    lcdPutString(defaultstr);
    wait(1);
    Initialise();
    lcdCommand(0x0c); //-- display on, cursor(blinks) off
    char str4[] = "Variables Saved";
    lcdPutString(str4);
    wait(1);
    int bobbincounter = 0;
    while (bobbincounter < bobbins) {
        Initialise();
        char str5[] = "  Initialising  ";
        lcdPutString(str5);
        plate_translate.initialise();
        servolifter.pulsewidth_us(850); //-- 20 ms time period
        int d = 35;
        while (d >= 25) {
           plate_translate.run(250);
           d--;
           wait(0.1);
        }
        while (d >= 0) {
           plate_translate.run(800);
           d--;
           wait(0.1);
        }

        plate_translate.stop();
        servolifter.pulsewidth_us(2350); //-- 20 ms time period
        wait(1);
        plate_translate.toggleDirection();
        d = 15;
        while (d >= 1) {
           plate_translate.run(400);
           d--;
           wait(0.1);
        }

      plate_rotate.runPulse(1);
        plate_translate.initialise();
        d = 13;
        while (d >= 1) {
           plate_translate.run(460);
           d--;
           wait(0.1);
        }
        d = 1;
        while (d >= 1) {
           Fly.run(1000);
           d--;
           wait(0.3);
        }
        Fly.toggleOn();
        plate_translate.toggleOn();
        char str10[] = "   Press 'A'    ";
        char str11[] = "  to continue   ";
        lcdCommand(0x01); //-- display clear
        wait_us(2000); //-- needs a 2msec delay !!
        lcdCommand(0x06); //-- cursor increments
        lcdCommand(0x0f); //-- display on, cursor(blinks) on
        lcdPutString(str10);
        wait(1);
        lcdCommand(0xc0);
        lcdPutString(str11);
        lcdCommand(0x0c);
        while (keyscan(1) != 1)  {
        }
        char str12[] = "   Winding...   ";
        lcdCommand(0x01); //-- display clear
        wait_us(2000); //-- needs a 2msec delay !!
        lcdCommand(0x06); //-- cursor increments
        lcdCommand(0x0c);
        lcdPutString(str12);
        lcdCommand(0x0c);
        lcdCommand(0xc0);
        int coil_count = 0;
        int coil_count2 = 0;
        d = 400;
        while (d >= 80){
            plate_translate.run(d*10);
            Fly.run(d);
            d--;
            wait(0.01);
            if (flyStop == 0) {
                coil_count++;
                coil_count2++;
                while (flyStop == 0) {
                }
            }
            if (coil_count2 == numberOfCoils/16)  {
                lcdPutChar('-');
                lcdCommand(0x0c); //-- display on, cursor(blinks) off
                coil_count2 = 0;
                while(coil_count2 == numberOfCoils/16){
                    }
            }

        }

        while (coil_count <= numberOfCoils-1){
            d--;
            if (flyStop == 0) {
                coil_count++;
                coil_count2++;
                while (flyStop == 0) {
                }
                if (coil_count%22 == 0)  {
                plate_translate.toggleDirection();
            }
            }
            wait(0.01);
            if (coil_count2 == numberOfCoils/16)  {
                lcdPutChar('-');
                lcdCommand(0x0c); //-- display on, cursor(blinks) off
                coil_count2 = 0;
                while(coil_count2 == numberOfCoils/16){
                    }
            }

        }
        plate_translate.setDirection(0);
        lcdPutChar('-');
        lcdCommand(0x0c); //-- display on, cursor(blinks) off
        Fly.run(1000);
        while (coil_count <= numberOfCoils){
            d--;
            if (flyStop == 0) {
                coil_count++;
                coil_count2++;

            wait(0.01);

        }
        }
        d = 1;
        while (d >= 1) {
           Fly.run(1000);
           d--;
           wait(0.3);
        }
        Fly.toggleOn();
        lcdCommand(0x01); //-- display clear
        wait_us(2000); //-- needs a 2msec delay !!
        lcdCommand(0x06); //-- cursor increments
        lcdCommand(0x0f); //-- display on, cursor(blinks) on
        lcdPutString(str10);
        wait(1);
        lcdCommand(0xc0);
        lcdPutString(str11);
        lcdCommand(0x0c);
        while (keyscan(1) != 1)  {
        }
        Fly.toggleDirection();
        d = 1;
        while (d >= 1) {
           Fly.run(1000);
           d--;
           wait(0.3);
        }
        Fly.toggleDirection();
        Fly.toggleOn();
        wait(0.5);
        plate_translate.initialise();
        d = 8;
        while (d >= 0) {
           plate_translate.run(225);
           d--;
           wait(0.1);
        }
        plate_translate.toggleOn();
        wait(3);
        d = 7;
        while (d >= 0) {
           plate_translate.run(225);
           d--;
           wait(0.1);
        }
        plate_translate.toggleOn();
        servolifter.pulsewidth_us(850);//-- pulse width of 1 ms; 0 degrees
        Initialise();
        char str6[] = "    Cutting     ";
        lcdPutString(str6);
        lcdCommand(0x0c); //-- display on, cursor(blinks) off
        wait(1);
        Initialise();
        char str13[] = "    Unloading   ";
        lcdPutString(str13);
        lcdCommand(0x0c); //-- display on, cursor(blinks) off
        plate_translate.initialise();
        servolifter.pulsewidth_us(2400); //-- pulse width of 2 ms; 90 degrees

        bobbincounter++;
        if (bobbincounter < bobbins) {
            char str7[] = "Press A to wind ";
            char str8[] = "the next bobbin?";
            lcdCommand(0x01); //-- display clear
            wait_us(2000); //-- needs a 2msec delay !!
            lcdCommand(0x06); //-- cursor increments
            lcdCommand(0x0f); //-- display on, cursor(blinks) on
            lcdPutString(str7);
            wait(1);
            lcdCommand(0xc0);
            lcdPutString(str8);
            lcdCommand(0x0c);
            if (keyscan(1) == 2)  {
                break;
            }
            while (keyscan(1) != 1)  {
            }
        }
        else {
          char str15[] = " Press Reset to ";
          char str16[] = "    continue    ";
          lcdCommand(0x01); //-- display clear
          wait_us(2000); //-- needs a 2msec delay !!
          lcdCommand(0x06); //-- cursor increments
          lcdCommand(0x0f); //-- display on, cursor(blinks) on
          lcdPutString(str15);
          wait(1);
          lcdCommand(0xc0);
          lcdPutString(str16);
          lcdCommand(0x0c);
        }
    }
}

//===[End of main code]===


//===[Start of LCD code]===
int Interface(char *str, int maxDigits, int minBoundary, int maxBoundary)
{
     int InterfaceInt;
     char incorrect1[] = " Invalid Input  ";
     char incorrect2[] = "   Try again    ";
     while(1){
         Initialise();
         lcdPutString(str);
         wait(0.5);
         InterfaceInt = keyscan(maxDigits);
         if(InterfaceInt>=minBoundary){
             if(InterfaceInt<=maxBoundary){
                 break;
             }
             else {
                 lcdCommand(0x01); //-- display clear
                 wait_us(2000); //-- needs a 2msec delay !!
                 lcdCommand(0x06); //-- cursor increments
                 lcdCommand(0x0f); //-- display on, cursor(blinks) on
                 lcdPutString(incorrect1);
                 wait(1);
                 lcdCommand(0xc0);
                 lcdPutString(incorrect2);
                 wait(1);
            }
         }
         else {
                 lcdCommand(0x01); //-- display clear
                 wait_us(2000); //-- needs a 2msec delay !!
                 lcdCommand(0x06); //-- cursor increments
                 lcdCommand(0x0f); //-- display on, cursor(blinks) on
                 lcdPutString(incorrect1);
                 wait(1);
                 lcdCommand(0xc0);
                 lcdPutString(incorrect2);
                 wait(1);
        }
     }
     return(InterfaceInt);
}

void Initialise()
{
     lcdEN.write(0); //-- GPIO_WriteBit(GPIOC, LCD_EN, Bit_RESET);
     wait_us(15000); //-- delay for >15msec second after power on
     lcdInit8Bit(0x30); //-- we are in "8bit" mode wait_us(4100); //-- 4.1msec delay
     lcdInit8Bit(0x30); //-- but the bottom 4 bits are ignored
     wait_us(100); //-- 100usec delay
     lcdInit8Bit(0x30);
     lcdInit8Bit(0x20);
     lcdCommand(0x28); //-- we are now in 4bit mode, dual line
     lcdCommand(0x08); //-- display off
     lcdCommand(0x01); //-- display clear
     wait_us(2000); //-- needs a 2msec delay !!
     lcdCommand(0x06); //-- cursor increments
     lcdCommand(0x0f); //-- display on, cursor(blinks) on

}
static void lcdSetRS(int mode)
{
     lcdRS.write(mode);
}
static void lcdPulseEN(void)
{
     lcdEN.write(1);
     wait_us(1); //-- enable pulse must be >450ns
     lcdEN.write(0);
     wait_us(1);
}
static void lcdInit8Bit(unsigned char command)
{
     lcdSetRS(LCD_INSTRUCTION);
     lcdD4.write((command>>4) & 0x01); //-- bottom 4 bits
     lcdD5.write((command>>5) & 0x01); //-- are ignored
     lcdD6.write((command>>6) & 0x01);
     lcdD7.write((command>>7) & 0x01);
     lcdPulseEN();
     wait_us(37); //-- let it work on the data
}
void lcdCommand(unsigned char command)
{
     lcdSetRS(LCD_INSTRUCTION);
     lcdD4.write((command>>4) & 0x01);
     lcdD5.write((command>>5) & 0x01);
     lcdD6.write((command>>6) & 0x01);
     lcdD7.write((command>>7) & 0x01);
     lcdPulseEN(); //-- this can't be too slow or it will time out
     lcdD4.write(command & 0x01);
     lcdD5.write((command>>1) & 0x01);
     lcdD6.write((command>>2) & 0x01);
     lcdD7.write((command>>3) & 0x01);
     lcdPulseEN();
     wait_us(37); //-- let it work on the data
}
void lcdPutChar(unsigned char c)
{
     lcdSetRS(LCD_DATA);
     lcdD4.write((c>>4) & 0x01);
     lcdD5.write((c>>5) & 0x01);
     lcdD6.write((c>>6) & 0x01);
     lcdD7.write((c>>7) & 0x01);
     lcdPulseEN(); //-- this can't be too slow or it will time out
     lcdD4.write(c & 0x01);
     lcdD5.write((c>>1) & 0x01);
     lcdD6.write((c>>2) & 0x01);
     lcdD7.write((c>>3) & 0x01);
     lcdPulseEN();
     wait_us(37); //-- let it work on the data
}

void lcdPutString(char *s)
{
    int i = 0;
    while(s[i])  {
        lcdPutChar(s[i]);
        i++;
    }
}

void setcol1()
// Sets col1 = high, set col2&3 = low
{
    col1 = 0;
    col2 = 1;
    col3 = 1;
    col4 = 1;

}

void setcol2()
// Sets col2 = high, set col1&3 = low
{
    col1 = 1;
    col2 = 0;
    col3 = 1;
    col4 = 1;
}

void setcol3()
// Sets col3 = high, set col1&2 = low
{
    col1 = 1;
    col2 = 1;
    col3 = 0;
    col4 = 1;
}

void setcol4()
// Sets col3 = high, set col1&2 = low
{
    col1 = 1;
    col2 = 1;
    col3 = 1;
    col4 = 0;
}

int square(int base, int power)
{
    int squareCount;
    int newbase = base;
    if(power == 0){
        return(1);
    }
    for(squareCount = 1; squareCount < power; squareCount++){
        newbase = (newbase*base);
    }
    return (newbase);
}

int keyscan(int digits){
    row1.mode(PullUp);
    row2.mode(PullUp);
    row3.mode(PullUp);
    row4.mode(PullUp);
    int PresentDigit = 0;
    int ReturnNumber = 0;
    int num[digits];
    int numb;
    lcdCommand(0xc0);
    for(numb=0; numb<=digits; numb++){
        num[numb] = 0;
    }
    while(1)
    {
        setcol1();
        wait(0.05);
        if (row1 == 0){
            if(PresentDigit < digits){
                lcdPutChar('1');
                num[PresentDigit] = 1;
                PresentDigit++;
                while(row1 == 0){
                    wait_us(1);
                }
            }
            }
        if (row2 == 0){
            if(PresentDigit < digits){
                lcdPutChar('4');
                num[PresentDigit] = 4;
                PresentDigit++;
                while(row2 == 0){
                    wait_us(1);
                }
            }
            }
        if (row3 == 0){
            if(PresentDigit < digits){
                lcdPutChar('7');
                num[PresentDigit] = 7;
                PresentDigit++;
                while(row3 == 0){
                    wait_us(1);
                }
            }
            }
        if (row4 == 0){
            if(PresentDigit > 0){
                lcdCommand(0x10);
                lcdPutChar(' ');
                lcdCommand(0x10);
                num[PresentDigit] = 0;
                PresentDigit--;

            }
            while(row4 == 0){
                wait_us(1);
            }
            }

        setcol2();
        wait(0.05);
        if (row1 == 0){
            if(PresentDigit < digits){
                lcdPutChar('2');
                num[PresentDigit] = 2;
                PresentDigit++;
                while(row1 == 0){
                    wait_us(1);
                }
            }
            }
        if (row2 == 0){
            if(PresentDigit < digits){
                lcdPutChar('5');
                num[PresentDigit] = 5;
                PresentDigit++;
                while(row2 == 0){
                    wait_us(1);
                }
            }
            }
        if (row3 == 0){
            if(PresentDigit < digits){
                lcdPutChar('8');
                num[PresentDigit] = 8;
                PresentDigit++;
                while(row3 == 0){
                    wait_us(1);
                }
            }
            }
        if (row4 == 0){
            if(PresentDigit < digits){
                lcdPutChar('0');
                num[PresentDigit] = 0;
                PresentDigit++;
                while(row4 == 0){
                    wait_us(1);
                }
            }
            }

        setcol3();
        wait(0.05);
        if (row1 == 0){
            if(PresentDigit < digits){
                lcdPutChar('3');
                num[PresentDigit] = 3;
                PresentDigit++;
                while(row1 == 0){
                    wait_us(1);
                }
            }
            }
        if (row2 == 0){
            if(PresentDigit < digits){
                lcdPutChar('6');
                num[PresentDigit] = 6;
                PresentDigit++;
                while(row2 == 0){
                    wait_us(1);
                }
            }
            }
        if (row3 == 0){
            if(PresentDigit < digits){
                lcdPutChar('9');
                num[PresentDigit] = 9;
                PresentDigit++;
                while(row3 == 0){
                    wait_us(1);
                }
            }
            }
        if (row4 == 0){
            if(PresentDigit <= digits){
                break;
            }
            }

    setcol4();
        wait(0.05);
        if (row1 == 0){
            if(PresentDigit < digits){
                return(1);
                while(row1 == 0){
                    wait_us(1);
                }
            }
            }
        if (row2 == 0){
            if(PresentDigit < digits){
                return(2);
                while(row2 == 0){
                    wait_us(1);
                }
            }
            }
        if (row3 == 0){
            if(PresentDigit < digits){
                lcdPutChar('C');
                while(row3 == 0){
                    wait_us(1);
                }
            }
            }
        if (row4 == 0){
            if(PresentDigit <= digits){
                lcdPutChar('D');
                while(row4 == 0){
                    wait_us(1);
                }
            }
            }
    }
    int i;
    int TotalDigits = PresentDigit;
    PresentDigit--;
    for(i=0; i < TotalDigits; i++){
        ReturnNumber += ((num[i])*square(10, PresentDigit));
        PresentDigit--;
    }
    return(ReturnNumber);
}

////==========[END OF LCD CODE]========================
