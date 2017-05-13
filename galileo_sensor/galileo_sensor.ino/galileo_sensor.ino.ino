/*
  UDPSendReceive
 
 This sketch receives UDP message strings, prints them to the serial port
 and sends an "acknowledge" string back to the sender
 
 A Processing sketch is included at the end of file that can be used to send 
 and received messages for testing with a computer.
 
 created 21 Aug 2010
 by Michael Margolis
 
 This code is in the public domain.
 
 */


#include <SPI.h>          // needed for Arduino versions later than 0018
#include <Ethernet.h>
#include <EthernetUdp.h>  // UDP library from: bjoern@cs.stanford.edu 12/30/2008


// Enter a MAC address
byte mac[] = {  
  0x98, 0x4F, 0xEE, 0x02, 0x07, 0x0F };

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
  Serial.setTimeout(100);
  
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
      authenticate(remote_ip, "sensor");      
    }
  }

  String data = Serial.readString();
  if(data != "" && registered)
  {
   sendData(data); 
  }
  else if(data != "")
  {
    Serial.println("No registration to a server");
  }
}

void sendData(String data)
{
  int attempt = 0;
  boolean acked = false;
  while(!acked && attempt < 10)
  {
    attempt++;
    Serial.print("sending data ");
    Serial.println(data);
    data.toCharArray(sendBuffer, UDP_TX_PACKET_MAX_SIZE);
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
        if(contents == "ack")
        {
          acked = true;
          break;
        }
      }
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


