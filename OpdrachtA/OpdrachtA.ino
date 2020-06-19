// Include files.
#include <SPI.h>                  // Ethernet shield uses SPI-interface
#include <Ethernet.h>             // Ethernet library (use Ethernet2.h for new ethernet shield v2)

// Set Ethernet Shield MAC address  (check yours)
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED }; // Ethernet adapter shield S. Oosterhaven
int ethPort = 11250;                                  // Take a free port (check your router)

int buttonPin = 2;
boolean beenPressed = false;
boolean boxLocked = false;

EthernetServer server(ethPort);              // EthernetServer instance (listening on port <ethPort>)

void setup()
{
  Serial.begin(9600);
  pinMode(buttonPin, INPUT);

  Serial.println("Domotica project, Arduino Domotica Server\n");

  //Try to get an IP address from the DHCP server.
  if (Ethernet.begin(mac) == 0)
  {
    Serial.println("Could not obtain IP-address from DHCP -> do nothing");
    while (true){ }   // no point in carrying on, so do nothing forevermore; check your router
  }

  //Start the ethernet server.
  server.begin();
  Serial.print("Listening address: ");
  Serial.print(Ethernet.localIP());
  Serial.print("\t Listening port: ");
  Serial.println(ethPort);
}

void loop()
{
  // Listen for incomming connection (app)
  EthernetClient ethernetClient = server.available();
  if (!ethernetClient) {
    return; // wait for connection and blink LED
  }

  Serial.println("Application connected");

  // Do what needs to be done while the socket is connected.
  while (ethernetClient.connected()) 
  {
    if (ringBell()){
      doorBellPress();
    }

    // Execute command hen byte is received.
    while (ethernetClient.available())
    {
      char inByte = ethernetClient.read();
      executeCommand(inByte);
      inByte = NULL;
    } 
  }
  
  Serial.println("Application disonnected");
}

// Implementation of (simple) protocol between app and Arduino
// Response (to app) is 4 chars and ends with \n (not all commands demand a response)
void executeCommand(char cmd)
{     
  //*  Command explanation 
  //*  C:  Connect
  //*      Confirmation the app is connected to the arduino
  //*    Returns: OK on successful connection
  //*  R:  Received bell
  //*      Confirmation from the app, it received the bell press
  //*    Returns: nothing
  //*  L:  Close package box
  //*      Call the function that will close and lock the package box
  //*    Returns: nothing
  //*  O:  Open the package box
  //*      Calls the function that will open the package box.
  //*      Start function to watch if the weight sensor has changed
  //*    Returns: nothing
  //*  S: Status of package box
  //*    Return: CLS if box is closed and locked, returns OPN if box is unlocked
  
  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  // !!!!!!!!!!!!!  IMPORTANT  !!!!!!!!!!!!!
  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  
  // Responses need to be 4 bytes (see below, see Connection class in app) and need to end with with "\n"
  // Syntax to respond: server.write(buf, len). Note "len": you need to specify the length of the buffer (4). Don't forget!
  
  char buf[4] = {'\0', '\0', '\0', '\0'}; 
  
  // Command protocol
  Serial.print("["); Serial.print(cmd); Serial.print("] -> ");
  switch(cmd)
  {
    case 'c':
      //server.write(" OK\n",4);
      break;
    case 'r':
      beenPressed = false;
      break;
    case 'l':
      closePackageBox();
      break;
    case 'o':
      openPackageBox();
      break;
    case 's':
      if (checkBoxStatus())
        server.write("CLS\n", 4);
      else 
        server.write("OPN\n", 4);
      break;
  }
}

boolean ringBell() {
  if (beenPressed == false)
    if (digitalRead(buttonPin))
      return true;

  return false;
}

void doorBellPress() 
{
  Serial.println("\nBell has been pressed");
  beenPressed = true;
  server.write("BEL\n");
}

//Close and lock the package box
void closePackageBox()
{
  boxLocked = true;
}

//Open the package box, start to check if the weightsensor value has changed
void openPackageBox() 
{
  boxLocked = false;
}

//Check the status of the box
// False = open / unlocked
// True = Closed / locked
bool checkBoxStatus() 
{
  return boxLocked;
}
