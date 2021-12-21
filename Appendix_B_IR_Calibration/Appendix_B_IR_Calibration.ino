#define IRLEFT A1     //Left IR sensor is analog pin A1
#define IRRIGHT A0    //Right IR sensor is analog pin A0

void setup() {
    Serial.begin(9600);

}

void loop() {
  Serial.print(analogRead(IRLEFT));
  Serial.print(" ");
  Serial.println(analogRead(IRRIGHT));

}
