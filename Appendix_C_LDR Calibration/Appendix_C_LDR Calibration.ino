#include <MeMCore.h>

MeLightSensor lightSensor(PORT_8);
MeRGBLed led(0, 30);

int RGB[3][3] = {{255, 0, 0}, {0, 255, 0}, {0, 0, 255}};
int value;
int temp;

void setup() {
  led.setpin(13);
  
  Serial.begin(9600);

  for (int i = 0; i < 3; i++)
  {
    value = 0;
    led.setColor(0, RGB[i][0], RGB[i][1], RGB[i][2]);
    led.setColor(1, RGB[i][0], RGB[i][1], RGB[i][2]);
    led.show();
    delay(100);
    //Takes an average reading of 10 values
    for (int j = 0; j < 10; j++)
    {
      temp = lightSensor.read();
      Serial.print(temp);
      Serial.print(" ");
      value += temp;
      delay(50);
    }
    Serial.println("");
    if (i == 0) {
      Serial.print("Red: ");
    }
    else if (i == 1) {
      Serial.print("Green: ");
    }
    else {
      Serial.print("Blue: ");
    }
    Serial.println(value / 10);
  }

  led.setColor(0, 0, 0, 0);
  led.setColor(1, 0, 0, 0);
  led.show();
}

void loop() {
  // Everything is run once in main
}
