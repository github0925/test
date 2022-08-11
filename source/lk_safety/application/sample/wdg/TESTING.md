INPUT "wdt"
------------------------
Tend to refresh wdt mode1[win_low,termination]
Tend to refresh wdt mode0
Tend to generate wdt reset interrupt
Tend to generate wdt underflow int and mode1 int


In order to test this example, the process is the following:

Tested with IAR and GCC ( ddram configration)

Step | Description | Expected Result | Result
-----|-------------|-----------------|-------
Press '1' | WDT will be refresh every 8 seconds | PASSED | PASSED
Press '2' | WDT has been Reset after 5s| PASSED | PASSED
Press '3' | WDT will be reset every 10 seconds | PASSED | PASSED
Press '4' | System will be restarted by WDT after 10 seconds| PASSED | PASSED