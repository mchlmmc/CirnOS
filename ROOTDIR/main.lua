ledPin = 47                    --> Initialize constant pin

pinMode(ledPin, output)        --> Set pin 47 (LED) to output mode

print "I like blinking ^_^"    

while true do
   delay(500)                  --> Wait half a second
   writePin(ledPin, on)        --> LED on
   delay(500)   
   writePin(ledPin, off)       --> LED off
end
