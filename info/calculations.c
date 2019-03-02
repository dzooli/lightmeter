float A[] = {1, 1.4, 2, 2.8, 4, 5.6, 8, 11, 16, 22, 32, 45, 64, 90};                          //Array of aperture values
float ISOs[] = {6, 12, 25, 50, 100, 200, 400, 800, 1600, 3200, 6400, 12800, 25600, 51200};       //Array of sensitivity values
//                                               1/8    1/15     1/30     1/60    1/125  0/250  1/500  1/1000  1/2000  1/4000   1/8000
//float T[] = {60, 30, 15, 8, 4, 2, 1, 0.5, 0.25, 0.125, 0.06666, 0.03333, 0.01666, 0.008, 0.004, 0.002, 0.001,  0.0005, 0.00025, 0.000125} //Array of speed values

# define K 64       // Calibration constant for this sample routine
//# define K 78.125   // Calibration constant based on the Sunny16 rule and "Sunny"=20000Lx

/*
    Base formula for reflected light meter:
    
    N^2   LS
    --- = --
    t     K
    
    Where:
        N: aperture value
        t: exposure time
        L: luminance
        S: ISO sensitivity
        K: calibration constant
                        
    # define K 64       // Calibration constant for this sample routine
    # define K 78.125   // Calibration constant based on the Sunny16 rule and "Sunny"=20000Lx
    
    // Exposure value (EV) calculation from the aperture and exposure time
    Ev = log(pow(A[Am],2))/log(2) + log(1/T)/log(2);
    display.println(floor(Ev+0.5),1);
    
    // Exposure time calculation
    float T = pow(A[Am],2)*K/(lux*ISOs[Sm]);                  //T = exposure time, in seconds
    
    // Aperture size calculation
    float Am = sqrt((lux*ISOs[Sm]*T)/K));
*/    
 
void refresh()                                              //Calling the function gives a new exposure calculation based on the last illuminance value, and refreshes the display.
{       
    float T = pow(A[Am],2)*K/(lux*ISOs[Sm]);                  //T = exposure time, in seconds
      if (T >= 60)                                 
      {
        Tdisplay = 0;  //Exposure is now in minutes
        Tmin = T/60;
      }
      else if (T < 0.75)
      {
        Tdisplay = 1;  //Exposure is now in fractional form
        if (T < 0.000125) {Tdisplay = 3;}
        if ((T <= 0.000188) && (T > 0.000125)) {Tfr = 8000;}
        if ((T <= 0.000375) && (T > 0.000188)) {Tfr = 4000;}
        if ((T <= 0.00075) && (T > 0.000375)) {Tfr = 2000;}
        if ((T <= 0.0015) && (T > 0.00075)) {Tfr = 1000;}
        if ((T <= 0.003) && (T > 0.0015)) {Tfr = 500;}
        if ((T <= 0.006) && (T > 0.003)) {Tfr = 250;}
        if ((T <= 0.012333) && (T > 0.006)) {Tfr = 125;}
        if ((T <= 0.025) && (T > 0.012333)) {Tfr = 60;}
        if ((T <= 0.05) && (T > 0.025)) {Tfr = 30;}
        if ((T <= 0.095833) && (T > 0.05)) {Tfr = 15;}
        if ((T <= 0.1875) && (T > 0.095833)) {Tfr = 8;}
        if ((T <= 0.375) && (T > 0.1875)) {Tfr = 4;}
        if ((T <= 0.75) && (T > 0.375)) {Tfr = 2;}  
      }
      else if ((T>=0.75)&&(T<60))
      {
        Tdisplay = 2;  //Exposure in seconds
      }
      if (lux == 0)                                        //This happens if the sensor is overloaded or senses no light.
      {
        Tdisplay = 3;
      }
    
    Serial.println(Tdisplay);
    
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    display.print("f/");
    display.println(A[Am], 1);
    
      if (Tdisplay == 0)
      {
        display.print(Tmin,1);
        display.println("m");
      }
      else if (Tdisplay == 1)
      {
        display.print("1/");
        display.println(Tfr);
      }
      else if (Tdisplay == 2)
      {
        display.print(T,1);
        display.println("s");
      }
      else if (Tdisplay == 3)
      {
        display.println("RANGE!");
      }
    
    display.println(T,3);
    
    display.drawLine(73, 0, 73, 32, WHITE);
    
    display.setTextSize(1);
    display.setCursor(76,0);
    display.print("ISO");
    display.println(S[Sm],0);
    display.setCursor(76,11);
    display.print("EV=");
    Ev = log(pow(A[Am],2))/log(2) + log(1/T)/log(2);
    display.println(floor(Ev+0.5),1);
    display.setCursor(76,22);
    display.print(lux,1);
    display.println("Lx");
    
    display.display();
    display.clearDisplay();
}
