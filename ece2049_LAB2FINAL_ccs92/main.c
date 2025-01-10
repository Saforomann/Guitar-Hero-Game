/************** ECE2049 DEMO CODE ******************/
/**************  13 March 2019   ******************/
/***************************************************/

#include <msp430.h>

/* Peripherals.c and .h are where the functions that implement
 * the LEDs and keypad, etc are. It is often useful to organize
 * your code by putting like functions together in files.
 * You include the header associated with that file(s)
 * into the main file of your project. */
#include "peripherals.h"
#include <stdlib.h>

void song1(char *notes, int *duration);
void SmokeOnTheWater(char *notes, int *duration);
unsigned char returnStateofButtons(void);
void configUserLEDs(unsigned char inbits);
void configLabBoardButtons(void);
void runtimerA2(void);
void stoptimerA2(int reset);

// Declare globals here
typedef enum {WELCOME,COUNTDOWN,PLAYSONG,INPUT,STOP,LOSER,WINNER} GAME_STATE;


#define QUARTER 128//64            //1     these are the ratios, if a QUARTER note is 96, then a HALF note will be 96*2 = 192
#define HALF 256//128              //2
#define QUARTERDOT 192//96         //1.5
#define HALFDOT 384//192           //3
#define EIGTH 64//32              //0.5
#define SIXTINTH 32//16           //0.25
#define FULL 512//256              //4


typedef enum {
    Stop,
    F0,
    Ab,
    A ,
    Bb,
    B,
    Cc,
    Cs,
    D,
    Eb,
    E,
    F,
    Fs,
    G,
    Ab2,
    A2,
    G0,
    E0,
    D0,
    C0,
    C0s,
    F0s,
    D0s
} NOTES_PITCH;

int count = 0;

// Main
void main(void)

          {
    // Define some local variables
    GAME_STATE state = WELCOME;
    unsigned char currKey = 0;
    unsigned char currState = 0;
    unsigned char correctInput = 0x00;
    unsigned char incorrectInput = 0x00;
    char numError = 0x00;
    int x = 0;
    int y = 1;
    int flagCountdown = 0;
    char currPitch = 0x00;
    char notes[40];
    int duration[40];


    WDTCTL = WDTPW | WDTHOLD;    // Stop watchdog timer. Always need to stop this!!
                                 // You can then configure it properly, if desired

    initLeds();                  //configure peripherals
    configDisplay();
    configKeypad();
    configLabBoardButtons();
    configUserLEDs(0x00);
    _BIS_SR(GIE);               // global interrupt enable

    Graphics_clearDisplay(&g_sContext);     //clear display

    while(1){
        currKey = getKey();
                                              // reset if # key is pressed at any point
        if (currKey == '#') {
            state = WELCOME;
            currKey = 0;
            BuzzerOff();
            setLeds(0);
            configUserLEDs(0x00);
            x = 0;
            y = 1;
            Graphics_clearDisplay(&g_sContext);
            numError = 0x00;
        }

        switch(state){

        case WELCOME:

                       Graphics_drawStringCentered(&g_sContext, "MSP430 Hero", AUTO_STRING_LENGTH, 50, 30, TRANSPARENT_TEXT);          //welcome screen on LCD
                       Graphics_drawStringCentered(&g_sContext, "Sam, Cos y Fer", AUTO_STRING_LENGTH, 48, 40, TRANSPARENT_TEXT);
                       Graphics_drawStringCentered(&g_sContext, "* to start", AUTO_STRING_LENGTH, 50, 70, TRANSPARENT_TEXT);
                       Graphics_drawStringCentered(&g_sContext, "# to reset", AUTO_STRING_LENGTH, 50, 80, TRANSPARENT_TEXT);
                       Graphics_flushBuffer(&g_sContext);

                        // Welcome screen code remains the same as last project.
                        // Change to initiate countdown directly after '*' is pressed.
                        if (currKey == '*')
                        {
                            // Load the song into notes and duration arrays.
                            SmokeOnTheWater(notes, duration); // Initialize Smoke on the Water
                            //songIndicator = 0x01; // Not strictly necessary unless used for debug or display
                            state = COUNTDOWN;
                            runtimerA2(); // Start timer for countdown
                            Graphics_clearDisplay(&g_sContext);
                            currKey = 0; // Reset key to avoid repeated processing
                        }
                        break;


        case COUNTDOWN:

            if ((count >= (flagCountdown + 200)) && (count <= (flagCountdown + 220)) || (count >= (flagCountdown + 400)) && (count <= (flagCountdown + 420)) || (count >= (flagCountdown + 600)) && (count <= (flagCountdown + 620))){  //if the count reaches the start of 2 or 1
                Graphics_clearDisplay(&g_sContext);                     //clear the previous number from the screen
                configUserLEDs(0xD8);
            }

            if (count < (flagCountdown + 200)) {            //write 3 if count is between 20 and 30
                Graphics_drawStringCentered(&g_sContext, "3", AUTO_STRING_LENGTH, 50, 50, TRANSPARENT_TEXT);
                Graphics_flushBuffer(&g_sContext);
                configUserLEDs(BIT0);


            } else if ((count > (flagCountdown + 220)) && (count < (flagCountdown + 400))) {     //write 2 if count is between 10 and 20
                Graphics_drawStringCentered(&g_sContext, "2", AUTO_STRING_LENGTH, 50, 50, TRANSPARENT_TEXT);
                Graphics_flushBuffer(&g_sContext);
                configUserLEDs(BIT1);



            } else if ((count > (flagCountdown + 420)) && (count < (flagCountdown + 600))) {      //write 1 if count is between 0 and 10
                Graphics_drawStringCentered(&g_sContext, "1", AUTO_STRING_LENGTH, 50, 50, TRANSPARENT_TEXT);
                Graphics_flushBuffer(&g_sContext);
                configUserLEDs(BIT0);


            } else if ((count > (flagCountdown + 620)) && (count < (flagCountdown + 720))){
                Graphics_drawStringCentered(&g_sContext, "GO!", AUTO_STRING_LENGTH, 50, 50, TRANSPARENT_TEXT);
                Graphics_flushBuffer(&g_sContext);
                configUserLEDs(BIT0 | BIT1);

            } else if (count > (flagCountdown + 720)){                         //once the count reaches 0, move onto the next state (PLAY)
                state = PLAYSONG;
                configUserLEDs(0x00);
                Graphics_clearDisplay(&g_sContext);
                Graphics_drawStringCentered(&g_sContext, "Click", AUTO_STRING_LENGTH, 50, 40, TRANSPARENT_TEXT);
                Graphics_drawStringCentered(&g_sContext, "those", AUTO_STRING_LENGTH, 50, 50, TRANSPARENT_TEXT);
                Graphics_drawStringCentered(&g_sContext, "Notes", AUTO_STRING_LENGTH, 50, 60, TRANSPARENT_TEXT);
                Graphics_drawStringCentered(&g_sContext, "!!!", AUTO_STRING_LENGTH, 50, 70, TRANSPARENT_TEXT);

                Graphics_flushBuffer(&g_sContext);
                flagCountdown = count;                                    //
            }

            break;


        case PLAYSONG:

            currPitch = notes[x];                           //hold the current note in a variable (will change as we iterate through all the notes in the notes array)

            if (count < (duration[x] + flagCountdown)) {
                    // Set LEDs according to the pitch, specific to "Smoke on the Water"
                    if ((currPitch == G)||(currPitch == D0)) {
                        setLeds(8); // Example: Set specific LEDs for these notes
                    } else if ((currPitch == Bb)||(currPitch == F0)) {
                        setLeds(4); // Different LED for Gb note
                    }else if ((currPitch == Cc)||(currPitch == G0)) {
                        setLeds(2); // Different LED for Gb note
                    }else if ((currPitch == Cs)||(currPitch == Ab2)) {
                        setLeds(1); // Different LED for Gb note
                    }else if (currPitch == 0x00) { // If a 0 is detected, that means the song is over
                        state = WINNER; // Move to winner state
                        Graphics_clearDisplay(&g_sContext);
                        currKey = 0;
                        break;
                    }

            if (numError > 7) {
                                y = 2;
                            }

                // period = 1 / frequency
                // ticks = (period / resolution) * max_count
                // ticks is what we pass to the buzzer
                switch(currPitch) {
                    case F0: // Assuming F0 is the low F
                        BuzzerOn(349); // Frequency = 349 Hz
                        break;
                    case Ab: // Assuming Ab is A flat
                        BuzzerOn(415); // Frequency = 415 Hz
                        break;
                    case A: // Assuming A is A
                        BuzzerOn(440); // Frequency = 440 Hz
                        break;
                    case Bb: // Assuming Bb is B flat
                        BuzzerOn(466); // Frequency = 466 Hz
                        break;
                    case B: // Assuming B is B
                        BuzzerOn(494); // Frequency = 494 Hz
                        break;
                    case Cc: // Assuming Cc is middle C
                        BuzzerOn(523); // Frequency = 523 Hz
                        break;
                    case Cs: // Assuming Cs is C sharp
                        BuzzerOn(554); // Frequency = 554 Hz
                        break;
                    case D: // Assuming D is D
                        BuzzerOn(587); // Frequency = 587 Hz
                        break;
                    case Eb: // Assuming Eb is E flat
                        BuzzerOn(622); // Frequency = 622 Hz
                        break;
                    case E: // Assuming E is E
                        BuzzerOn(659); // Frequency = 659 Hz
                        break;
                    case F: // Assuming F is F
                        BuzzerOn(698); // Frequency = 698 Hz
                        break;
                    case Fs: // Assuming Fs is F sharp
                        BuzzerOn(740); // Frequency = 740 Hz
                        break;
                    case G: // Assuming G is G
                        BuzzerOn(784); // Frequency = 784 Hz
                        break;
                    case A2: // Assuming A2 is A high (one more octave than A)
                        BuzzerOn(880); // Frequency = 880 Hz
                        break;
                    case Ab2: // A flat one octave higher
                         BuzzerOn(830); // Approximated to the nearest whole number for simplicity
                         break;
                    case G0: // Assuming G0 is G low (one less octave than G)
                        BuzzerOn(392); // Frequency = 392 Hz
                        break;
                    case E0: // Assuming E0 is E low (one less octave than E)
                        BuzzerOn(330); // Frequency = 329.63 Hz, rounded to 330 for simplicity
                        break;
                    case D0: // Assuming D0 is D low (one less octave than D)
                        BuzzerOn(294); // Frequency = 293.66 Hz, rounded to 294 for simplicity
                        break;
                    case C0: // Assuming C0 is C low (one less octave than C)
                        BuzzerOn(262); // Frequency = 261.63 Hz, rounded to 262 for simplicity
                        break;
                    case C0s: // Assuming C0s is C sharp low (one less octave than C sharp)
                        BuzzerOn(277); // Frequency = 277.18 Hz
                        break;
                    case F0s: // Assuming F0s is F sharp low (one less octave than F sharp)
                        BuzzerOn(370); // Frequency = 369.99 Hz, rounded to 370 for simplicity
                        break;
                    case D0s: // Assuming D0s is D sharp low (one less octave than D sharp)
                        BuzzerOn(311); // Frequency = 311.13 Hz
                        break;
                    case 0: // End of song
                        BuzzerOff();
                        break;
                }


              } else {
                  setLeds(0);
                  BuzzerOff();
                  state = STOP;
                  flagCountdown = count;
                  break;
              }

              state = INPUT;
              break;







        case INPUT:
            currState = returnStateofButtons();
                                                                                //Check the pitch to set the LED
                if ((currPitch == G) || (currPitch == D0) ) {
                    if (currState == 0x01) {
                        correctInput = 0x01;
                        configUserLEDs(0x02);
                    } else if (currState != 0x00) {
                        configUserLEDs(0x01);
                        incorrectInput = 0x01;
                    }
                } else if ((currPitch == Bb) || (currPitch == F0) ) {
                    if (currState == 0x02) {
                        correctInput = 0x01;
                        configUserLEDs(0x02);
                    } else if (currState != 0x00) {
                        configUserLEDs(0x01);
                        incorrectInput = 0x01;
                    }
                } else if ((currPitch == Cc) || (currPitch == G0) ) {
                    if (currState == 0x04) {
                        correctInput = 0x01;
                        configUserLEDs(0x02);
                    } else if (currState != 0x00) {
                        configUserLEDs(0x01);
                        incorrectInput = 0x01;
                    }
                } else if ((currPitch == Cs)||(currPitch == Ab2)) {
                    if (currState == 0x08) {
                        correctInput = 0x01;
                        configUserLEDs(0x02);
                    } else if (currState != 0x00) {
                        configUserLEDs(0x01);
                        incorrectInput = 0x01;
                    }
                } else if (currPitch == 0x00) {                     //if a 0 is detected, that means the song is over, we can assume the player won because otherwise the song would not finish
                    state = WINNER;                                 //move to winner state
                    Graphics_clearDisplay(&g_sContext);
                    currKey = 0;
                    break;
                }

            state = PLAYSONG;
            break;









        case STOP:
            if (count < (flagCountdown + 2)){     //delay after each note to differentiate notes
                break;
            }


            if ((correctInput != 0x01) || (incorrectInput == 0x01)) { //check if there was an error to add one to the number of total errors
                numError ++;
                incorrectInput = 0x00;
            } else {
                correctInput = 0x00;    //if there was no error reset the correctness indicator
            }


            if (numError == 0x0C){      //if you reach 10 errors you go to the LOSER state, you have lost the game
                state = LOSER;
                Graphics_clearDisplay(&g_sContext);
                numError = 0x00;        //reset number of errors for future
                Graphics_drawStringCentered(&g_sContext, "jaja puto", AUTO_STRING_LENGTH, 50, 45, TRANSPARENT_TEXT);
                Graphics_drawStringCentered(&g_sContext, "perdiste", AUTO_STRING_LENGTH, 50, 60, TRANSPARENT_TEXT);
                Graphics_flushBuffer(&g_sContext);
                setLeds(15);
                flagCountdown = count;
                break;
            }


            if (notes[x] == 0x00){       //if you reach the note 0x00, (our indicator for the end of the song), it means that you have not gotten 3 errors and you've won
                state = WINNER;
                Graphics_clearDisplay(&g_sContext);
                break;
            }

            x ++;                       //iterate to the next note in notes[]
            state = PLAYSONG;
            configUserLEDs(0x00);
            break;

        case LOSER:

            if ((count == (flagCountdown + 8)) || (count == (flagCountdown + 18)) || (count == (flagCountdown + 34))){
                BuzzerOff();
            }

            if (count <= (flagCountdown + 6)) {            //play first note
                BuzzerOn(66);

            } else if ((count <= (flagCountdown + 16)) && (count > (flagCountdown + 8))) {     //write 2 if count is between 10 and 20
                BuzzerOn(74);

            } else if ((count <= (flagCountdown + 32)) && (count > (flagCountdown + 18))) {      //write 1 if count is between 0 and 10
                BuzzerOn(111);
            }
            break;

        case WINNER:
            Graphics_drawStringCentered(&g_sContext, "Que crack!", AUTO_STRING_LENGTH, 50, 45, TRANSPARENT_TEXT);
            Graphics_drawStringCentered(&g_sContext, "You Win!!!", AUTO_STRING_LENGTH, 50, 55, TRANSPARENT_TEXT);
            Graphics_flushBuffer(&g_sContext);

            if (count % 4 == 0) {
                setLeds(8);
            } else if (count % 4 == 1) {
                setLeds(4);
            } else if (count % 4 == 2) {
                setLeds(2);
            } else if (count % 4 == 3) {
                setLeds(1);
            }
            break;
        }
    }
}


void SmokeOnTheWater(char *notes, int *duration){
    // Original part of the riff
    notes[0] = G;
    notes[1] = Bb;
    notes[2] = Cc;
    notes[3] = G;
    notes[4] = Bb;
    notes[5] = Cs;
    notes[6] = Cc;
    notes[7] = G;
    notes[8] = Bb;
    notes[9] = Cc;
    notes[10] = Bb;
    notes[11] = G;

    // Transposed riff an octave lower (assuming D0, F0, G0, and Ab0 are correctly defined for the lower octave)
    notes[12] = D0;
    notes[13] = F0;
    notes[14] = G0;
    notes[15] = D0;
    notes[16] = F0;
    notes[17] = Ab2; // Using Ab0 here to keep the minor feel in the lower octave
    notes[18] = G0;
    notes[19] = D0;
    notes[20] = F0;
    notes[21] = G0;
    notes[22] = F0;
    notes[23] = D0;
    notes[24] = Stop;

    duration[0] = QUARTER;
    duration[1] = QUARTER;
    duration[2] = QUARTER;
    duration[3] = QUARTER;
    duration[4] = QUARTER;
    duration[5] = QUARTER;
    duration[6] = QUARTER;
    duration[7] = QUARTER;
    duration[8] = QUARTER;
    duration[9] = HALF;
    duration[10] = HALF;
    duration[11] = FULL;
    // Repeat the durations for the riff
    duration[12] = QUARTER;
    duration[13] = QUARTER;
    duration[14] = HALF;
    duration[15] = QUARTER;
    duration[16] = QUARTER;
    duration[17] = HALF;
    duration[18] = FULL;
    duration[19] = QUARTER;
    duration[20] = QUARTER;
    duration[21] = HALF;
    duration[22] = QUARTER;
    duration[23] = FULL;
    duration[24] = 0; // End of song marker
}


void runtimerA2(void) {

    TA2CTL = TASSEL_1 | MC_1 | ID_0;
    TA2CCR0 = 162;                               // tint = 0.00494384765625 sec
    TA2CCTL0 = CCIE;
}

void stoptimerA2(int reset) {

    TA2CTL = MC_0;
    TA2CCTL0 &= ~CCIE;

    if(reset) {
        count = 0;
    }
}

#pragma vector=TIMER2_A0_VECTOR
__interrupt void Timer_A2_ISR(void) {
    count++;
}

void configLabBoardButtons(void) {
    // Configure buttons S1, S2, S3, S4 as inputs with pull-up resistors
    // Buttons are connected to P7.0 (S1), P3.6 (S2), P2.2 (S3), P7.4 (S4)
    P7DIR &= ~(BIT0 | BIT4); // Set P7.0 and P7.4 as input
    P3DIR &= ~BIT6;          // Set P3.6 as input
    P2DIR &= ~BIT2;          // Set P2.2 as input

    P7REN |= (BIT0 | BIT4);  // Enable pull-up resistors for P7.0 and P7.4
    P3REN |= BIT6;           // Enable pull-up resistors for P3.6
    P2REN |= BIT2;           // Enable pull-up resistors for P2.2

    P7OUT |= (BIT0 | BIT4);  // Set pull-up resistors for P7.0 and P7.4
    P3OUT |= BIT6;           // Set pull-up resistors for P3.6
    P2OUT |= BIT2;           // Set pull-up resistors for P2.2
}

void configUserLEDs(unsigned char inbits) {
    // Configure user LEDs LED1 and LED2 as outputs
    // LED1 is connected to P1.0 and LED2 is connected to P4.7
    P1DIR |= BIT0; // Set P1.0 as output
    P4DIR |= BIT7; // Set P4.7 as output

    // Set the LEDs according to the lower two bits of inbits
    if (inbits & BIT0) P1OUT |= BIT0; // If BIT0 is set, turn on LED1
    else P1OUT &= ~BIT0;              // Else, turn off LED1

    if (inbits & BIT1) P4OUT |= BIT7; // If BIT1 is set, turn on LED2
    else P4OUT &= ~BIT7;              // Else, turn off LED2
}

unsigned char returnStateofButtons(void) {
    unsigned char state = 0;

    // Read the buttons' state
    // A pressed button is read as 0
    if (!(P7IN & BIT0)) state |= BIT0; // If S1 is pressed
    if (!(P3IN & BIT6)) state |= BIT1; // If S2 is pressed
    if (!(P2IN & BIT2)) state |= BIT2; // If S3 is pressed
    if (!(P7IN & BIT4)) state |= BIT3; // If S4 is pressed

    return state;
}


/*In the embedded system I'm developing, I'll manage the musical functionality by utilizing a `struct`
 * in C programming. This data structure will encapsulate the characteristics of each musical note,
 * including its frequency (`pitch`), the time span for which it is held (`duration`), and an identifier
 * for the corresponding LED illumination. Since the device is equipped with a quartet of LEDs, each capable
 * of exhibiting a spectrum of colors, I'll assign unique colors to represent distinct notes.

To store the composition of an entire melody, these `structs` will be arrayed in a sequence,
creating a `song` array. The total count of notes within a song is inherently constrained by the memory
 capacity of the microcontroller. Consequently, there's a practical necessity to balance the song's length
 with the available memory resources, ensuring the song array does not exceed the storage limitations of the device.
 */


