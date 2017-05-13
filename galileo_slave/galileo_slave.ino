/*
 galileo_slave
 
 */


#include <SPI.h>          // needed for Arduino versions later than 0018
#include <Ethernet.h>
#include <EthernetUdp.h>  // UDP library from: bjoern@cs.stanford.edu 12/30/2008
#define DEBUG true

int led = 12;
// Enter a MAC address
byte mac[] = {  
  0x98, 0x4F, 0xEE, 0x02, 0xD5, 0x92 };

//const int UDP_TX_PACKET_MAX_SIZE = 1024;
unsigned int localPort = 8888;              // local port to listen on
IPAddress server_ip;
boolean registered = false;

// buffers for receiving and sending data
char recvBuffer[UDP_TX_PACKET_MAX_SIZE];
char sendBuffer[UDP_TX_PACKET_MAX_SIZE];

// An EthernetUDP instance to let us send and receive packets over UDP
EthernetUDP Udp;

void setup() {
  // start the Ethernet and UDP:
  system("ifup eth0");
  Serial.begin(9600);
  Serial.setTimeout(500);
  Serial1.begin(9600);
  Serial1.setTimeout(500);
  
  // start the Ethernet connection:
  Serial.println("Trying to get an IP address using DHCP");
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
  }
  // print your local IP address:
  Serial.print("My IP address: ");
  IPAddress ip = Ethernet.localIP();
  printIP(ip);
  Serial.println();
  
  Udp.begin(localPort);
  pinMode(led, OUTPUT);

  sendCommand("AT+CWSAP=\"Arduino\",\"password\",3,0\r\n", 1000, DEBUG);
  sendCommand("AT+CIPMUX=1\r\n", 1000, DEBUG);
  sendCommand("AT+CIPSERVER=1,80\r\n", 1000, DEBUG);
  sendCommand("AT+CIFSR\r\n", 1000, DEBUG);
}

void loop() {
  
  // if there's data available, read a packet
  int packetSize = Udp.parsePacket();
  if(packetSize)
  {
    IPAddress remote_ip = Udp.remoteIP();
    printIP(remote_ip);
    String contents = readPacket(packetSize);

    if(contents == "authenticate")
    {
      authenticate(remote_ip, "actuator");
      
    }
    else if(registered && remote_ip == server_ip)
    {
      if(contents == "1")
      {
        digitalWrite(led, HIGH);
      }
      else if(contents == "2")
      {
        digitalWrite(led, LOW);
      }
    }
    
  }

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


void authenticate(IPAddress remote_ip, String registration)
{
  server_ip = remote_ip;
  registered = false;
  int attempt = 0;
  while(!registered && attempt < 10)
  {
    attempt++;
    Serial.print("Authenticating on ");
    printIP(server_ip);
    Serial.println(localPort);
    registration.toCharArray(sendBuffer, UDP_TX_PACKET_MAX_SIZE);
    Udp.beginPacket(server_ip, localPort);
    Udp.write(sendBuffer);
    Udp.endPacket();

      //wait for registration ack for 1 sec
    unsigned long t0 = millis();
    while(millis() - t0 < 1000)
    {
      int packetSize = Udp.parsePacket();
      if(packetSize)
      {
        String contents = readPacket(packetSize);
        if(contents == "registered")
        {
          registered = true;
          break;
        }
      }
    }
  }
  if(registered)
  {
    Serial.println("Authenticated");
  }
}

String readPacket(int packetSize)
{
  String contents = "";
  Serial.print("Received packet of size ");
  Serial.println(packetSize);

    // read the packet into packetBufffer
  Udp.read(recvBuffer,UDP_TX_PACKET_MAX_SIZE);
  for(int i = 0;i<packetSize;i++)
  {
    contents += recvBuffer[i];
  }
    
  Serial.println("Contents:");
  Serial.println(contents);
  return contents;
}

void printIP(IPAddress ip)
{
  for (byte thisByte = 0; thisByte < 4; thisByte++) {
    // print the value of each byte of the IP address:
    Serial.print(ip[thisByte], DEC);
    Serial.print("."); 
  }
  Serial.println();
}

//wifi esp8266 stuff---------
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
   String command = Serial.readString();
   return command;
}

String sendCommand(String command, const int timeout, boolean debug)
{
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
