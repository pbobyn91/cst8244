 #!/bin/sh
#######################################################################
## - Test Script
## - Assignment 2
## - RT QNX Neutrino
## - Description: Test the functionlities of the metronome assignment
#######################################################################


 #Unit Test 1  : Demo B and C
 echo "********************************************"
 echo "Unit Test [1] \ncat /dev/local/metronom" 
 echo "Expected: [metronome: 120 beats/min, time signature 2/4, secs-per-interval: 0.25, nanoSecs: 250000000]"
 echo "\nOutput:"
 cat /dev/local/metronome
 echo "********************************************"
 sleep 10

echo "\n"
echo "\n"

#Unit Test 2 : Demo D
echo "********************************************"
echo "Unit Test [2] \ncat /dev/local/metronom-help"
echo "Expected: information regarding the metronome resmgr’s API, as seen above."
echo "\nOutput:"
cat /dev/local/metronome-help
echo "********************************************"
sleep 10

echo "\n"
echo "\n"


#Unit Test : Write to metronome-help
echo "********************************************"
echo "Unit Test [Additional] \necho stop > /dev/local/metronome-help"
echo "Expected: Cannot Write to Device /dev/local/metronome-help"
echo "Output:\n"
echo stop > /dev/local/metronome-help
echo "********************************************"
sleep 10

echo "\n"
echo "\n"

#Unit Test 3  : Demo E and F
echo "********************************************"
echo "Unit Test [3] \necho set 100 2 4 >/dev/local/metrnonome\ncat /dev/local/metronome"
echo "Expected: [metronome: 100 beats/min, time signature 2/4, secs-per- interval: 0.30, nanoSecs: 300000000]" 
echo set 100 2 4 > /dev/local/metronome
echo "\nOutput:"
cat /dev/local/metronome
echo "********************************************"
sleep 10

echo "\n"
echo "\n"

#Unit Test 4 : Demo G and H
echo "********************************************"
echo "Unit Test [4] \necho set 200 5 4 > /dev/local/metronome\ncat /dev/local/metronome" 
echo "Expected: [metronome: 200 beats/min, time signature 5/4, secs-per- interval: 0.15, nanoSecs: 150000000]"
echo set 200 5 4 > /dev/local/metronome
echo "\nOutput:"
cat /dev/local/metronome
echo "********************************************"
sleep 10

echo "\n"
echo "\n"

#Unit Test 5  : Demo I
echo "********************************************"
echo "Unit Test [5] \necho stop > /dev/local/metronome"
echo "Expected: Metronome stops running while resmgr still running" 
echo stop > /dev/local/metronome
echo "********************************************"
sleep 10

echo "\n"
echo "\n"

#Unit Test 6 : Demo J and K
echo "********************************************"
echo "Unit Test [6] \necho start > /dev/local/metronome\ncat /dev/local/metronome"
echo "Expected: [metronome: 200 beats/min, time signature 5/4, secs-per- interval: 0.15, nanoSecs: 150000000]"
echo start > /dev/local/metronome
echo "\nOutput:"
cat /dev/local/metronome
echo "********************************************"
sleep 10

echo "\n"
echo "\n"

#Unit Test 7 : Demo L
echo "********************************************"
echo "Unit Test [7] \necho stop > /dev/local/metronome"
echo "Expected: Metronome stops running while resmgr still running"  
echo stop > /dev/local/metronome
echo "********************************************"
sleep 10

echo "\n"
echo "\n"

#Unit Test 8 : Demo M
echo "********************************************"
echo "Unit Test [8] \necho stop > /dev/local/metronome"
echo "Expected: Metronome remains stopped running while resmgr still running"   
echo stop > /dev/local/metronome
echo "********************************************"
sleep 10

echo "\n"
echo "\n"

#Unit Test 9 : Demo N
echo "********************************************"
echo "Unit Test [9] \necho start > /dev/local/metronome"
echo "Expected: Metronome starts, at 200 5 4" 
echo start > /dev/local/metronome
echo "********************************************"
sleep 10

echo "\n"
echo "\n"

#Unit Test 10 Demo : O and P
echo "********************************************"
echo "Unit Test [10] \necho start > /dev/local/metronome\ncat /dev/local/metronome"
echo "Expected: [metronome: 200 beats/min, time signature 5/4, secs-per- interval: 0.15, nanoSecs: 150000000]" 
echo start > /dev/local/metronome
echo "\nOutput:"
cat /dev/local/metronome
echo "********************************************"
sleep 10

echo "\n"
echo "\n"


#Unit Test 12 : Demo Q
echo "********************************************"
echo "Unit Test [11] \necho pause 3 > /dev/local/metronome"
echo "Expected: metronome continues on next beat after 3 seconds (not next measure)"
echo pause 3 > /dev/local/metronome
echo "********************************************"
sleep 10

echo "\n"
echo "\n"

#Unit Test 13 : Demo R
echo "********************************************"
echo "Unit Test [12] \necho pause 10 > /dev/local/metronome"
echo "Expected: Error Message, max 9 second pause and minimum 1"
echo pause 10 > /dev/local/metronome
echo "********************************************"
sleep 10

echo "\n"
echo "\n"

#Unit Test 13 : Demo S
echo "********************************************"
echo "Unit Test [13] \necho bogus > /dev/local/metronome"
echo "Expected: Error Message: Invalid Command"
echo bogus > /dev/local/metronome
echo "********************************************"
sleep 10

echo "\n"
echo "\n"

#Unit Test 15 Demo : W
echo "********************************************"
echo "Unit Test [15] \necho quit > /dev/local/metronome"
echo "Expected: metronome gracefully terminates."
echo quit > /dev/local/metronome 
echo "********************************************"

















