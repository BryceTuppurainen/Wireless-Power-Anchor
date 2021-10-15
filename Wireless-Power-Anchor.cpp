// Load Wi-Fi library
#include <WiFi.h>

// The delay inbetween the motor changing speed - alters the accelleration
#define delayChanging 200
#define maxDutyCycle 200
#define freq 5000
#define res 8

// Replace with your network credentials
const char *ssid = "ESP32-Access-Point";
const char *password = "123456789";

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// --------------------------------- Setting up the motor pins as an array (e.g. Access the enable pin of the second motor with motor[1][2])
// Where 0 - pin 1, 1 - pin2, 2 - enable for motors 0 and 1
int motor[2][3] = {{27, 33, 12}, {15, 32, 14}};
bool motorState[2] = {false, false};
int motorChn[2] = {0, 2};

int dutyCycle[2] = {0, 0};

void setup()
{
    Serial.begin(115200);
    // --------------------------------- Init. the motor pins (not enable, that gets attached to )
    for (int i = 0; i < 2; i++)
    {
        ledcSetup(motorChn[i], freq, res);
        ledcAttachPin(motor[i][2], motorChn[i]);
        for (int j = 0; j < 2; j++)
            pinMode(motor[i][j], OUTPUT);
    }

    // --------------------------------- Init. the Wifi point and connect to WiFi --------------------------------

    // Connect to Wi-Fi network with SSID and password
    Serial.print("Setting AP (Access Point)...");
    // Remove the password parameter, if you want the AP (Access Point) to be open
    WiFi.softAP(ssid, password);

    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP);

    server.begin();
}

void loop()
{
    WiFiClient client = server.available(); // Listen for incoming clients

    if (client)
    {                                  // If a new client connects,
        Serial.println("New Client."); // print a message out in the serial port
        String currentLine = "";       // make a String to hold incoming data from the client
        while (client.connected())
        { // loop while the client's connected
            if (client.available())
            {                           // if there's bytes to read from the client,
                char c = client.read(); // read a byte, then
                Serial.write(c);        // print it out the serial monitor
                header += c;
                if (c == '\n')
                { // if the byte is a newline character
                    // if the current line is blank, you got two newline characters in a row.
                    // that's the end of the client HTTP request, so send a response:
                    if (currentLine.length() == 0)
                    {
                        // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
                        // and a content-type so the client knows what's coming, then a blank line:
                        client.println("HTTP/1.1 200 OK");
                        client.println("Content-type:text/html");
                        client.println("Connection: close");
                        client.println();

                        // turns the GPIOs on and off
                        // **************************** Here is the code we are after, all of the code surrounding this handles the webserver itself ********

                        if (header.indexOf("GET /" + motor[0][0] + "/on") >= 0)
                        { // Start motor A
                            Serial.println("Motor on GPIO " + motor[0][0] + " starting");
                            motorState[0] = true;

                            digitalWrite(motor[0][0], HIGH);
                            digitalWrite(motor[0][1], LOW);

                            while (dutyCycle[0] <= maxDutyCycle)
                            {
                                ledcWrite(motor[0][3], dutyCycle[0]);

                                Serial.print("Forward with duty cycle (MotorA): ");
                                Serial.println(dutyCycle[0]);

                                dutyCycle[0] = dutyCycle[0] + 5;
                                delay(delayChanging);
                            }
                        }
                        else if (header.indexOf("GET /" + motor[0][0] + "/off") >= 0)
                        { // Stop motor A
                            Serial.println("Motor on GPIO " + motor[0][0] + " stopping");
                            motorState[0] = false;

                            dutyCycle[0] = 0;
                            ledcWrite(motor[0][3], dutyCycle[0]);
                        }
                        else if (header.indexOf("GET /" + motor[1][0] + "/on") >= 0)
                        { // Start motor B
                            Serial.println("Motor on GPIO " + motor[1][0] + " starting");
                            motorState[1] = true;

                            digitalWrite(motor2Pin1, HIGH);
                            digitalWrite(motor2Pin2, LOW);

                            while (dutyCycle[1] <= maxDutyCycle)
                            {
                                ledcWrite(motor[1][2], dutyCycle[1]);

                                Serial.print("Forward with duty cycle (MotorB): ");
                                Serial.println(dutyCycle[1]);

                                dutyCycle[1] = dutyCycle[1] + 5;
                                delay(delayChanging);
                            }
                        }
                        else if (header.indexOf("GET /" + motor[1][0] + "/off") >= 0)
                        { // Stop motor B
                            Serial.println("Motor on GPIO " + motor[1][0] + " stopping");
                            motorState[1] = false;

                            dutyCycle[1] = 0;
                            ledcWrite(motor[0][3], dutyCycle[1]);
                        }

                        // Display the HTML web page
                        client.println("<!DOCTYPE html><html>");
                        client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
                        client.println("<link rel=\"icon\" href=\"data:,\">");
                        // CSS to style the on/off buttons
                        // Feel free to change the background-color and font-size attributes to fit your preferences
                        client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
                        client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
                        client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
                        client.println(".button2 {background-color: #555555;}</style></head>");

                        // Web Page Heading
                        client.println("<body><h1>ESP32 Web Server</h1>");

                        // Display current state, and ON/OFF buttons for GPIO 27
                        motorState[0] ? client.println("<p>GPIO 27 - State OFF</p>") : client.println("<p>GPIO 27 - State ON</p>");

                        // If the first motor is off, it displays the ON button, or the off button
                        if (!motorState[0])
                        {
                            client.println("<p><a href=\"/" + motor[0][0] + "/on\"><button class=\"button\">ON</button></a></p>");
                        }
                        else
                        {
                            client.println("<p><a href=\"/" + motor[0][0] + "/off\"><button class=\"button button2\">OFF</button></a></p>");
                        }

                        // Display current state, and ON/OFF buttons for GPIO 15
                        client.println("<p>GPIO 15 - State " + motorState[1] + "</p>");
                        // If the motorState[1] is off, it displays the ON button
                        if (motorState[1] == "off")
                        {
                            client.println("<p><a href=\"/" + motor[1][0] + "/on\"><button class=\"button\">ON</button></a></p>");
                        }
                        else
                        {
                            client.println("<p><a href=\"/" + motor[1][0] + "/off\"><button class=\"button button2\">OFF</button></a></p>");
                        }
                        client.println("</body></html>");

                        // The HTTP response ends with another blank line
                        client.println();
                        // Break out of the while loop
                        break;
                    }
                    else
                    { // if you got a newline, then clear currentLine
                        currentLine = "";
                    }
                }
                else if (c != '\r')
                {                     // if you got anything else but a carriage return character,
                    currentLine += c; // add it to the end of the currentLine
                }
            }
        }
        // Clear the header variable
        header = "";
        // Close the connection
        client.stop();
        Serial.println("Client disconnected.");
        Serial.println("");
    }
}