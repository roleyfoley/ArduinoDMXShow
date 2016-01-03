// Button Based Lighting Controller 
// Contains different sequences to control a series of DMX based lights
// Hardware:
// * Toggle Buttons - 1 Per Sequence
// * Conceptinetics DMX Shield 
// Sequences: 
// -- Sequence 1 - All On
// -- Sequence 2 - All Flash 
// -- Sequence 3 - Round the World - Cross fade run through list of lights. Next light starts half way through previous lights up down cylce. 
//                  Uses 2 arrays, loops through the arrays to select lights
// -- Sequence 4 - Array On - Specifiy a list of DMX Channel numbers and turn them on
// Written by: Michael 

// DMX Shield recieves signal via Serial - You cannot use serial output with this shield
// -- Usage - 255 = Full
// dmx_master.setChannelRange ( begin_channel, end_channel, <0-255> );
// dmx_master.setChannelValue ( channel, <0-255> );

// DMX Master setup using Conceptinetics Library 
#include <Conceptinetics.h>
const int DMX_MASTER_CHANNELS  = 55;
const int RXEN_PIN = 2;

DMX_Master dmx_master ( DMX_MASTER_CHANNELS , RXEN_PIN );

// ***** Behaviour Constants
// Button Control
const int ButtonAPin = 8;
const int ButtonBPin = 9;
const int ButtonCPin = 10;
const int ButtonDPin = 11;

// Sequence Specific
// -- Sequnce 1 
const int Seq1State = 255; 

// -- Sequence 2 
const int Seq2OnState = 255;
const int Seq2OffState = 0;
const int Seq2Delay = 250;

// -- Sequence 3
// Time for Single light up/down = 255/Seq3Incramet * Delay
// e.g. 255/10 * 100 = 2550 Milliseconds = 2.55 Seconds
const int Seq3Delay = 5;
const int Seq3Incrament = 5;
int Seq3SecDelay = 255 / Seq3Incrament / 2 * Seq3Delay;

const int Seq3UPChanCount = 23;
const int Seq3UPChannels[] = {13, 1, 3, 5, 6, 7, 8, 9, 11, 14, 16, 18, 20, 22, 24, 27, 53, 51, 49, 47, 46, 45, 44};
const int Seq3DOWNChanCount = 22;
const int Seq3DOWNChannels[] = {45, 46, 47, 48, 50, 52, 54, 26, 25, 23, 21, 19, 17, 15, 12, 10, 8, 7, 6, 5, 4, 2}; 

// -- Sequence 4
int Seq4ChannelCount = 17;
int Seq4Channels[] = {10, 12, 15, 17, 19, 21, 23, 25, 26, 54, 52, 50, 48, 48, 47, 46, 45, 44 };
int Seq4State = 150;


// ** Running Variables
// Time Control
unsigned long previousMillis = 0;

// Button Selection
unsigned int ButtonPinSelection = 4;

// Sequence Specific Variables
// -- Sequence 2
int Seq2State = 0;

// -- Sequence 3
unsigned long Seq3PriMillis = 0;
unsigned long Seq3SecMillis = 0;
unsigned long Seq3GoMillis = 0;
String Seq3Run = "STOP";
String Seq3Direction = "UP";
int Seq3PriCount = 0;
int Seq3SecCount = 1;
int Seq3Pri = Seq3UPChannels[Seq3PriCount]; 
int Seq3Sec = Seq3UPChannels[Seq3SecCount]; 
int Seq3PriState = 0;
int Seq3SecState = 0;
String Seq3PriDirection = "UP";
String Seq3SecDirection = "UP";

// -- Sequence 4
int Seq4Count = 0;


void setup() {
  // Button Setup
  pinMode(ButtonAPin, INPUT_PULLUP);
  pinMode(ButtonBPin, INPUT_PULLUP);
  pinMode(ButtonCPin, INPUT_PULLUP);
  pinMode(ButtonDPin, INPUT_PULLUP);

  // Enable the DMX sheild as a master and turn all outputs off
  dmx_master.enable ();
  dmx_master.setChannelRange ( 1, DMX_MASTER_CHANNELS, 0 );
}


void loop() {

  // Set Current Time
  unsigned long currentMillis = millis();

  // Get Button Output and assign a Sequence
  if (digitalRead(ButtonAPin) == LOW)
  {
    delay(250);
    if (ButtonPinSelection == 1)
    {
     OffButton();   
    }
    else {
      ButtonPinSelection = 1; 
    }
  }
  if (digitalRead(ButtonBPin) == LOW)
  {
    delay(250);
    if (ButtonPinSelection == 2)
    {
      OffButton();
    }
    else {
      ButtonPinSelection = 2;
      Seq2State = 0;
    }
  }
  if (digitalRead(ButtonCPin) == LOW)
  {
    delay(250);
    if (ButtonPinSelection == 3)
    {
      OffButton();
    }
    else {
      ButtonPinSelection = 3;
      Seq3GoMillis = currentMillis;
      Seq3Run = "STOP";
      Seq3PriCount = 0;
      Seq3SecCount = 1;
      Seq3Pri = Seq3UPChannels[Seq3PriCount]; 
      Seq3Sec = Seq3UPChannels[Seq3SecCount];
      Seq3PriState = 0;
      Seq3SecState = 0;
      Seq3PriDirection = "UP";
      Seq3SecDirection = "UP";
      Seq3Direction = "UP";
    }
  }
  if (digitalRead(ButtonDPin) == LOW)
  {
    delay(250);
    if (ButtonPinSelection == 4)
    {
      OffButton();
    }
    else {
      ButtonPinSelection = 4;
    }
  }
  
  // Lighting Sequences
  switch (ButtonPinSelection) {
    case 1:
      // Sequence 1 - All On
      dmx_master.setChannelRange ( 1, DMX_MASTER_CHANNELS, Seq1State );
      break;

    case 2:
      // Sequence 2 - Flash/Strobe All - Delay set in variables at top
      if (currentMillis - previousMillis >= Seq2Delay) {
        previousMillis = currentMillis;

        if (Seq2State == Seq2OffState) {
          Seq2State = Seq2OnState;
        } else {
          Seq2State = Seq2OffState;
        }
        dmx_master.setChannelRange ( 1, DMX_MASTER_CHANNELS, Seq2State );
      }
      break;

    case 3:
      // Sequence 3 - Round the world
      // Delay action without pausing script
      if ((currentMillis - Seq3PriMillis) >= Seq3Delay) {
        Seq3PriMillis = currentMillis;

        // Change Direction on last channel
        if ( Seq3PriCount == Seq3UPChanCount && Seq3Direction == "UP") {
          Seq3Direction = "DOWN";
          Seq3PriCount = 0;
        }

        if (Seq3PriCount == Seq3DOWNChanCount && Seq3Direction == "DOWN") {
          Seq3Direction = "UP";
          Seq3PriCount = 0;
        }

        // Start Control of Primary Light
        // Add the incrament value to the current state
        if ( Seq3PriDirection == "UP") {
          Seq3PriState = Seq3PriState + Seq3Incrament;
        }

        // Subtract the incrament value from the current state 
        if ( Seq3PriDirection == "DOWN") {
          Seq3PriState = Seq3PriState - Seq3Incrament;
        }

        // Don't let state get out of range
        if (Seq3PriState > 255) {
          Seq3PriState = 255;
        }
        if (Seq3PriState < 0 ) {
          Seq3PriState = 0;
        }

        if (Seq3PriState == 255 && Seq3PriDirection == "UP") {
          Seq3PriDirection = "DOWN";
        }

        // If both of these match the light has done the full run so we can start on the next one       
        if (Seq3PriState == 0 && Seq3PriDirection == "DOWN") {
          dmx_master.setChannelValue ( Seq3Pri, Seq3PriState );
          Seq3PriDirection = "UP";
          Seq3PriCount = Seq3PriCount + 2;
          
          if ( Seq3Direction == "UP") {
            Seq3Pri = Seq3UPChannels[Seq3PriCount];           
          }
          
          if ( Seq3Direction == "DOWN") {
            Seq3Pri = Seq3DOWNChannels[Seq3PriCount];           
          }
          
        }
        dmx_master.setChannelValue ( Seq3Pri, Seq3PriState );

        
      }

      // Start Control of Secondary Light
      if ((currentMillis - Seq3GoMillis) >= Seq3SecDelay && Seq3Run == "STOP") {
        Seq3GoMillis = currentMillis;
        Seq3Run = "GO";
      }

      if (Seq3Run == "GO" && (currentMillis - Seq3SecMillis) >= Seq3Delay ) {
        Seq3SecMillis = currentMillis;

        // Change Direction on last channel
        if ( Seq3SecCount >= Seq3UPChanCount && Seq3Direction == "UP") {
          Seq3Direction = "DOWN";
          Seq3SecCount = 0;
        }

        if (Seq3SecCount >= Seq3DOWNChanCount && Seq3Direction == "DOWN") {
          Seq3Direction = "UP";
          Seq3SecCount = 0;
        }

        // Start Control of Secondary Light
        // Add the incrament value to the current state
        if ( Seq3SecDirection == "UP") {
          Seq3SecState = Seq3SecState + Seq3Incrament;
        }

        // Subtract the incrament value from the current state 
        if ( Seq3SecDirection == "DOWN") {
          Seq3SecState = Seq3SecState - Seq3Incrament;
        }

        // Don't let state get out of range
        if (Seq3SecState > 255) {
          Seq3SecState = 255;
        }
        if (Seq3SecState < 0 ) {
          Seq3SecState = 0;
        }

        if (Seq3SecState == 255 && Seq3SecDirection == "UP") {
          Seq3SecDirection = "DOWN";
        }

        // If both of these match the light has done the full run so we can start on the next one       
        if (Seq3SecState == 0 && Seq3SecDirection == "DOWN") {
          dmx_master.setChannelValue ( Seq3Sec, Seq3SecState );
          Seq3SecDirection = "UP";
          
          Seq3SecCount = Seq3SecCount + 2;
          
          if ( Seq3Direction == "UP") {
            Seq3Sec = Seq3UPChannels[Seq3SecCount];           
          }
          
          if ( Seq3Direction == "DOWN") {
            Seq3Sec = Seq3DOWNChannels[Seq3SecCount];           
          }
          
        }
        dmx_master.setChannelValue ( Seq3Sec, Seq3SecState );
         
      }

      break;

    case 4:
      // Sequence 4 - Parking State
      for ( int Seq4Count = 0; Seq4Count < (Seq4ChannelCount + 1); Seq4Count++) {
        dmx_master.setChannelValue ( Seq4Channels[Seq4Count] , Seq4State );
      }

      break;

    default:
      // Do nothing

      break;
  }

}

void OffButton() {
  // This function defaults variables back to what they should be on a button press
  dmx_master.setChannelRange ( 1, DMX_MASTER_CHANNELS, 0 );
  ButtonPinSelection = 0;
  previousMillis = 0;
}

