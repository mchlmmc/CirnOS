
ledPin = 47                    --> Initialize constant pin

pinMode(ledPin, OUTPUT)        --> Set pin 47 (LED) to output mode

print "Hello, World."

while true do                  --> Flicker LED 3 times then Spam 10 times
   delay(500)                  --> Delay 500ms
   writePin(ledPin, ON)        --> LED on
   delay(500)
   writePin(ledPin, OFF)       --> LED off
   delay(500)
   writePin(ledPin, ON)
   delay(500)
   writePin(ledPin, OFF)
   delay(500)
   writePin(ledPin, ON)
   delay(500)
   writePin(ledPin, OFF)
   delay(500)
   local state = OFF
   for i=1, 10 do              --> LED spam
     state = state == ON and OFF or ON
     writePin(ledPin, state)
     delay(100) -- (100ms * 10ms == 1000ms) aka 1 second
   end
end
