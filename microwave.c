//include somthing
#include <stdio.h>
#include <wiringPi.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>


//define pin
#define LELAY1 4
#define LELAY2 5

#define MOTOR_IN_1 6
#define MOTOR_IN_2 3

#define PWM 1

#define HUMID_CUTOFF 60
#define TEMP_CUTOFF 28

float temp = 0;
float humid = 0;

#define MAXTIMINGS 85
static int DHTPIN = 2;
static int dht22_dat[5] = {0,0,0,0,0};

static int read_dht22_dat();
static uint8_t sizecvt(const int read);

//setup
void setup(void)  {
    //initialize pin
    wiringPiSetup();
    pinMode(LELAY1, OUTPUT);
    pinMode(LELAY2, OUTPUT);
    pinMode(MOTOR_IN_1, OUTPUT);
    pinMode(MOTOR_IN_2, OUTPUT);
    pinMode(PWM, PWM_OUTPUT);
}


//main
int main(void) {
	
    int tries = 100;
    int status;
    //initilize 
    setup();

    //test motor

    digitalWrite(MOTOR_IN_1, LOW);
    digitalWrite(MOTOR_IN_2, HIGH);
    pwmWrite(PWM, 47);
    
    //set relay off when first start
    digitalWrite(LELAY1, HIGH);
    digitalWrite(LELAY2, HIGH);
    
    //forever loop
    while (1) {

        //read humidity from sensor

        while (status = read_dht22_dat() == 0 && tries--) {
            delay(1000); // wait 1sec to refresh
        }

        printf("Humidity = %.2f %% Temperature = %.2f *C \n", humid, temp );
        if (humid > HUMID_CUTOFF ) {
            //on heater and motor
            digitalWrite(LELAY1, LOW);
            digitalWrite(LELAY2, LOW);
            if (temp > TEMP_CUTOFF) {
                //off heater and motor
                digitalWrite(LELAY1, HIGH);
                digitalWrite(LELAY2, HIGH);
            }
        } else {
            //off heater and motor
            digitalWrite(LELAY1, HIGH);
            digitalWrite(LELAY2, HIGH);
        }

        

    }


}

static uint8_t sizecvt(const int read)
{
  /* digitalRead() and friends from wiringpi are defined as returning a value
  < 256. However, they are returned as int() types. This is a safety function */

  if (read > 255 || read < 0)
  {
    printf("Invalid data from wiringPi library\n");
    exit(EXIT_FAILURE);
  }
  return (uint8_t)read;
}

static int read_dht22_dat()
{
  uint8_t laststate = HIGH;
  uint8_t counter = 0;
  uint8_t j = 0, i;

  dht22_dat[0] = dht22_dat[1] = dht22_dat[2] = dht22_dat[3] = dht22_dat[4] = 0;

  // pull pin down for 18 milliseconds
  pinMode(DHTPIN, OUTPUT);
  digitalWrite(DHTPIN, HIGH);
  delay(10);
  digitalWrite(DHTPIN, LOW);
  delay(18);
  // then pull it up for 40 microseconds
  digitalWrite(DHTPIN, HIGH);
  delayMicroseconds(40); 
  // prepare to read the pin
  pinMode(DHTPIN, INPUT);

  // detect change and read data
  for ( i=0; i< MAXTIMINGS; i++) {
    counter = 0;
    while (sizecvt(digitalRead(DHTPIN)) == laststate) {
      counter++;
      delayMicroseconds(1);
      if (counter == 255) {
        break;
      }
    }
    laststate = sizecvt(digitalRead(DHTPIN));

    if (counter == 255) break;

    // ignore first 3 transitions
    if ((i >= 4) && (i%2 == 0)) {
      // shove each bit into the storage bytes
      dht22_dat[j/8] <<= 1;
      if (counter > 16)
        dht22_dat[j/8] |= 1;
      j++;
    }
  }

  // check we read 40 bits (8bit x 5 ) + verify checksum in the last byte
  // print it out if data is good
  if ((j >= 40) && 
      (dht22_dat[4] == ((dht22_dat[0] + dht22_dat[1] + dht22_dat[2] + dht22_dat[3]) & 0xFF)) ) {
        float t, h;
        h = (float)dht22_dat[0] * 256 + (float)dht22_dat[1];
        h /= 10;
        t = (float)(dht22_dat[2] & 0x7F)* 256 + (float)dht22_dat[3];
        t /= 10.0;
        if ((dht22_dat[2] & 0x80) != 0)  t *= -1;


    //printf("Humidity = %.2f %% Temperature = %.2f *C \n", h, t );
    //put temperature and humidity to out side function 
    temp = t;
    humid = h;

    return 1;
  }
  else
  {
    //printf("Data not good, skip\n");
    return 0;
  }
}


