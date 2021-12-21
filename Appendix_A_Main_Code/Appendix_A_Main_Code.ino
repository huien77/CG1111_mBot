#include <MeMCore.h>

MeDCMotor motor1(M1); //Motor1 is Left Motor
MeDCMotor motor2(M2); //Motor2 is Right Motor
MeLineFollower lineFinder(PORT_2);
MeUltrasonicSensor ultrasonic(PORT_3);
MeLightSensor lightSensor(PORT_8);
MeRGBLed led(0, 30);
MeBuzzer buzzer;

#define IRLEFT A1     //Left IR sensor is analog pin A1
#define IRRIGHT A0    //Right IR sensor is analog pin A0
#define IRVOLTAGELEFT 200   //Acceptable threshold voltage reading of IR sensors
#define IRVOLTAGERIGHT 70  //Acceptable threshold voltage reading of IR sensors
#define IRVOLTAGELEFTUTURN 420
#define IRVOLTAGERIGHTUTURN 270

#define FORWARDSPEED 190  //Speed of the mBot
#define TURNSPEED 115     //Turn speed of the mBot
#define BOUNDARY 40       //Boundary for the light sensing
#define TWOTURNSPEED 130  //Forward speed during a double turn

#define FRONTWALL 15 //Distance(cm) away from front wall for normal forward movement
#define FRONTWALLTWOTURN 8 //Distance(cm) away from front wall during a two turn

#define DN 294
#define GN 392
#define AN 440
#define BN 494
#define HCN 524
#define HDN 587
#define HGN 784

float speed_multiplier = 1.10;
float uturn_multiplier = 0.8;

int red_value;
int green_value;
int blue_value;

/*Update the following based on tested values (0 - 1023)
  Red, Green, Yellow, Purple, Light-Blue, Black */
int color[6][3] = {{693,369,406},{527,443,420},{765,517,451},{615,441,548},{629,523,579},{485,342,378}};

bool run_program = false;

void setup() {
  Serial.begin(9600);
  pinMode(7, INPUT); //Define button pin as input
  led.setpin(13);
}

void loop() {
  //Waits for button press to start program
  while (!run_program)
  {
    start_program();
  }
  delay(500);
  
  //Runs the following after button press
  while (run_program)
  {
    //Checks IR Proximity reading
    forward_correction();
    
    //Slows down mBot to allow time for mBot to detect black line
    if (ultrasonic.distanceCm() < FRONTWALL)
    {
      slow_down();
    }

    //Need to code for change in offset in forward based on detection by IR sensor
    
    //Perform waypoint challenges
    if (lineFinder.readSensors() == S1_IN_S2_IN)
    {
      stop_moving();
      
      red_value = light_measure(255, 0, 0);
      green_value = light_measure(0, 255, 0);
      blue_value = light_measure(0, 0, 255);

      if (color_check(red_value, green_value, blue_value, 0))
      {
        left_turn(); //Red paper detected
      }
      else if (color_check(red_value, green_value, blue_value, 1))
      {
        right_turn(); //Green paper detected
      }
      else if (color_check(red_value, green_value, blue_value, 2))
      {
        u_turn(); //Yellow paper detected
      }
      else if (color_check(red_value, green_value, blue_value, 3))
      {
        two_left_turn(); //Purple paper detected
      }
      else if (color_check(red_value, green_value, blue_value, 4))
      {
        two_right_turn(); //Light Blue paper detected
      }
      else if (color_check(red_value, green_value, blue_value, 5))
      {
        //End of program
        stop_moving();
        victory_tune();
        run_program = false;
      }
    }
  }
}

//Starts program on button press
void start_program() {
  if (analogRead(7) < 100)
  {
    run_program = true;
  }
}

//Cuts the mBot speed by half
void slow_down() {
  motor1.run(-(FORWARDSPEED * (speed_multiplier * 2.5)) / 2); //Motor1 (Left)  forward is -negative
  motor2.run(FORWARDSPEED / 2);  //Motor2 (Right) forward is +positive
}

//Stop the mBot (use after any mBot movement)
void stop_moving() {
  motor1.stop(); //Stop Motor1 
  motor2.stop(); //Stop Motor1 
}

//Changes speed multiplier to account for drifting
void forward_correction() {
  if (analogRead(IRLEFT) < IRVOLTAGELEFT)
  {
      motor2.run(3); //Motor1 (Left)  forward is -negative
      delay(10);
  }
  if (analogRead(IRRIGHT) < IRVOLTAGERIGHT)
  {
      motor1.run(-3);  //Motor2 (Right) forward is +positive
      delay(10);
  }
  motor1.run(-(FORWARDSPEED * speed_multiplier)); //Motor1 (Left)  forward is -negative
  motor2.run(FORWARDSPEED);  //Motor2 (Right) forward is +positive
}

//Measure Light Intensity
int light_measure(int R, int G, int B) {
  int sum = 0;
  led.setColor(0, R, G, B);
  led.setColor(1, R, G, B);
  led.show();
  delay(50);
  
  //Takes an average reading of 10 values
  for (int i = 0; i < 10; i++)
  {
    sum += lightSensor.read();
    delay(10);
  }

  //Turns off LED
  led.setColor(0, 0, 0, 0);
  led.setColor(1, 0, 0, 0);
  led.show();
    
  return sum / 10;
}

//Check against callibrated RGB values
bool color_check(int measured_red, int measured_green, int measured_blue, int index) {
  bool check_1 = (measured_red > (color[index][0] - BOUNDARY)) && (measured_red < (color[index][0] + BOUNDARY));
  bool check_2 = (measured_green > (color[index][1] - BOUNDARY)) && (measured_green < (color[index][1] + BOUNDARY));
  bool check_3 = (measured_blue > (color[index][2] - BOUNDARY)) && (measured_blue < (color[index][2] + BOUNDARY));
  return check_1 && check_2 && check_3;
}

//Rotate mBot 90 degrees anticlockwise
void left_turn() { 
  motor1.run(TURNSPEED); //Motor1 (Left)  backward is +positive
  motor2.run(TURNSPEED); //Motor2 (Right) forward is +positive
  delay(500);
  stop_moving();
}

//Rotate mBot 90 degrees clockwise
void right_turn() {
  motor1.run(-TURNSPEED + 20); //Motor1 (Left)  forward is -negative
  motor2.run(-TURNSPEED); //Motor2 (Right) backward is -negative
  delay(500);
  stop_moving();
}

//Rotate mBot 180 degrees
void u_turn() {
  if (analogRead(IRRIGHT) < IRVOLTAGERIGHTUTURN)
  {
    motor1.run(TURNSPEED * 2 * uturn_multiplier); //Motor1 (Left)  backward is +positive
    motor2.run(TURNSPEED * 2 * uturn_multiplier); //Motor2 (Right) forward is +positive
  }
  else
  {
    motor1.run(- (TURNSPEED * 2 * uturn_multiplier)); //Motor1 (Left)  backward is +positive
    motor2.run(- (TURNSPEED * 2 * uturn_multiplier)); //Motor2 (Right) forward is +positive    
  }
  delay(500);
  stop_moving();
}

/* Rotate mBot 90 degrees anticlockwise
 * Move forward by one tile
 * Rotate mBot 90 degrees anticlockwise
 */
void two_left_turn() {
  left_turn();
  delay(100);
  while(ultrasonic.distanceCm() > FRONTWALLTWOTURN) {
    motor1.run(-(TWOTURNSPEED));
    motor2.run(TWOTURNSPEED);
  }
  left_turn();
}

/* Rotate mBot 90 degrees clockwise
 * Move forward by one tile
 * Rotate mBot 90 degrees clockwise
 */
void two_right_turn() {
  right_turn();
  delay(100);
  while(ultrasonic.distanceCm() > FRONTWALLTWOTURN) {
    motor1.run(-(TWOTURNSPEED));
    motor2.run(TWOTURNSPEED);
  }
  right_turn();
}

//Play first verse of Star Wars Main Theme
void victory_tune() {
  int frequency[36] = {DN, DN, DN, GN, GN, GN, GN, HDN, HDN, HDN, HDN, HCN, BN, AN, HGN, HGN, HGN, HGN, HDN, HDN, HCN, BN, AN, HGN, HGN, HGN, HGN, HDN, HDN, HCN, BN, HCN, AN, AN, AN, AN};
  for(int i = 0; i < 36; i++)
  {
    buzzer.tone(frequency[i], 273);
  }
}
