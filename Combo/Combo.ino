#include <SoftwareSerial.h>
#include <EEPROM.h>

#include "Types.h"
#include "Utils.h"
#include "MidiReader.h"
#include "StopDriver.h"

#define MIDI_BOARD_SWITCH_D2  2
#define MIDI_BOARD_SWITCH_D3  3
#define MIDI_BOARD_SWITCH_D4  4

#define FIRST_SWELL_COUPLER_STOP 31  /* in this firmware, stops start at 0; arbitrary large number past all stops & possible couplers */
#define FIRST_PEDAL_COUPLER_STOP 9   /* in this firmware, stops start at 0 */
#define FIRST_CHIOR_COUPLER_STOP 12  /* in this firmware, stops start at 0 */
#define FIRST_GREAT_COUPLER_STOP 9   /* in this firmware, stops start at 0 */

// RC constant of switches combined with ribbon cable capacitance isn't great; slow down our signal
#define RIBBON_CABLE_RC_DELAY_US 100

#define MIDI_BOARD_SWITCH_PROG MIDI_BOARD_SWITCH_D2  
#define MIDI_BOARD_SWITCH_RECALL MIDI_BOARD_SWITCH_D3
#define MIDI_BOARD_SWITCH_CLEAR MIDI_BOARD_SWITCH_D4

#define EEPROM_MEMORY_START 0x0000

#define PISTON_GT_OUTPUT_ROW_PIN  43 /* PL6 */
#define PISTON_CH_OUTPUT_ROW_PIN  42 /* PL4 */
#define PISTON_SW_OUTPUT_ROW_PIN  41 /* PL7 */
#define PISTON_PL_OUTPUT_ROW_PIN  40 /* PG1 */
#define PISTON_PR_OUTPUT_ROW_PIN  39 /* PG2 */

#define SET_PISTON_INPUT_PIN  33 /* PC4 */

#define FULL_ORGAN_OUTPUT_PIN 52

#define TOGGLE_DEBOUNCE_TIME_MS 300
#define RESTORE_DEBOUNCE_MS 200
#define STORE_DEBOUNCE_MS 300

#define CH_GT_STROBE_PIN 48
#define CH_GT_CLOCK_PIN 46
#define CH_GT_DATA_PIN 44
#define CH_GT_SKIP_BITS 12

#define SW_PD_STROBE_PIN 49
#define SW_PD_CLOCK_PIN 47
#define SW_PD_DATA_PIN 45
#define SW_PD_SKIP_BITS 12

#define LF 10
#define CR 13

#define GT_ROW 1
#define SW_ROW 2
#define CH_ROW 3
#define PL_ROW 4
#define PR_ROW 5

#define ROW_CODE_SHIFT 8

#define NO_KEY 0

#define PISTON_OFFSET_ERROR ((unsigned long) -1)
#define SW_1  ((SW_ROW << ROW_CODE_SHIFT) + 4) 
#define SW_2  ((SW_ROW << ROW_CODE_SHIFT) + 2) 
#define SW_3  ((SW_ROW << ROW_CODE_SHIFT) + 1) 
#define SW_4  ((SW_ROW << ROW_CODE_SHIFT) + 8) 

#define GT_GEN_1  ((GT_ROW << ROW_CODE_SHIFT) + 1)  
#define GT_GEN_2  ((GT_ROW << ROW_CODE_SHIFT) + 128) 
#define GT_GEN_3  ((GT_ROW << ROW_CODE_SHIFT) + 64) 
#define GT_GEN_4  ((GT_ROW << ROW_CODE_SHIFT) + 32)
#define GT_1      ((GT_ROW << ROW_CODE_SHIFT) + 16)
#define GT_2      ((GT_ROW << ROW_CODE_SHIFT) + 8)
#define GT_3      ((GT_ROW << ROW_CODE_SHIFT) + 4)
#define GT_4      ((GT_ROW << ROW_CODE_SHIFT) + 2)

#define CH_GENSET ((CH_ROW << ROW_CODE_SHIFT) + 1)   /* special; not in matrix */
#define CH_1      ((CH_ROW << ROW_CODE_SHIFT) + 2)
#define CH_2      ((CH_ROW << ROW_CODE_SHIFT) + 4)
#define CH_3      ((CH_ROW << ROW_CODE_SHIFT) + 8)
#define CH_4      ((CH_ROW << ROW_CODE_SHIFT) + 16)
#define CH_GENCAN ((CH_ROW << ROW_CODE_SHIFT) + 32)

#define PD_GEN_1          ((PL_ROW << ROW_CODE_SHIFT) + 1)
#define PD_GEN_2          ((PL_ROW << ROW_CODE_SHIFT) + 2)
#define PD_GEN_3          ((PL_ROW << ROW_CODE_SHIFT) + 4)
#define PD_GEN_4          ((PL_ROW << ROW_CODE_SHIFT) + 8)
#define PD_SW_TO_GT_CPL   ((PL_ROW << ROW_CODE_SHIFT) + 16)

#define PD_1             ((PR_ROW << ROW_CODE_SHIFT) + 2)
#define PD_2             ((PR_ROW << ROW_CODE_SHIFT) + 4)
#define PD_3             ((PR_ROW << ROW_CODE_SHIFT) + 8)
#define PD_4             ((PR_ROW << ROW_CODE_SHIFT) + 16)
#define PD_GT_TO_PD_CPL  ((PR_ROW << ROW_CODE_SHIFT) + 1)
#define PD_FULL_ORGAN    ((PR_ROW << ROW_CODE_SHIFT) + 32)

#define DIVISIONS 4
#define DIVISION_MEM_SIZE_BYTES sizeof(long)
#define GENERAL_MEM_SIZE_BYTES (DIVISION_MEM_SIZE_BYTES * DIVISIONS)
#define DIVISION_MEM_PISTON_COUNT 16
#define GENERAL_MEM_PISTON_COUNT 8
#define MEMORY_LEVEL_SIZE_BYTES ((DIVISION_MEM_SIZE_BYTES * DIVISION_MEM_PISTON_COUNT) + (GENERAL_MEM_SIZE_BYTES * GENERAL_MEM_PISTON_COUNT))
#define FIRST_GENERAL_MEM  (DIVISION_MEM_SIZE_BYTES * DIVISION_MEM_PISTON_COUNT)
     
enum Piston {pbNone,
             // divisions
             pbSW1, pbSW2, pbSW3, pbSW4, 
             pbGT1, pbGT2, pbGT3, pbGT4, 
             pbCH1, pbCH2, pbCH3, pbCH4,
             pbPD1, pbPD2, pbPD3, pbPD4, 
             // generals
             pbKGen1, pbKGen2, pbKGen3, pbKGen4, 
             pbPGen1, pbPGen2, pbPGen3, pbPGen4,
             // other
             pbToggleSWToGTCoupler, pbToggleGTToPDCoupler, pbToggleFullOrgan,
             pbGenCan};

enum Division {diNone, diOther, diSwell, diGreat, diChior, diPedal, diGeneral};

SoftwareSerial* debugSerial;
StopState* stopState;
StopDriver* driver_CH_GT;
StopDriver* driver_SW_PD;
MidiReader* midiReader;

char caCommandBuffer[50];
char* pCommandBufferNextChar;

enum TestMode {tmNormal, tmSequential, tmCycleAll, tmCustom};

TestMode eTestMode = tmNormal;  
bool bDebug = false;
bool m_bFullOrgan = false;

void resetCommandBuffer() {
  pCommandBufferNextChar = &caCommandBuffer[0];
  *pCommandBufferNextChar=0;
}

void setup() {
  debugSerial = &SoftwareSerial(DEBUG_RX_PORT, DEBUG_TX_PORT);
  
  debugSerial->begin(19200);
  debugSerial->println("Starting...");

  resetCommandBuffer();
  
  pinMode(HEART_BEAT_LED, OUTPUT);

  pinMode(MIDI_BOARD_SWITCH_PROG, INPUT_PULLUP);
  pinMode(MIDI_BOARD_SWITCH_RECALL, INPUT_PULLUP);
  pinMode(MIDI_BOARD_SWITCH_CLEAR, INPUT_PULLUP);

  pinMode(PISTON_GT_OUTPUT_ROW_PIN, OUTPUT);
  pinMode(PISTON_CH_OUTPUT_ROW_PIN, OUTPUT);
  pinMode(PISTON_SW_OUTPUT_ROW_PIN, OUTPUT);
  pinMode(PISTON_PL_OUTPUT_ROW_PIN, OUTPUT);
  pinMode(PISTON_PR_OUTPUT_ROW_PIN, OUTPUT);

  pinMode(SET_PISTON_INPUT_PIN, INPUT_PULLUP);

  pinMode(FULL_ORGAN_OUTPUT_PIN, OUTPUT);

  setFullOrgan(false);
  
  // lower 4 bits on port C are for the memory switch
  pinMode(37, INPUT_PULLUP);
  pinMode(36, INPUT_PULLUP);
  pinMode(35, INPUT_PULLUP);
  pinMode(34, INPUT_PULLUP);

  // port A is an input with pullup
  DDRA  = B00000000;
  PORTA = B11111111;
  
  stopState = new StopState();
  midiReader = new MidiReader(stopState);

  // 2nd division starts at pin 41 (64 pins if all used - 40 pins allocated) / 2 = 12 upper bits to skip on lower value provided last)
  // same offset for both boards.
  // the "41" mentioned below is a port #
  driver_SW_PD = new StopDriver(SW_PD_STROBE_PIN, SW_PD_CLOCK_PIN, SW_PD_DATA_PIN, SW_PD_SKIP_BITS, FIRST_SWELL_COUPLER_STOP, FIRST_PEDAL_COUPLER_STOP);
  driver_CH_GT = new StopDriver(CH_GT_STROBE_PIN, CH_GT_CLOCK_PIN, CH_GT_DATA_PIN, CH_GT_SKIP_BITS, FIRST_CHIOR_COUPLER_STOP, FIRST_GREAT_COUPLER_STOP);  
}


void handleCommands() { 
  while (debugSerial->available() > 0) {
    // get incoming byte:
    char c = debugSerial->read();

    // discard unused char
    if (c == 10)
      continue;

    if (c == 13) {
      char* pCommandBuffer = &caCommandBuffer[0];
      
      if (strcmp(pCommandBuffer, "Normal")==0) {
        eTestMode = tmNormal;
        debugSerial->println("Normal mode selected");
      } else if (strcmp(pCommandBuffer, "Custom")==0) {
        eTestMode = tmCustom;
        debugSerial->println("Custom test mode selected");
      } else if (strcmp(pCommandBuffer, "Seq")==0) {
        eTestMode = tmSequential;
        debugSerial->println("Sequential mode selected");
      } else if (strcmp(pCommandBuffer, "CycleAll")==0) {
        eTestMode = tmCycleAll;
        debugSerial->println("Cycle All mode selected");
      } else if (strcmp(pCommandBuffer, "DebugOn")==0) {
        bDebug = true;
        debugSerial->println("Debug is now on");
      } else if (strcmp(pCommandBuffer, "DebugOff")==0) {
        bDebug = false;
        debugSerial->println("Debug is now off");
      } else {        debugSerial->print("Unrecognized command:  ");  
        debugSerial->print("Unrecognized command:  ");  
        debugSerial->println(pCommandBuffer);  
      }

      resetCommandBuffer();
    } else {
      *pCommandBufferNextChar = c;
      pCommandBufferNextChar++;
      *pCommandBufferNextChar=0;
    }
  }
}


int readRow(int iRowValue) {
  delayMicroseconds(RIBBON_CABLE_RC_DELAY_US);
  
  byte iValue = PINA ^ 0xff;
  
  if (iValue>0)
    return (int) (iRowValue << ROW_CODE_SHIFT) + iValue;
    else
    return 0;
}


int readScanCode() {
  digitalWrite(PISTON_GT_OUTPUT_ROW_PIN, HIGH);  // unnecessary
  digitalWrite(PISTON_CH_OUTPUT_ROW_PIN, HIGH);
  digitalWrite(PISTON_SW_OUTPUT_ROW_PIN, HIGH);
  digitalWrite(PISTON_PL_OUTPUT_ROW_PIN, HIGH);
  digitalWrite(PISTON_PR_OUTPUT_ROW_PIN, HIGH);

  int iValue;
  
  digitalWrite(PISTON_GT_OUTPUT_ROW_PIN, LOW);
  iValue = readRow(GT_ROW);
  digitalWrite(PISTON_GT_OUTPUT_ROW_PIN, HIGH);
  if (iValue>0)
    return iValue;

  digitalWrite(PISTON_CH_OUTPUT_ROW_PIN, LOW);
  iValue = readRow(CH_ROW);
  digitalWrite(PISTON_CH_OUTPUT_ROW_PIN, HIGH);
  if (iValue>0)
    return iValue;

  digitalWrite(PISTON_SW_OUTPUT_ROW_PIN, LOW);
  iValue = readRow(SW_ROW);
  digitalWrite(PISTON_SW_OUTPUT_ROW_PIN, HIGH);
  if (iValue>0)
    return iValue;

  digitalWrite(PISTON_PL_OUTPUT_ROW_PIN, LOW);
  iValue = readRow(PL_ROW);
  digitalWrite(PISTON_PL_OUTPUT_ROW_PIN, HIGH);
  if (iValue>0)
    return iValue;

  digitalWrite(PISTON_PR_OUTPUT_ROW_PIN, LOW);
  iValue = readRow(PR_ROW);
  digitalWrite(PISTON_PR_OUTPUT_ROW_PIN, HIGH);
  if (iValue>0)
    return iValue;

  return NO_KEY;
}


Piston getPressedPiston() {
   int iScanCode = readScanCode();

//   debugSerial->print("RawCode:  " );
//   debugSerial->print(PINA, HEX);
//   debugSerial->print("    " );

//   debugSerial->print("ScanCode:  " );
//   debugSerial->println(iScanCode, HEX);

   switch (iScanCode) {
     case NO_KEY:
        return pbNone;
        
     case SW_1:
        return pbSW1;
        
     case SW_2:
        return pbSW2;
        
     case SW_3:
        return pbSW3;
        
     case SW_4:
        return pbSW4;

     case GT_1:
        return pbGT1;
        
     case GT_2:
        return pbGT2;
        
     case GT_3:
        return pbGT3;
        
     case GT_4:
        return pbGT4;
        
     case GT_GEN_1:
        return pbKGen1;
        
     case GT_GEN_2:
        return pbKGen2;
        
     case GT_GEN_3:
        return pbKGen3;
        
     case GT_GEN_4:
        return pbKGen4;
        
     case CH_1:
        return pbCH1;
        
     case CH_2:
        return pbCH2;
        
     case CH_3:
        return pbCH3;
        
     case CH_4:
        return pbCH4;

     case CH_GENCAN:
        return pbGenCan;
        
     case PD_GEN_1:
        return pbPGen1;
        
     case PD_GEN_2:
        return pbPGen2;
        
     case PD_GEN_3:
        return pbPGen3;
        
     case PD_GEN_4:
        return pbPGen4;
        
     case PD_SW_TO_GT_CPL:
        return pbToggleSWToGTCoupler;
        
     case PD_1:
        return pbPD1;
        
     case PD_2:
        return pbPD2;
        
     case PD_3:
        return pbPD3;
        
     case PD_4:
        return pbPD4;
        
     case PD_GT_TO_PD_CPL:
        return pbToggleGTToPDCoupler;

     case PD_FULL_ORGAN:
        return pbToggleFullOrgan;

     default:  return pbNone;
   }
}


Division getDivision(Piston piston) {
  switch (piston) {
    case pbNone:
      return diNone;
      
    case pbSW1 ... pbSW4:
      return diSwell;
      
    case pbGT1 ... pbGT4:
      return diGreat;
      
    case pbCH1 ... pbCH4:
      return diChior;
      
    case pbPD1 ... pbPD4:
      return diPedal;
      
    case pbKGen1 ... pbKGen4:
    case pbPGen1 ... pbPGen4:
      return diGeneral;
      
    default:
      return diOther;      
  }
}


void describeDivision(Division division) {
  switch (division) {
    case diNone:
      break;
      
    case diSwell:
      debugSerial->print("diSwell");
      break;
      
    case diGreat:
      debugSerial->print("diGreat");
      break;
      
    case diChior:
      debugSerial->print("diChior");
      break;
      
    case diPedal:
      debugSerial->print("diPedal");
      break;
      
    case diGeneral:
      debugSerial->print("diGeneral");
      break;
      
    case diOther:
      debugSerial->print("diOther");
      break;
      
    default: debugSerial->print("UNKNOWN DIVISION!");
  }
}


void describePiston(Piston piston) {
  switch (piston) {
    case pbNone:
      //debugSerial->println("pbNone");
      break;
      
    case pbSW1:
      debugSerial->println("pbSW1");
      break;
      
    case pbSW2:
      debugSerial->println("pbSW2");
      break;
      
    case pbSW3:
      debugSerial->println("pbSW3");
      break;
      
    case pbSW4:
      debugSerial->println("pbSW4");
      break;

      
    case pbGT1:
      debugSerial->println("pbGT1");
      break;
      
    case pbGT2:
      debugSerial->println("pbGT2");
      break;
      
    case pbGT3:
      debugSerial->println("pbGT3");
      break;
      
    case pbGT4:
      debugSerial->println("pbGT4");
      break;

      
    case pbCH1:
      debugSerial->println("pbCH1");
      break;
      
    case pbCH2:
      debugSerial->println("pbCH2");
      break;
      
    case pbCH3:
      debugSerial->println("pbCH3");
      break;
      
    case pbCH4:
      debugSerial->println("pbCH4");
      break;

      
    case pbPD1:
      debugSerial->println("pbPD1");
      break;
      
    case pbPD2:
      debugSerial->println("pbPD2");
      break;
      
    case pbPD3:
      debugSerial->println("pbPD3");
      break;
      
    case pbPD4:
      debugSerial->println("pbPD4");
      break;  

      
    case pbKGen1:
      debugSerial->println("pbKGen1");
      break;
      
    case pbKGen2:
      debugSerial->println("pbKGen2");
      break;
      
    case pbKGen3:
      debugSerial->println("pbKGen3");
      break;
      
    case pbKGen4:
      debugSerial->println("pbKGen4");
      break;
      

    case pbPGen1:
      debugSerial->println("pbPGen1");
      break;
      
    case pbPGen2:
      debugSerial->println("pbPGen2");
      break;
      
    case pbPGen3:
      debugSerial->println("pbPGen3");
      break;
      
    case pbPGen4:
      debugSerial->println("pbPGen4");
      break;
      

    case pbToggleSWToGTCoupler:
      debugSerial->println("pbToggleSWToGTCoupler");
      break;
      
    case pbToggleGTToPDCoupler:
      debugSerial->println("pbToggleGTToPDCoupler");
      break;
      
    case pbToggleFullOrgan:
      debugSerial->println("pbToggleFullOrgan");
      break;
      
    case pbGenCan:
      debugSerial->println("pbGenCan");
      break;
      
    default:
      debugSerial->println("OOPS...UNKNOWN KEY!");
      break;      
  }
}

int getPistonMemOffset(Piston piston) {
  switch (piston) {
     case pbSW1:return DIVISION_MEM_SIZE_BYTES * 0;
     case pbSW2:return DIVISION_MEM_SIZE_BYTES * 1;
     case pbSW3:return DIVISION_MEM_SIZE_BYTES * 2;
     case pbSW4:return DIVISION_MEM_SIZE_BYTES * 3;
     case pbGT1:return DIVISION_MEM_SIZE_BYTES * 4;
     case pbGT2:return DIVISION_MEM_SIZE_BYTES * 5;
     case pbGT3:return DIVISION_MEM_SIZE_BYTES * 6;
     case pbGT4:return DIVISION_MEM_SIZE_BYTES * 7;
     case pbCH1:return DIVISION_MEM_SIZE_BYTES * 8;
     case pbCH2:return DIVISION_MEM_SIZE_BYTES * 9;
     case pbCH3:return DIVISION_MEM_SIZE_BYTES * 10;
     case pbCH4:return DIVISION_MEM_SIZE_BYTES * 11;
     case pbPD1:return DIVISION_MEM_SIZE_BYTES * 12;
     case pbPD2:return DIVISION_MEM_SIZE_BYTES * 13;
     case pbPD3:return DIVISION_MEM_SIZE_BYTES * 14;
     case pbPD4:return DIVISION_MEM_SIZE_BYTES * 15;
     case pbKGen1:return FIRST_GENERAL_MEM + (GENERAL_MEM_SIZE_BYTES * 0);
     case pbKGen2:return FIRST_GENERAL_MEM + (GENERAL_MEM_SIZE_BYTES * 1);
     case pbKGen3:return FIRST_GENERAL_MEM + (GENERAL_MEM_SIZE_BYTES * 2);
     case pbKGen4:return FIRST_GENERAL_MEM + (GENERAL_MEM_SIZE_BYTES * 3);
     case pbPGen1:return FIRST_GENERAL_MEM + (GENERAL_MEM_SIZE_BYTES * 4);
     case pbPGen2:return FIRST_GENERAL_MEM + (GENERAL_MEM_SIZE_BYTES * 5);
     case pbPGen3:return FIRST_GENERAL_MEM + (GENERAL_MEM_SIZE_BYTES * 6);
     case pbPGen4:return FIRST_GENERAL_MEM + (GENERAL_MEM_SIZE_BYTES * 7);
     default:  return PISTON_OFFSET_ERROR;
   }
}


int getMemoryPos() {
  // note that switch is wired in a funny way such that 0 is last when it should be first:  1..11, 0
  return (PINC & 0x0f) ^ 0x0f;
}


int getMemOffset(Piston piston) {
  int iPistonMemOffset = getPistonMemOffset(piston);

  if (iPistonMemOffset<0)
    return PISTON_OFFSET_ERROR;
    else 
    return EEPROM_MEMORY_START + (MEMORY_LEVEL_SIZE_BYTES * getMemoryPos()) + iPistonMemOffset;
}


void storePiston(Piston piston) {
  unsigned long iPistonMemoryOffset = getMemOffset(piston);

  // invalid address
  if (iPistonMemoryOffset < 0) 
    return;
  
//  debugSerial->print("Storing to ");
//  debugSerial->print(iPistonMemoryOffset); 
//  debugSerial->print(" for ");
//  describePiston(piston);

  switch (piston) {
    case pbSW1 ... pbSW4:
//      debugSerial->print(" S:"); 
//      debugSerial->print(stopState->swell, HEX); 
      eeprom_write_dword((unsigned long*) iPistonMemoryOffset, stopState->swell);

//      debugSerial->print(" (P:"); 
//      debugSerial->print(stopState->pedal, HEX); 
//      debugSerial->print(")"); 
      break;
      
    case pbGT1 ... pbGT4:
//      debugSerial->print(" G:"); 
//      debugSerial->print(stopState->great, HEX); 
      eeprom_write_dword((unsigned long*) iPistonMemoryOffset, stopState->great);
      break;
      
    case pbCH1 ... pbCH4:
//      debugSerial->print(" C:"); 
//      debugSerial->print(stopState->chior, HEX); 
      eeprom_write_dword((unsigned long*) iPistonMemoryOffset, stopState->chior);
      break;

    case pbPD1 ... pbPD4:
//      debugSerial->print(" P:"); 
//      debugSerial->print(stopState->pedal, HEX); 
      eeprom_write_dword((unsigned long*) iPistonMemoryOffset, stopState->pedal);
      break;
      
    case pbKGen1 ... pbKGen4:
    case pbPGen1 ... pbPGen4:
//      debugSerial->print(" C:"); 
//      debugSerial->print(stopState->chior, HEX); 
      eeprom_write_dword((unsigned long*) iPistonMemoryOffset, stopState->chior);
      iPistonMemoryOffset+=DIVISION_MEM_SIZE_BYTES;
      
//      debugSerial->print(" G:"); 
//      debugSerial->print(stopState->great, HEX); 
      eeprom_write_dword((unsigned long*) iPistonMemoryOffset, stopState->great);
      iPistonMemoryOffset+=DIVISION_MEM_SIZE_BYTES;
      
//      debugSerial->print(" S:"); 
//      debugSerial->print(stopState->swell, HEX); 
      eeprom_write_dword((unsigned long*) iPistonMemoryOffset, stopState->swell);
      iPistonMemoryOffset+=DIVISION_MEM_SIZE_BYTES;
      
//      debugSerial->print(" P:"); 
//      debugSerial->print(stopState->pedal, HEX); 
      eeprom_write_dword((unsigned long*) iPistonMemoryOffset, stopState->pedal);
      break;
  }

//  debugSerial->print(" to ");
//  debugSerial->print(iPistonMemoryOffset); 
//  debugSerial->print(" for ");
//  describePiston(piston);

  // generous debounce
  stopDelay(RESTORE_DEBOUNCE_MS);
}


void restorePiston(Piston piston) {
  unsigned long iPistonMemoryOffset = getMemOffset(piston);

  // invalid address
  if (iPistonMemoryOffset < 0) 
    return;

//  debugSerial->println(); 
//  debugSerial->print("Reading from:  "); 
//  debugSerial->println(iPistonMemoryOffset); 
//  debugSerial->print("Piston pressed:  "); 
//  describePiston(piston);
   
  long iRestoreDivValue = eeprom_read_dword((unsigned long*) iPistonMemoryOffset);

//  debugSerial->print("Restoring value ");

  midiReader->readMessages();   

  switch (piston) {
    case pbSW1 ... pbSW4:
//      debugSerial->print(" S:"); 
//      debugSerial->print(iRestoreDivValue, HEX); 
//      debugSerial->print(" (P:"); 
//      debugSerial->print(stopState->pedal, HEX); 
//      debugSerial->print(")"); 
      driver_SW_PD->send(iRestoreDivValue, stopState->swell,   // SW
                         stopState->pedal, stopState->pedal,   // PD
                         false);  // rule:  divisionals exclude couplers
      break;
      
    case pbGT1 ... pbGT4:
//      debugSerial->print(" G:"); 
//      debugSerial->print(iRestoreDivValue, HEX); 
      driver_CH_GT->send(stopState->chior, stopState->chior,   // CH
                         iRestoreDivValue, stopState->great,   // GT
                         false);  // rule:  divisionals exclude couplers
      break;
      
    case pbCH1 ... pbCH4:
//      debugSerial->print(" C:"); 
//      debugSerial->print(iRestoreDivValue, HEX); 
      driver_CH_GT->send(iRestoreDivValue, stopState->chior,   // CH
                         stopState->great, stopState->great,   // GT
                         false);  // rule:  divisionals exclude couplers
      break;

    case pbPD1 ... pbPD4:
//      debugSerial->print(" P:"); 
//      debugSerial->print(iRestoreDivValue, HEX); 
      driver_SW_PD->send(stopState->swell, stopState->swell,   // SW
                         iRestoreDivValue, stopState->pedal,   // PD
                         false);  // rule:  divisionals exclude couplers
      break;
      
    case pbKGen1 ... pbKGen4:
    case pbPGen1 ... pbPGen4:
      long iChior = eeprom_read_dword((unsigned long*) iPistonMemoryOffset);
//      debugSerial->print(" C:"); 
//      debugSerial->print(iChior, HEX); 
      iPistonMemoryOffset+=DIVISION_MEM_SIZE_BYTES;

      long iGreat = eeprom_read_dword((unsigned long*) iPistonMemoryOffset);
//      debugSerial->print(" G:"); 
//      debugSerial->print(iGreat, HEX); 
      iPistonMemoryOffset+=DIVISION_MEM_SIZE_BYTES;

      driver_CH_GT->send(iChior, stopState->chior,   // CH
                         iGreat, stopState->great,   // GT
                         true);  // rule:  generals include couplers

      long iSwell = eeprom_read_dword((unsigned long*) iPistonMemoryOffset);
//      debugSerial->print(" S:"); 
//      debugSerial->print(iSwell, HEX); 
      iPistonMemoryOffset+=DIVISION_MEM_SIZE_BYTES;

      long iPedal = eeprom_read_dword((unsigned long*) iPistonMemoryOffset);
//      debugSerial->print(" P:"); 
//      debugSerial->print(iPedal, HEX); 
      
      driver_SW_PD->send(iSwell, stopState->swell,   // SW
                         iPedal, stopState->pedal,   // PD
                         true);  // rule:  generals include couplers
      break;
  }

//  debugSerial->print(" from ");
//  debugSerial->print(iPistonMemoryOffset); 
//  debugSerial->print(" for ");
//  describePiston(piston);
  
  stopDelay(STOP_DRIVE_TIME_MS);
  
  driver_CH_GT->setAllOff();
  driver_SW_PD->setAllOff();

  stopDelay(RESTORE_DEBOUNCE_MS);
}


bool getStorePistonState() {
  digitalWrite(PISTON_CH_OUTPUT_ROW_PIN, LOW);
  delayMicroseconds(RIBBON_CABLE_RC_DELAY_US);
  
  bool bSet = !digitalRead(SET_PISTON_INPUT_PIN);
  digitalWrite(PISTON_CH_OUTPUT_ROW_PIN, HIGH);

  return bSet;
}


void setFullOrgan(bool bFullOrgan) {
  m_bFullOrgan = bFullOrgan;
  digitalWrite(FULL_ORGAN_OUTPUT_PIN, m_bFullOrgan);
  
  if (m_bFullOrgan)
    debugSerial->println("fo");
    else
    debugSerial->println("/fo");    
}


void doPiston(Piston piston) {
  if (getStorePistonState()) 
    storePiston(piston);
    else
    restorePiston(piston);

  switch (piston) {
    case pbToggleSWToGTCoupler:
//      debugSerial->println("SW->GT"); 
      driver_CH_GT->send(stopState->chior, stopState->chior,   // CH
                         stopState->great ^ stopValue(10), stopState->great,   // GT
                         true);

      stopDelay(STOP_DRIVE_TIME_MS);
      
      driver_CH_GT->setAllOff();
      driver_SW_PD->setAllOff();
      break;
      
    case pbToggleGTToPDCoupler:
      driver_SW_PD->send(stopState->swell, stopState->swell,   // SW
                         stopState->pedal ^ stopValue(9), stopState->pedal,   // PD
                         true);

      stopDelay(STOP_DRIVE_TIME_MS);
      
      driver_CH_GT->setAllOff();
      driver_SW_PD->setAllOff();
      break;
      
    case pbToggleFullOrgan:
      break;
      
    case pbGenCan:
      driver_CH_GT->send(0, stopState->chior,   // CH
                         0, stopState->great,   // GT
                         true);

      driver_SW_PD->send(0, stopState->swell,   // SW
                         0, stopState->pedal,   // PD
                         true);

      stopDelay(STOP_DRIVE_TIME_MS);
      
      driver_CH_GT->setAllOff();
      driver_SW_PD->setAllOff();
      break;
  }
}
    

void normalRun() {
  midiReader->readMessages();

  Piston piston = getPressedPiston();

  if (piston != pbNone) {
    //debugSerial->print("Piston pressed:  "); 
    //describePiston(piston);
    // debugSerial->println(piston);
    
//    Division division = getDivision(piston);
//    describeDivision(division);
//    debugSerial->print("    ");
//    describePiston(piston);

    doPiston(piston);
  }
}  // normalRun


void stopDelay(int iDelayMS) {
  unsigned long iStart = millis();
  unsigned long iEnd = iStart + iDelayMS;
  while (millis() < iEnd) {
    midiReader->readMessages();   
  }
}


void stateTest() {
  driver_CH_GT->send((unsigned long) 0xffffffff, 0,   // CH
                     (unsigned long) 0xffffffff, 0,   // GT
                     true);
  driver_SW_PD->send((unsigned long) 0xffffffff, 0,   // SW
                     (unsigned long) 0xffffffff, 0,   // PD
                     true);

  stopDelay(STOP_DRIVE_TIME_MS);

  driver_CH_GT->setAllOff();
  driver_SW_PD->setAllOff();

  midiReader->readMessages();
  stopState->debug_ShowAllStopStates("T0 activate");
                       
  stopDelay(500);

  midiReader->readMessages();
  stopState->debug_ShowAllStopStates("T100");
                       
//  stopDelay(500);
//
//  midiReader->readMessages();
//  stopState->debug_ShowAllStopStates("T200");
                       
  stopDelay(500);

  //debugSerial->print("PART OFF:  ");
  driver_CH_GT->send(0, 0xffffffff,   // CH
                     0, 0xffffffff,   // GT
                     true);
  driver_SW_PD->send(0, 0xffffffff,   // SW
                     0, 0xffffffff,   // PD
                     true);
                     
  stopDelay(STOP_DRIVE_TIME_MS);

  driver_CH_GT->setAllOff();
  driver_SW_PD->setAllOff();
      
  midiReader->readMessages();
  stopState->debug_ShowAllStopStates("T0 deactivate");
                       
  stopDelay(500);

  midiReader->readMessages();
  stopState->debug_ShowAllStopStates("T100");
                       
//  stopDelay(500);
//
//  midiReader->readMessages();
//  stopState->debug_ShowAllStopStates("T200");
}


void loop() {
  stateTest();
  
  while(1) {
 //   int iStop;

    driver_CH_GT->setAllOff();
    driver_SW_PD->setAllOff();

    handleCommands();

    switch (eTestMode) {
  
      case tmNormal:
        normalRun();
        break;  
  
      case tmSequential:
        int iStop;
        
        iStop=1;
        
        // could use MAX_DIV_STOPS instead but we don't have that many stops
        for (int i=0; i<17; i++) 
        {
          driver_CH_GT->send(iStop, 0,    // CH
                             iStop, 0,    // GT
                             true);
          driver_SW_PD->send(iStop, 0,    // SW
                             iStop, 0,    // PD
                             true);

          stopDelay(STOP_DRIVE_TIME_MS);
          
          driver_CH_GT->setAllOff();
          driver_SW_PD->setAllOff();
          
          iStop=iStop<<1;
          stopDelay(1000);
        }
        break;

      case tmCustom:
        stopDelay(1000);
      
        debugSerial->print("PART OFF:  ");
        driver_CH_GT->send(0, 0xffffffff,   // CH
                           0, 0xffffffff,   // GT
                           true);
        driver_SW_PD->send(0, 0xffffffff,   // SW
                           0, 0xffffffff,   // PD
                           true);
                           
        stopDelay(STOP_DRIVE_TIME_MS);
      
        driver_CH_GT->setAllOff();
        driver_SW_PD->setAllOff();
      
        debugSerial->println();

        break;
        
      case tmCycleAll: 
        //debugSerial->print("PART ON:   ");
        //2048, 1
        driver_CH_GT->send((unsigned long) 0xffffffff, 0,   // CH
                           (unsigned long) 0xffffffff, 0,   // GT
                           true);
        driver_SW_PD->send((unsigned long) 0xffffffff, 0,   // SW
                           (unsigned long) 0xffffffff, 0,   // PD
                           true);
      
        stopDelay(STOP_DRIVE_TIME_MS);
      
        driver_CH_GT->setAllOff();
        driver_SW_PD->setAllOff();

        debugSerial->println();
                             
        stopDelay(1000);
      
        //debugSerial->print("PART OFF:  ");
        driver_CH_GT->send(0, 0xffffffff,   // CH
                           0, 0xffffffff,   // GT
                           true);
        driver_SW_PD->send(0, 0xffffffff,   // SW
                           0, 0xffffffff,   // PD
                           true);
                           
        stopDelay(STOP_DRIVE_TIME_MS);
      
        driver_CH_GT->setAllOff();
        driver_SW_PD->setAllOff();
      
        debugSerial->println();

        stopDelay(1000);
        break;
      }
  
    digitalWrite(HEART_BEAT_LED, (millis() % 500) > 250);   
  }
}
