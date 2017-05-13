
//#include <SoftwareSerial.h>
#define DEBUG true

char data;
int led = 12;
//SoftwareSerial esp(10,11); //rx = 10 ; tx = 11;
String response;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.setTimeout(500);
  Serial1.begin(9600);
  Serial1.setTimeout(500);
  pinMode(led, OUTPUT);

  sendCommand("AT+CWSAP=\"Arduino\",\"password\",3,0\r\n", 1000, DEBUG);
  sendCommand("AT+CIPMUX=1\r\n", 1000, DEBUG);
  sendCommand("AT+CIPSERVER=1,80\r\n", 1000, DEBUG);
  sendCommand("AT+CIFSR\r\n", 1000, DEBUG);
}

void loop() {
  // put your main code here, to run repeatedly:
  String command = readCommand();
  if(command != "")
  {
    sendCommand(command, 2000, DEBUG);
  }
  
  if(Serial1.available())
  {
   String sig = readSignal(DEBUG);
    if(sig == "1")
    {
      digitalWrite(led, HIGH);
    }
    else if(sig == "2")
    {
      digitalWrite(led, LOW);
    }
  }
}

String readSignal(boolean debug)
{
  String sig = "default";
  String response = Serial1.readString();

  int st_pos = response.indexOf(',');
  int end_pos = response.indexOf(',', st_pos+1);
  String conn_id = response.substring(st_pos+1, end_pos);
  
  int pos = response.indexOf(':');
  if(conn_id == "0" && pos != -1){
    sig = response.substring(pos+1);
  }
  sig.trim();
  if(debug)
  {
    Serial.println("response is "+response); 
    Serial.println("signal is "+sig);
  }
  return sig;
}

String readCommand()
{
    String command = "";
    char c;
    
  while(Serial.available())
      {
        c = Serial.read(); // read the next character.
        if (c == '$')
        {
          break;
        }     
        command+=c;
        delay(500);
      }
    return command;
}

String sendCommand(String command, const int timeout, boolean debug)
{
  Serial.println(command);
    String response = "";
    Serial1.println(command); // send the read character to the esp8266
    
    long int time = millis();
    
    while( (time+timeout) > millis())
    {
      while(Serial1.available())
      {
        
        // The esp has data so display its output to the serial window 
        char c = Serial1.read(); // read the next character.
        response+=c;
      }  
    }
    
    if(debug)
    {
      Serial.print(response);
    }
    
    return response;
}
