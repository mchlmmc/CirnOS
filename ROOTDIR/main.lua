ledPin = 47                    --> Initialize constant pin

pinMode(ledPin, OUTPUT)        --> Set pin 47 (LED) to output mode

print "Hello, World."

while true do
   delay(500)                  --> Wait half a second
   writePin(ledPin, ON)        --> LED on
   delay(500)   
   writePin(ledPin, OFF)       --> LED off
end