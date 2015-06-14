#include <SPI.h>
#include <SD.h>

boolean turn = true;

File myFile;
boolean LEDstate[14];
boolean pingstate[14];
long prevping[14];
int analogstate[14];

long waitUntilTurn = 30000;
long waitUntilPing = 0;
long waitUntilCPU = 0;
long waitUntilMemory = 0;

void setup()
{
  for (int i = 0; i < 14; i++) {
    analogstate[i] = 200;
    LEDstate[i] = true;
    pingstate[i] = false;
    prevping[i] = 0;
  }
  
  Serial.begin(9600);
  while (!Serial) {
    ; 
  }

  Serial.print("Initializing SD card...");

  pinMode(11, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(3, OUTPUT);

  if (!SD.begin(4)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");

}


void ping(char* command, int led)
{
  digitalWrite(led, analogstate[led]);
  double dout;

  if (millis() >= waitUntilPing){
    system(command);
    
    myFile = SD.open("pingresult.txt");
    if (myFile) {
      
      if (!myFile.available()){
        pingstate[led] = false;
      }
      else{
        char out[4];
        for (int i = 0; i < 4; i++) {
          out[i] = (char)myFile.read();
        }

        dout = atof(out);
        
        pingstate[led] = true;
      }

      myFile.close();
    }
    else {
      Serial.println("error opening test.txt");
    }

    waitUntilPing = millis() + 5000;
  }

  if (!pingstate[led]) {
    if (millis() >= prevping[led]){
      if (analogstate[led] > 0){
        analogstate[led] = 0;  
      }
      else{
        analogstate[led] = 200;
      }
      
      prevping[led] = millis() + 1000;
    }
  }
  else{
    analogstate[led] = (int) (255 * (1 - dout/2000));
  }

}

void cpu(char* command, int led_on, int led_down)
{
  if (millis() >= waitUntilCPU){
    system(command);
    
    myFile = SD.open("cpuresult.txt");
    if (myFile) {
      char out[4];
      for (int i = 0; i < 4; i++) {
        out[i] = (char)myFile.read();
      }
      
      double dout = atof(out);
      Serial.println(dout);
        
      analogWrite(led_on, (int)255*dout/100);
      analogWrite(led_down, 255 - ((int)255*dout/100));
      
      myFile.close();
    }
    else {
      Serial.println("error opening test.txt");
    }

    waitUntilCPU = millis() + 5000;
  }
}


void memory(char* command, int led_on, int led_down)
{
  if (millis() >= waitUntilMemory){
    system(command);

    myFile = SD.open("memoryresult.txt");
    if (myFile) {
      char out[4];
      for (int i = 0; i < 4; i++) {
        out[i] = (char)myFile.read();
      }
      
      double dout = atof(out);
      Serial.println(dout);
        
      analogWrite(led_on, (int)255 * dout);
      analogWrite(led_down, (int)255 * (1-dout));
      
      // close the file:
      myFile.close();
    }
    else {
      Serial.println("error opening test.txt");
    }

    waitUntilMemory = millis() + 5000;
  }
}


void loop()
{
  if (turn){
    ping("ping -c 1 -W 2 linux1.csie.ntu.edu.tw | grep 'time=.* ' -o | grep '[^time=].*' -o > /media/mmcblk0p1/pingresult.txt", 11);
    cpu("ssh -i /home/root/.ssh/id_rsa b01705041@linux1.csie.ntu.edu.tw \"top -n1 -b | head -n3 | tail -n1\" | awk '{ print $8 }' > /media/mmcblk0p1/cpuresult.txt", 3, 5);
    memory("ssh -i /home/root/.ssh/id_rsa b01705041@linux1.csie.ntu.edu.tw \"free -t\" | sed -n '2,2p' | awk '{ print $4/$2 }' > /media/mmcblk0p1/memoryresult.txt", 6, 9);
    digitalWrite(7, HIGH);
    digitalWrite(8, LOW);
  }
  else{
    ping("ping -c 1 -W 2 linux2.csie.ntu.edu.tw | grep 'time=.* ' -o | grep '[^time=].*' -o > /media/mmcblk0p1/pingresult.txt", 11);
    cpu("ssh -i /home/root/.ssh/id_rsa b01705041@linux2.csie.ntu.edu.tw \"top -n1 -b | head -n3 | tail -n1\" | awk '{ print $8 }' > /media/mmcblk0p1/cpuresult.txt", 3, 5);
    memory("ssh -i /home/root/.ssh/id_rsa b01705041@linux2.csie.ntu.edu.tw \"free -t\" | sed -n '2,2p' | awk '{ print $4/$2 }' > /media/mmcblk0p1/memoryresult.txt", 6, 9);
    digitalWrite(8, HIGH);
    digitalWrite(7, LOW);
  }

  if (millis() >= waitUntilTurn){
    turn = !turn;
    waitUntilTurn = millis() + 15000;
  }
   
}

