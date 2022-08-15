// AC Current Sensor without LCD By Solarduino 

// Note Summary
// Note :  Safety is very important when dealing with electricity. We take no responsibilities while you do it at your own risk.
// Note :  This AC Current Sensor Code is for ACS712 current module and Hall effect split core current transformer use.
// Note :  The value shown in Serial Monitor is refreshed every second and is the average value of 1000 sample readings.
// Note :  The current measured is the Root Mean Square (RMS) value.
// Note :  The analog value per sample is squared and accumulated for every 1000 samples before being averaged. The averaged value is then getting square-rooted.
// Note :  The auto calibration (currentOffset1) is using averaged analogRead value of 1000 samples.
// Note :  The auto calibration (currentOffset2) is using calculated RMS current value including currentOffset1 value for calibration.  
// Note :  The unit provides reasonable accuracy and may not be comparable with other expensive branded and commercial product.
// Note :  All credit shall be given to Solarduino.

/*/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/////////////*/

        /* 0- General */

        int decimalPrecision = 2;                   // decimal places for all values shown in LED Display & Serial Monitor


        /* 1- AC Current Measurement */

        int CurrentAnalogInputPin = A1;             // Which pin to measure Current Value (A0 is reserved for LCD Display Shield Button function)
        float mVperAmpValue = 185;                  // If using ACS712 current module : for 5A module key in 185, for 20A module key in 100, for 30A module key in 66
                                                    // If using "Hall-Effect" Current Transformer, key in value using this formula: mVperAmp = maximum voltage range (in milli volt) / current rating of CT
                                                    // For example, a 20A Hall-Effect Current Transformer rated at 20A, 2.5V +/- 0.625V, mVperAmp will be 625 mV / 20A = 31.25mV/A 
        float offsetSampleRead = 0;                 /* to read the value of a sample for offset purpose later */
        float currentSampleRead  = 0;               /* to read the value of a sample including currentOffset1 value*/
        float currentLastSample  = 0;               /* to count time for each sample. Technically 1 milli second 1 sample is taken */
        float currentSampleSum   = 0;               /* accumulation of sample readings */
        float currentSampleCount = 0;               /* to count number of sample. */
        float currentMean ;                         /* to calculate the average value from all samples, in analog values*/ 
        float RMSCurrentMean ;                      /* square roof of currentMean, in analog values */
        float adjustRMSCurrentMean ;                /* RMScurrentMean including currenOffset2, in analog values */    
        float FinalRMSCurrent ;                     /* the final RMS current reading*/


            /*1.1 Offset AC Current */
    
            float currentOffset1 = 0;                   // to Offset deviation and accuracy. Offset any fake current when no current operates. 
                                                        // Offset will automatically callibrate when SELECT Button on the LCD Display Shield is pressed.
                                                        // If you do not have LCD Display Shield, look into serial monitor to add or minus the value manually and key in here.
                                                        // 26 means add 26 to all analog value measured
            float currentOffset2 = 0;                   // to offset value due to calculation error from squared and square root.
      
 

void setup()                                              /*codes to run once */

{                                      

        /* 0- General */
        
        Serial.begin(9600);                               /* to display readings in Serial Monitor at 9600 baud rates */


}


void loop()                                               /*codes to run again and again */
{                                      

        /* 1- AC Current Measurement */
        
        if(millis() >= currentLastSample + 1)                                                                 /* every 1 milli second taking 1 reading */
          {
           offsetSampleRead = analogRead(CurrentAnalogInputPin)-512;                                          /* Read analog value. This is for offset purpose */ 
           currentSampleRead = analogRead(CurrentAnalogInputPin)-512 + currentOffset1;                        /* read the sample value including offset value*/
           currentSampleSum = currentSampleSum + sq(currentSampleRead) ;                                      /* accumulate total analog values for each sample readings*/
           
           currentSampleCount = currentSampleCount + 1;                                                       /* to count and move on to the next following count */  
           currentLastSample = millis();                                                                      /* to reset the time again so that next cycle can start again*/ 
          }
        
        if(currentSampleCount == 1000)                                                                        /* after 1000 count or 1000 milli seconds (1 second), do this following codes*/
          {
            currentMean = currentSampleSum/currentSampleCount;                                                /* average accumulated analog values*/
            RMSCurrentMean = sqrt(currentMean);                                                               /* square root of the average value*/
            adjustRMSCurrentMean = RMSCurrentMean + currentOffset2;                                           /* square root of the average value including offset value */
            FinalRMSCurrent = (((adjustRMSCurrentMean /1024) *5000) /mVperAmpValue);                          /* calculate the final RMS current*/
            Serial.print(" The Current RMS value is: ");
            Serial.print(FinalRMSCurrent,decimalPrecision);
            Serial.println(" A ");
            currentSampleSum =0;                                                                              /* to reset accumulate sample values for the next cycle */
            currentSampleCount=0;                                                                             /* to reset number of sample for the next cycle */
          }

    
}
