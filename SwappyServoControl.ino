/*
 * ServoControl
 * 
 * Version: 1.0
 * Creator: Sebastian Ehnert
 * Date: 16.11.2022
 * 
 * Description:
 * Control up to 8 Servo motors. 
 * The Host can control the Servo motors over a SPI protocol.
 */

#include <Servo.h>

#define DATA_PIN_SERVO_1 2
#define DATA_PIN_SERVO_2 3
#define DATA_PIN_SERVO_3 4
#define DATA_PIN_SERVO_4 5
#define DATA_PIN_SERVO_5 6
#define DATA_PIN_SERVO_6 7
#define DATA_PIN_SERVO_7 8
#define DATA_PIN_SERVO_8 9

// Commands:
#define READY_REPLY                   0x00
#define CHANGE_ANGLE_AND_SPEED        0x10
#define CALIBRATE_DEVICE              0x11

// Devices:
#define UNKNOWN   0x00
#define SERVO_1   0x01
#define SERVO_2   0x02
#define SERVO_3   0x03
#define SERVO_4   0x04
#define SERVO_5   0x05
#define SERVO_6   0x06
#define SERVO_7   0x07
#define SERVO_8   0x08
#define ALL       0x09

// ReturnStates:
#define RETURN_SUCCESS  0x01
#define RETURN_ERROR    0x02

Servo Servo_1;
Servo Servo_2;
Servo Servo_3;
Servo Servo_4;
Servo Servo_5;
Servo Servo_6;
Servo Servo_7;
Servo Servo_8;

// SPI Communication
char spi_receive_buffer[30];
char spi_send_buffer[30];
volatile byte index;
volatile bool process_spi_command;

void setup() 
{
  // Default Values
  memset(spi_receive_buffer, 0, sizeof(spi_receive_buffer));
  memset(spi_send_buffer, 0, sizeof(spi_send_buffer));
  index = 0;
  process_spi_command = false;
  
  // SPI Slave
  pinMode(MISO, OUTPUT);
  SPCR |= _BV(SPE); // slave mode
  SPCR |= _BV(SPIE); // enable interrupts
  process_spi_command = false;

  // Servo's
  Servo_1.attach(DATA_PIN_SERVO_1);
  Servo_2.attach(DATA_PIN_SERVO_2);
  Servo_3.attach(DATA_PIN_SERVO_3);
  Servo_4.attach(DATA_PIN_SERVO_4);
  Servo_5.attach(DATA_PIN_SERVO_5);
  Servo_6.attach(DATA_PIN_SERVO_6);
  Servo_7.attach(DATA_PIN_SERVO_7);
  Servo_8.attach(DATA_PIN_SERVO_8);

  // Test UART
  Serial.begin(9600);
}

ISR (SPI_STC_vect)
{
  byte c = SPDR; // read a byte from SPI register

  if(process_spi_command == false)
  {
    if (index < sizeof spi_receive_buffer) 
    {
      spi_receive_buffer[index] = c; 
      if (c == 0xFF) 
      {
        process_spi_command = true;
      }
    }

    c = spi_send_buffer[index];

    index++;
  }
}

void rotateServo(Servo& servo, int angle, int speed)
{
  int delayStep = round(speed / angle);

  for(int pos = 0; pos < angle; pos++)
  {
    servo.write(pos);
    delay(delayStep);
  }
}

Servo& getServo(byte device)
{
  switch(device)
  {
    case SERVO_1:
    {
      return Servo_1;
      break;      
    }
    case SERVO_2:
    {
      return Servo_2;
      break;      
    } 
    case SERVO_3:
    {
      return Servo_3;
      break;      
    } 
    case SERVO_4:
    {
      return Servo_4;
      break;      
    } 
    case SERVO_5:
    {
      return Servo_5;
      break;      
    } 
    case SERVO_6:
    {
      return Servo_6;
      break;      
    } 
    case SERVO_7:
    {
      return Servo_7;
      break;      
    } 
    case SERVO_8:
    {
      return Servo_8;
      break;      
    }     
  }
}

void processSpiCommand()
{
  byte command = spi_receive_buffer[0];
  byte device = spi_receive_buffer[1];

  switch(command)
  {
    case READY_REPLY:
    {
      memset(spi_send_buffer, 0, sizeof(spi_send_buffer));
      break;      
    }
    case CHANGE_ANGLE_AND_SPEED:
    {
      int angle = spi_receive_buffer[2];
      int speed = spi_receive_buffer[3] | (spi_receive_buffer[4] << 8);

      rotateServo(getServo(device), angle, speed);
      spi_send_buffer[0] = RETURN_SUCCESS;
      spi_send_buffer[1] = device;
      break;      
    }
    case CALIBRATE_DEVICE:
    {
      spi_send_buffer[0] = RETURN_SUCCESS;
      spi_send_buffer[1] = device;
      break;      
    }
    default:
    {
      Serial.println("Wrong Command: " + String(command));
      break;
    }
  }
}

void loop() 
{
  if(process_spi_command == true)
  {
    processSpiCommand();

    process_spi_command = false;
    index = 0;
  }
}
