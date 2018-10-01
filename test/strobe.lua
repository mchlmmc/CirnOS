
ledPin = 47                    --> Initialize constant pin

pinMode(ledPin, OUTPUT)        --> Set pin 47 (LED) to output mode

print "Hello, World."

while true do                  --> Flicker LED 3 times then Spam 10 times
   delay(500)                  --> Delay 500ms
   writePin(ledPin, ON)        --> LED on
   delay(500)
   writePin(ledPin, OFF)       --> LED off
end
