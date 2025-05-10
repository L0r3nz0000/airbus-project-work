int a = 3;
int b = 5;
int c = 6;
int d = 9;
int e = 10;
int f = 11;
int g = 12;

void setup() {

  pinMode(c, OUTPUT);
  pinMode(b, OUTPUT);
  pinMode(a, OUTPUT);
  pinMode(d, OUTPUT);
  pinMode(e, OUTPUT);
  pinMode(f, OUTPUT);
  pinMode(g, OUTPUT);
  pinMode(2, INPUT);
  Serial.begin(9600);
}

int number = 0;
bool reset = false;

void loop() {
  int brightnessA = random(1, 8);
  int brightnessB = random(1, 8);
  int brightnessC = random(1, 8);
  int brightnessD = random(1, 8);
  int brightnessE = random(1, 8);
  int brightnessF = random(1, 8);
  int brightnessG = random(1, 8);
  number++;
  number %= 10;

  if (reset) {
    number = 0;
    reset = false;
  }

  switch (number) {
    case 0:
      analogWrite(a, brightnessA);
      analogWrite(b, brightnessB);
      analogWrite(c, brightnessC);
      analogWrite(d, brightnessD);
      analogWrite(e, brightnessE);
      analogWrite(f, brightnessF);
      digitalWrite(g, LOW);
      break;
    case 1:
      digitalWrite(a, LOW);
      analogWrite(b, brightnessB);
      analogWrite(c, brightnessC);
      digitalWrite(d, LOW);
      digitalWrite(e, LOW);
      digitalWrite(f, LOW);
      digitalWrite(g, LOW);
      break;
    case 2:
      analogWrite(a, brightnessA);
      analogWrite(b, brightnessB);
      digitalWrite(c, LOW);
      analogWrite(d, brightnessD);
      analogWrite(e, brightnessE);
      digitalWrite(f, LOW);
      digitalWrite(g, HIGH);
      break;
    case 3:
      analogWrite(a, brightnessA);
      analogWrite(b, brightnessB);
      analogWrite(c, brightnessC);
      analogWrite(d, brightnessD);
      digitalWrite(e, LOW);
      digitalWrite(f, LOW);
      digitalWrite(g, HIGH);
      break;
    case 4:
      digitalWrite(a, LOW);
      analogWrite(b, brightnessB);
      analogWrite(c, brightnessC);
      digitalWrite(d, LOW);
      digitalWrite(e, LOW);
      analogWrite(f, brightnessF);
      digitalWrite(g, HIGH);
      break;
    case 5:
      analogWrite(a, brightnessA);
      digitalWrite(b, LOW);
      analogWrite(c, brightnessC);
      analogWrite(d, brightnessD);
      digitalWrite(e, LOW);
      analogWrite(f, brightnessF);
      digitalWrite(g, HIGH);
      break;
    case 6:
      analogWrite(a, brightnessA);
      digitalWrite(b, LOW);
      analogWrite(c, brightnessC);
      analogWrite(d, brightnessD);
      analogWrite(e, brightnessE);
      analogWrite(f, brightnessF);
      digitalWrite(g, HIGH);
      break;
    case 7:
      analogWrite(a, brightnessA);
      analogWrite(b, brightnessB);
      analogWrite(c, brightnessC);
      digitalWrite(d, LOW);
      digitalWrite(e, LOW);
      digitalWrite(f, LOW);
      digitalWrite(g, LOW);
      break;
    case 8:
      analogWrite(a, brightnessA);
      analogWrite(b, brightnessB);
      analogWrite(c, brightnessC);
      analogWrite(d, brightnessD);
      analogWrite(e, brightnessE);
      analogWrite(f, brightnessF);
      digitalWrite(g, HIGH);
      break;
    case 9:
      analogWrite(a, brightnessA);
      analogWrite(b, brightnessB);
      analogWrite(c, brightnessC);
      analogWrite(d, brightnessD);
      digitalWrite(e, LOW);
      analogWrite(f, brightnessF);
      digitalWrite(g, HIGH);
      break;
    default:
      digitalWrite(a, LOW);
      digitalWrite(b, LOW);
      digitalWrite(c, LOW);
      digitalWrite(d, LOW);
      digitalWrite(e, LOW);
      digitalWrite(f, LOW);
      digitalWrite(g, HIGH);
  }

  
  Serial.print("Mostro il numero: ");
  Serial.println(number);

  unsigned long start = millis();
  while (digitalRead(2) == HIGH) {
    if (millis() - start >= 20) {
      digitalWrite(a, LOW);
      digitalWrite(b, LOW);
      digitalWrite(c, LOW);
      digitalWrite(d, LOW);
      digitalWrite(e, LOW);
      digitalWrite(f, LOW);
      digitalWrite(g, LOW);
      for (int i = 0; i < 30; i++) {
        digitalWrite(LED_BUILTIN, HIGH);
        delay(30);
        digitalWrite(LED_BUILTIN, LOW);
        delay(30);
      }
      reset = true;
    }
  }
  while (digitalRead(2) == LOW);
}
