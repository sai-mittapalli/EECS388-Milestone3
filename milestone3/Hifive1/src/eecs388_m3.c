#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "eecs388_lib.h"
#include "metal/i2c.h"

struct metal_i2c *i2c;
uint8_t bufWrite[5];
uint8_t bufRead[1];
volatile int g_angle;
volatile int g_direction;
volatile int g_time;

//The entire setup sequence
void set_up_I2C()
{
    uint8_t oldMode;
    uint8_t newMode;
    _Bool success;

    bufWrite[0] = PCA9685_MODE1;
    bufWrite[1] = MODE1_RESTART;
    printf("%d\n",bufWrite[0]);
    
    i2c = metal_i2c_get_device(0);

    if(i2c == NULL){
        printf("Connection Unsuccessful\n");
    }
    else{
        printf("Connection Successful\n");
    }
    
    //Setup Sequence
    metal_i2c_init(i2c,I2C_BAUDRATE,METAL_I2C_MASTER);
    success = metal_i2c_write(i2c,PCA9685_I2C_ADDRESS,2,bufWrite,METAL_I2C_STOP_DISABLE);//reset
    delay(100);
    printf("resetting PCA9685 control 1\n");

    //Initial Read of control 1
    bufWrite[0] = PCA9685_MODE1;//Address
    success = metal_i2c_transfer(i2c,PCA9685_I2C_ADDRESS,bufWrite,1,bufRead,1);//initial read
    printf("Read success: %d and control value is: %d\n", success, bufWrite[0]);
    
    //Configuring Control 1
    oldMode = bufRead[0];
    newMode = (oldMode & ~MODE1_RESTART) | MODE1_SLEEP;
    printf("sleep setting is %d\n", newMode);
    bufWrite[0] = PCA9685_MODE1;//address
    bufWrite[1] = newMode;//writing to register
    success = metal_i2c_write(i2c,PCA9685_I2C_ADDRESS,2,bufWrite,METAL_I2C_STOP_DISABLE);//sleep
    bufWrite[0] = PCA9685_PRESCALE;//Setting PWM prescale
    bufWrite[1] = 0x79;
    success = metal_i2c_write(i2c,PCA9685_I2C_ADDRESS,2,bufWrite,METAL_I2C_STOP_DISABLE);//sets prescale
    bufWrite[0] = PCA9685_MODE1;
    bufWrite[1] = 0x01 | MODE1_AI | MODE1_RESTART;
    printf("on setting is %d\n", bufWrite[1]);
    success = metal_i2c_write(i2c,PCA9685_I2C_ADDRESS,2,bufWrite,METAL_I2C_STOP_DISABLE);//awake
    delay(100);
    printf("Setting the control register\n");
    bufWrite[0] = PCA9685_MODE1;
    success = metal_i2c_transfer(i2c,PCA9685_I2C_ADDRESS,bufWrite,1,bufRead,1);//initial read
    printf("Set register is %d\n",bufRead[0]);
} 

void breakup(int bigNum, uint8_t* low, uint8_t* high)
{
     *low = bigNum & 0xff;
    *high = (bigNum >> 8);
}

void steering(int angle)
{
    int valToBreak = getServoCycle(angle);
   bufWrite[0]=PCA9685_LED0_ON_L;
   bufWrite[1]=0;
   bufWrite[2]=0; 
   breakup(valToBreak,&bufWrite[3],&bufWrite[4]);
   metal_i2c_transfer(i2c,PCA9685_I2C_ADDRESS,bufWrite,5,bufRead,1);
}

void stopMotor()
{
    bufWrite[0]=PCA9685_LED1_ON_L;
    bufWrite[1]=0;
    bufWrite[2]=0;
    breakup(280,&bufWrite[3],&bufWrite[4]);
    metal_i2c_transfer(i2c,PCA9685_I2C_ADDRESS,bufWrite,5,bufRead,1);
}

void driveForward(uint8_t speedFlag)
{
    if(speedFlag ==1)
   {
        bufWrite[0]=PCA9685_LED1_ON_L;
        bufWrite[1]=0;
        bufWrite[2]=0; 
        breakup(313,&bufWrite[3],&bufWrite[4]);
        metal_i2c_transfer(i2c,PCA9685_I2C_ADDRESS,bufWrite,5,bufRead,1);
   }
   else if(speedFlag ==2){
         bufWrite[0]=PCA9685_LED1_ON_L;
         bufWrite[1]=0;
         bufWrite[2]=0; 
         breakup(315,&bufWrite[3],&bufWrite[4]);
         metal_i2c_transfer(i2c,PCA9685_I2C_ADDRESS,bufWrite,5,bufRead,1);
   }
   else if(speedFlag == 3)
   {
         bufWrite[0]=PCA9685_LED1_ON_L;
         bufWrite[1]=0;
         bufWrite[2]=0; 
         breakup(317,&bufWrite[3],&bufWrite[4]);
         metal_i2c_transfer(i2c,PCA9685_I2C_ADDRESS,bufWrite,5,bufRead,1);
   }
}

void driveReverse(uint8_t speedFlag)
{
    if(speedFlag ==1)
   {
        bufWrite[0]=PCA9685_LED1_ON_L;
        bufWrite[1]=0;
        bufWrite[2]=0; 
        breakup(267,&bufWrite[3],&bufWrite[4]);
        metal_i2c_transfer(i2c,PCA9685_I2C_ADDRESS,bufWrite,5,bufRead,1);
   }
   else if(speedFlag ==2){
        bufWrite[0]=PCA9685_LED1_ON_L;
        bufWrite[1]=0;
        bufWrite[2]=0; 
        breakup(265,&bufWrite[3],&bufWrite[4]);
        metal_i2c_transfer(i2c,PCA9685_I2C_ADDRESS,bufWrite,5,bufRead,1);
   }
   else if(speedFlag == 3)
   {
        bufWrite[0]=PCA9685_LED1_ON_L;
        bufWrite[1]=0;
        bufWrite[2]=0; 
        breakup(263,&bufWrite[3],&bufWrite[4]);
        metal_i2c_transfer(i2c,PCA9685_I2C_ADDRESS,bufWrite,5,bufRead,1);
   }
}

void raspberrypi_int_handler(int devid)
{
    char * str = malloc(20 * sizeof(char));

    int time,ang,dir;
    //read input store into str
    ser_readline(devid,20,str);

    //break str into vars
    // sscanf(str, "%d %d %d", &g_direction, &g_time, &g_angle);
    sscanf(str, "%d", &dir);
    sscanf(str+2, "%d", &time);
    sscanf(str+4, "%d", &ang);

    //debug output
    printf("%d %d %d ", dir, time, ang);

    //free space used for str
    free(str);
}


int main()
{
    // Initialize I2C
    set_up_I2C();
    delay(2000);

    // Calibrate Motor
    printf("Calibrate Motor.\n");
    stopMotor();
    delay(2000);

    // initialize UART channels
    ser_setup(0); // uart0 (receive from raspberry pi)
    ser_setup(1);
    
    printf("Setup completed.\n");
    printf("Begin the main loop.\n");
    
    // Initialize global angle
    g_angle = 0;
    g_direction = 0;
    g_time = 0;
    // Drive loop
    while (1) {

        if (ser_isready(1))
        {
            //read command
            //raspberrypi_int_handler(1);
            char * str = malloc(20 * sizeof(char));

            int time,ang,dir;
            //read input store into str
            ser_readline(1,20,str);

            //break str into vars
            // sscanf(str, "%d %d %d", &g_direction, &g_time, &g_angle);
            sscanf(str, "%d", &dir);
            sscanf(str+2, "%d", &time);
            sscanf(str+4, "%d", &ang);

            //debug output
            printf("%d %d %d ", dir, time, ang);

            //free space used for str
            free(str);

            //set driving direction or stop
            if (dir == 1)
            {
                driveForward(1);
            }
            else if (dir == 2)
            {
                driveReverse(1);
            }
            else
            {
                stopMotor();
            }

            //set steering
            steering(ang);
            //set delay time
            delay(1000 * time);
        }
    
    }
    return 0;
}
