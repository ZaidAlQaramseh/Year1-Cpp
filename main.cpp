#include "PinNames.h"
#include "PinNamesTypes.h"
#include "ThisThread.h"
#include "Timer.h"
#include "mbed.h"
#include "mbed_power_mgmt.h"
#include "mbed_rtc_time.h"
#include "mbed_thread.h"
#include "protocol_abstract.h"
#include <cstdint>
#include <cstdlib> 

//note definitions
#define NOTE_FS7 2960

//establish serial communications between PC and NUCLEO
BufferedSerial pc(USBTX, USBRX,115200);

//Define output pins
DigitalOut led1(PC_0);  //led outputs
DigitalOut led2(PC_1);
DigitalOut led3(PB_0);
DigitalOut led4(PA_4);
PwmOut buzzer(PA_15);  //PWM output to buzzer
PwmOut red_LED(PB_3);   //PWM output to led
PwmOut blue_LED(PB_4);
PwmOut green_LED(PB_5);
BusOut SegDis(PA_11,PA_12,PB_1,PB_15,PB_14,PB_12,PB_11); //7 seg display pins in correct order

//Define input pins
DigitalIn button1(PC_10);
DigitalIn button2(PC_11);

//define analugue input to read potentiometer voltage
AnalogIn pot1(PA_5);
AnalogIn pot2(PA_6);
AnalogIn pot3(PA_7);
AnalogIn FSR(PA_1); //Force sensor pin

//declare other variables
const float beepFrequency = 2960; //float
float FSR_Val = 0.0f;
Timer hold_timer;  //Timer
int mode = 0;  //integers
int temp = 0;
int Time = 0;
int Clock = 0;
int default_time = 0;
bool machineOn = false;      //bool
bool noCycle = false;
bool start_cycle = false;
bool overload = false;
bool overload_printed = false;

//declare pot variables
int pot1_val_1 = 0;  
int pot1_val_2 = 0;      
int pot1_val_3 = 0;
int pot2_val_1 = 0;
int pot2_val_2 = 0;
int pot2_val_3 = 0;
int pot2_val_4 = 0;
int pot3_val_1 = 0;
int pot3_val_2 = 0;
int pot3_val_3 = 0;
int pot3_val_4 = 0;
int pot3_val_5 = 0;
int pot3_val_6 = 0;

//Declare functions
void washing_machine_power();
void select_mode();
void select_temperature();
void select_time();
void normal_cycle();
void perm_press();
void delicate();
void noMode();
void cycle();
void force_sensor();

// main() runs in its own thread in the OS
int main(){
    
    //print instructions once for new users
    printf("Turn the washing machine on/off using button 1\n\nAfter selecting the settings of the cycle, press button 2 to start the cycle\n\n"); 
    
    while(1) {
                      
    washing_machine_power();
                
    select_mode();

    select_temperature();

    select_time();

    cycle();

    force_sensor();

    }
    
}
         

// Functions implementation

void washing_machine_power(){
                
        // If button is pressed, toggle LED state
        if (button1 == 1) {

            led1 = !led1;
        
            // Make the buzzer beep for the specified duration
            buzzer = 0.5;
            buzzer.period(1.0/beepFrequency);

            // Wait 350 ms to avoid button bouncing
             thread_sleep_for(350);
            
            hold_timer.start(); //start the hold timer

            machineOn = true; //set machine state to on
            if(!led1){red_LED.write(0); blue_LED.write(0); green_LED.write(0);} //turn off all leds when machine is off
            if(led1){printf("Select Mode using pot 1\t\tSelect Temperature using pot 2\t\tSet Timer using pot 3\n\n");}
            } 
        else{
                buzzer = 0; //buzzer off
                hold_timer.stop(); //stop the hold timer
                hold_timer.reset(); //reset the hold timer
                 }
            while (hold_timer.read_ms() >= 350){ //if user holds on/off button for more than
            buzzer = 0;                          // 350 ms turn off machine to avoid glitching
            led1 = 0;
            if(button1 == 0){hold_timer.reset();}
            }
}

void select_mode(){

    if(machineOn){
    
    if(led1){

    pot1_val_1 = pot1.read_u16()>=2000 && pot1.read_u16()<=20845;
    pot1_val_2 = 20845<pot1.read_u16() && pot1.read_u16()<=36690;
    pot1_val_3 = 36690<pot1.read_u16() && pot1.read_u16()<=65535;

    if(pot1_val_1){mode = 1;}
    
    else if(pot1_val_2){mode = 2;}
    
    else if(pot1_val_3){mode = 3;}

    else{mode = 4;}

    switch (mode) {
        
        case 1:
        normal_cycle();
        break;
        
        case 2:
        perm_press();
        break;
        
        case 3:
        delicate();
        break;

        case 4:
        noMode();
        break;
    }
    ThisThread::sleep_for(50);
    }
    }

}

void normal_cycle(){
    

    printf("Normal cycle                                                      \r");
    noCycle = false;
    overload_printed=false;
    default_time = 3000000;   //set default cycle time to 50 min
    buzzer = 0; led2 = 0; led3 = 0; led4 = 0;
    while(FSR.read()*120 > 90){    //Warn user when weight is above limit
        
        if(!overload_printed){printf("\n\nOVERLOAD! TAKE WEIGHT OFF (9 KG Max)\n\n"); overload_printed = true;}

        led2 = !led2;   //toggle leds every 1 second
        led3 = !led3;
        led4 = !led4;
        overload = !overload;

        if(overload){
        buzzer = 0.5; buzzer.period(1.0/beepFrequency);  // toggle buzzer every 1 second
        }
        if(!overload){
        buzzer = 0; led2 = 0; led3 = 0; led4 = 0;
        }

        red_LED.write(0); blue_LED.write(0); green_LED.write(0);
        
        thread_sleep_for(1000);

    }

    
}

void perm_press(){
    

    printf("Permanent press                                                   \r");
    noCycle = false;
    overload_printed=false;
    default_time = 1800000;    //set default cycle time to 30 min
    buzzer = 0; led2 = 0; led3 = 0; led4 = 0;
    while(FSR.read()*120 > 60){    //Warn user when weight is above limit
        
        if(!overload_printed){printf("\n\nOVERLOAD! TAKE WEIGHT OFF (6 KG Max)\n\n"); overload_printed = true;}

        led2 = !led2;   //toggle leds every 1 second
        led3 = !led3;
        led4 = !led4;
        overload = !overload;

        if(overload){
        buzzer = 0.5; buzzer.period(1.0/beepFrequency);  // toggle buzzer every 1 second
        }
        if(!overload){
        buzzer = 0; led2 = 0; led3 = 0; led4 = 0;
        }

        red_LED.write(0); blue_LED.write(0); green_LED.write(0);
        
        thread_sleep_for(1000);

    }

}

void delicate(){

    printf("Delicate                                                          \r");
    noCycle = false;
    overload_printed=false;
    default_time = 2700000;    //set default cycle time to 45 min
    buzzer = 0; led2 = 0; led3 = 0; led4 = 0;
    while(FSR.read()*120 > 40){    //Warn user when weight is above limit
        
        if(!overload_printed){printf("\n\nOVERLOAD! TAKE WEIGHT OFF (4 KG Max)\n\n"); overload_printed = true;}

        led2 = !led2;   //toggle leds every 1 second
        led3 = !led3;
        led4 = !led4;
        overload = !overload;

        if(overload){
        buzzer = 0.5; buzzer.period(1.0/beepFrequency);  // toggle buzzer every 1 second
        }
        if(!overload){
        buzzer = 0; led2 = 0; led3 = 0; led4 = 0;
        }

        red_LED.write(0); blue_LED.write(0); green_LED.write(0);
        
        thread_sleep_for(1000);

    }

}

void noMode(){

            printf("No mode selected\t\tNo mode selected\t\t\tNo mode selected\r");
            noCycle = true;
            red_LED.write(0); blue_LED.write(0); green_LED.write(0);
    
}

void select_temperature(){

    if(machineOn){
    
    if(led1){

        if(!noCycle){

    pot2_val_1 = pot2.read_u16()>=3500 && pot2.read_u16()<=17845;
    pot2_val_2 = 17845<pot2.read_u16() && pot2.read_u16()<=32690;
    pot2_val_3 = 32690<pot2.read_u16() && pot2.read_u16()<=47536;
    pot2_val_4 = 47436<pot2.read_u16() && pot2.read_u16()<=65535;

    if(pot2_val_1){temp = 1;}
    
    else if(pot2_val_2){temp = 2;}
    
    else if(pot2_val_3){temp = 3;}

    else if(pot2_val_4){temp = 4;}
    
    else{temp = 5;}

    switch (temp) {
        
        case 1:
        printf("\t\t\t\t20°C   \r");
        blue_LED.write(1 - (pot2.read())); red_LED.write(0); green_LED.write(0);
        break;
        
        case 2:
        printf("\t\t\t\t30°C   \r");
        blue_LED.write(1 - (pot2.read()*2)); red_LED.write(0); green_LED.write(0);
        break;
        
        case 3:
        printf("\t\t\t\t40°C   \r");
        red_LED.write(pot2.read()/100); blue_LED.write(0); green_LED.write(0);
        break;
        
        case 4:
        printf("\t\t\t\t60°C   \r");
        red_LED.write(pot2.read()/100); blue_LED.write(0); green_LED.write(0);
        break;

        case 5:
        printf("\t\t\t\tDefault\r");
        red_LED.write(0); blue_LED.write(0); green_LED.write(0.5);
        break;
           
    }
    ThisThread::sleep_for(25);
    }
    }
        }
}

void select_time(){

    if(machineOn){
    
    if(led1){

        if(!noCycle){

    pot3_val_1 = pot3.read_u16()>=3500 && pot3.read_u16()<=12845;
    pot3_val_2 = 12845<pot3.read_u16() && pot3.read_u16()<=24690;
    pot3_val_3 = 24690<pot3.read_u16() && pot3.read_u16()<=34536;
    pot3_val_4 = 34536<pot3.read_u16() && pot3.read_u16()<=45536;
    pot3_val_5 = 45536<pot3.read_u16() && pot3.read_u16()<=55536;
    pot3_val_6 = 55536<pot3.read_u16() && pot3.read_u16()<=65535;

    if(pot3_val_1){Time = 1;}
    
    else if(pot3_val_2){Time = 2;}
    
    else if(pot3_val_3){Time= 3;}

    else if(pot3_val_4){Time = 4;}

    else if(pot3_val_5){Time = 5;}

    else if(pot3_val_6){Time = 6;}
    
    else{Time = 7;}

    switch (Time) {
        
        case 1:
        printf("\t\t\t\t\t\t\t\t\t30 minutes      \r");
        Clock = 1800000; //set cycle time to 30 min
        break;
        
        case 2:
        printf("\t\t\t\t\t\t\t\t\t40 minutes      \r");
        Clock = 2400000; //set cycle time to 40 min
        break;
        
        case 3:
        printf("\t\t\t\t\t\t\t\t\t50 minutes      \r");
        Clock = 3000000; //set cycle time to 50 min
        break;
        
        case 4:
        printf("\t\t\t\t\t\t\t\t\t60 minutes      \r");
        Clock = 3600000; //set cycle time to 60 min
        break;

        case 5:
        printf("\t\t\t\t\t\t\t\t\t75 minutes      \r");
        Clock = 4500000; //set cycle time to 75 min
        break;
        
        case 6:
        printf("\t\t\t\t\t\t\t\t\t90 minutes      \r");
        Clock = 5400000; //set cycle time to 90 min
        break;

        case 7:
        printf("\t\t\t\t\t\t\t\t\tDefault         \r");
        Clock = default_time;
        break;

    }
    ThisThread::sleep_for(50);
    }
    }
        }
}

void cycle(){

    Timer cycle_timer;
    bool start_cycle = false;
    SegDis.write(0x00); //turn off the display by setting all segments to '0'
    //               0     1     2     3     4     5     6     7     8     9     0 
    int hexDis[] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F, 0x3F}; 
    // Add the rest of the hex values associated with the display items 0-9.

        if(machineOn){
    
        if(led1){

        if(!noCycle){
    
        if(button2 == 1){
        
        for(int i = 0; i < 5; i++) { 
            
            if(i == 0){led2 = 1;thread_sleep_for(1000);}
            
            else if(i == 1){led2 = 1; led3 = 1;thread_sleep_for(1000);}
            
            else if(i == 2){led2 = 1; led3 = 1; led4 = 1;thread_sleep_for(1000);}
            
            else if(i == 3){led2 = 1; led3 = 1; led4 = 1; 
            printf("\n******************************************************* CYCLE STARTED ***********************************************************\n\n"); 
            buzzer = 0.5; buzzer.period(1.0/beepFrequency);thread_sleep_for(500);}
            
            else{led2 = 1; led3 = 1; led4 = 1; buzzer = 0;}
        }

        start_cycle = !start_cycle;
        ThisThread::sleep_for(200);
        cycle_timer.start();
        red_LED.write(0); blue_LED.write(0); green_LED.write(0);
    
    while(start_cycle){
        
        int seconds = Clock/1000; // Set timer to time specified 

            while(seconds > 0) {

        for(int i = 10; i > 0; i--){    
        
        SegDis.write(hexDis[i]);  //cycle through the array and display each number
           
        int minutes = seconds / 60; // Calculate remaining minutes
        int remainingSeconds = seconds % 60; // Calculate remaining seconds
        printf("Time remaining: %d:%02d\n", minutes, remainingSeconds);
        thread_sleep_for(1000); // Wait for 1 second
        seconds--; // Decrement the remaining time
        }

            }
        printf("\n******************************************************* CYCLE FINISHED **********************************************************\n\n");
    
        if(cycle_timer.read_ms()>=Clock){  //notify user when timer is finished
            start_cycle=false;
            cycle_timer.reset();
            led2 = 0; led3 = 0; led4 = 0;
            buzzer = 0.5; buzzer.period(1.0/beepFrequency);
            }
    
    }
             thread_sleep_for(1000);
        }
    
        }
        }
        }

}

void force_sensor(){

    FSR_Val = FSR.read()*120; //read value of force sensor

}

