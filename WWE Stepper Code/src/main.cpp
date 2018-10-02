// Code working on github
// Ryan was here haha 2k18
// Hamish was here

// WWE - Stepper Code

#include "mbed.h"
#include "math.h"
#define LCD_DATA 1
#define LCD_INSTRUCTION 0

//===[Stepper Class]===
class Stepper {
private:
    InterruptIn endStop;
    DigitalOut dirPin;      // stepper direction pin
    DigitalOut stepPin;     // stepper atep pinh
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
    int getRotation(void);
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
    while (pulseCount <= n*stepMode*200) {
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
    Stepper::run(100);
    while (endStop == 1) {
    }
    Stepper::stop();
    Stepper::runPulse(4000);
}

int Stepper::getRotation() {
    int rotation = (pulseCount/(stepMode*200));
    return rotation;
}

void Stepper::setDirection(int d) {
    dirPin= d;
}

int main()
{


    DigitalIn user_button (USER_BUTTON);
    Stepper plate_translate(PB_14, PB_15, PC_5, 1, 200, 32);       //green a4988
    Stepper plate_rotate(PB_1, PB_2, PC_5, 1, 200, 32);
    Stepper fly_winder(PB_12, PA_11, PC_12,  1, 200, 16);       // 0-clock, 1-anticlock  //red a4988
    Stepper conveyor(PC_4, PB_13, PB_7, 1, 200, 32);


    // If user input says "Go", then run the following CODE

    // In the occurence of turning 120 degrees



//==========[TEST CODE SET 1]========================
//
//    int d = 400;
//    while (d >= 100) {
//        plate_rotate.run(d);
//        plate_translate.run(d);
//        fly_winder.run(d);
//        conveyor.run(d);
//        d--;
//        wait(0.003);
//    }
//
//    while (d <= 400) {
//       plate_rotate.run(d);
//       plate_translate.run(d);
//       fly_winder.run(d);
//       conveyor.run(d);
//       d++;
//       wait(0.003);
//    }
//
//    while (d >= 100) {
//       plate_rotate.run(d);
//       plate_translate.run(d);
//       fly_winder.run(d);
//       conveyor.run(d);
//       d--;
//       wait(0.003);
//    }
//
//    while (d <= 400) {
//       plate_rotate.run(d);
//       plate_translate.run(d);
//       fly_winder.run(d);
//       conveyor.run(d);
//       d++;
//       wait(0.003);
//    }
// }


//==========[END OF TEST CODE SET 1]========================

//==========[TEST CODE SET 2]========================
// int d = 400;
//     while (d > 80)  {
//          // Delay is decreasing because d is initially 400 and going down to 80
//          fly_winder.run(d);
//          plate_translate.run(d);
//          wait(0.001);
//          // Wait 0.001 before decrementing step (and accelerating)
//          d--;
//     }
//     wait(0.1);
//     // Stays at delay of 80 (constant speed) for 1 seconds
//     while (d > 22)  {
//          fly_winder.run(d);
//          plate_translate.run(d);
//          // Decrement from 80 down to 20 every s 1 second (wait(1))
//          wait(0.1);
//          d--;
//          //Still accelerating
//          }
//     wait(20);
//     // Stay at delay=20 (constant speed) for 10 seconds
//     while (d < 1000)  {
//          fly_winder.run(d);
//          plate_translate.run(d);
//          // Incrementing delay up every 0.1
//          wait(.001);
//          // Decelerating because delay (between pulses) is now increasing
//          d++;
//      }
//     return 0;
// }
//==========[END OF TEST CODE SET 2]========================


//==========[TEST CODE SET 3]========================
// int d = 300;
// plate_rotate.run(200);
// // fly_winder.run(600);
// while (d >= 100) {
//    fly_winder.run(d);
//    d--;
//    wait(0.003);
// }

//==========[END OF TEST CODE SET 3]========================

//==========[TEST CODE SET 4]========================
//Using fly_winder derived class as plate rotate class for example purposes
//Plate translate class will toggle directions as if to guide the wire onto fly_winder
//Plate will rotate 120 degrees after specified time, after fly_winder has been wound (e.g. 20 seconds)
int noOfBobbin = 3;
int bob = 0;
while(bob <= noOfBobbin)  {
  // Conveyor
  int d = 20;
  int count;
  while (d >= 0) {
     conveyor.run(500);
     d--;
     wait(0.1);
  }
  conveyor.toggleOn();

  // Plate Translate
  d = 20;
  while (d >= 0) {
     plate_translate.run(500);
     d--;
     wait(0.1);
  }
  plate_translate.toggleDirection();
  d = 20;
  while (d >= 0) {
     plate_translate.run(500);
     d--;
     wait(0.1);
  }

  plate_translate.toggleOn();

  d = 5;
  while (d >=  0){
     plate_rotate.run(500);
     d--;
     wait(0.1);
  // wait(20);
  }
  plate_rotate.toggleOn();
  d = 200;
  count = 0;
  while (d>=50) {
     fly_winder.run(d);
     d--;
     count++;
     plate_translate.run(d*4);
     if (count == 10) {
       count = 0;
       plate_translate.toggleDirection();
     }
     wait(0.1);
     }

  d = 0;
  while (d<=100) {
    fly_winder.run(50);
    d++;
    count++;
    plate_translate.run(200);
    if (count == 10) {
      count = 0;
      plate_translate.toggleDirection();
    }
    wait(0.1);
    }

  fly_winder.toggleOn();

  d = 30;
  while (d >= 0) {
     plate_translate.run(500);
     d--;
     wait(0.1);
  }

  plate_translate.toggleDirection();
  d = 30;
  while (d >= 0) {
     plate_translate.run(500);
     d--;
     wait(0.1);
  }
  plate_translate.toggleOn();
  bob++;
}
  // wait(20);

}







// while (d <= 100) {
//    fly_winder.run(d);
// }

//==========[END OF TEST CODE SET 4]=======================
