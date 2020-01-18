//#############################################################
//# file:        DumpNanoEeprom.ino                           #
//# version:     0.9.0                                        #
//# author:      Mark Ebersole                                #
//# contact:     markebby@gmail.com                           #
//#                                                           #
//# description: Place text strings with info in second       #
//#              half of Arduino Nano EEPROM.                 #
//#                                                           #
//#############################################################

//------------------------------------------------------------------------------------------------------------
#include <Streaming.h>
#include <EEPROM.h>

#define I_LENGTH    100
#define I_DONE      222
#define I_FIRST     0


int     InitialOffset = 512;
int     CurrentItem = -1;
int     NextItem = I_LENGTH;
String  Str;

struct STRING_DATA_S
    {
    int Length;
    int Count;
    };
STRING_DATA_S     Data;

//------------------------------------------------------------------------------------------------------------
// SaveString - Save string to EEprom offset 512 + 4 + offset for length.
//------------------------------------------------------------------------------------------------------------
void SaveString ()
    {
    for ( int z = 0;  z <= Str.length ();  z++ )
        EEPROM.update  (4 + InitialOffset + (CurrentItem * Data.Length) + z, Str.charAt (z));
    }

//------------------------------------------------------------------------------------------------------------
// SaveData - Save data structure to EEpromt offset 512.
//------------------------------------------------------------------------------------------------------------
void SaveData ()
    {
    EEPROM.put (512, Data);
    }

//------------------------------------------------------------------------------------------------------------
// EnterNumber - Enter a numurical value between.
//               Return -1 if still entering value
//------------------------------------------------------------------------------------------------------------
int EnterNumber (char c)
    {
    if ( isDigit (c) )
        {
        Serial << c;
        Str += c;
        }
    else if ( c == 0x0D )    // CR
        {
        if ( Str.length () )
            {
            int val = Str.toInt ();
            if ( val >= 60 )    // 60 bytes max length
                val = 60;
            return (val);
            }
        else
            return (0);
        }
    else if ( (c == 0x08) && Str.length () )
        {
        Serial << c << ' ' << c;
        Str.remove (Str.length () - 1);
        }
    else if ( c == '\e' )    // ESC
        return (0);
    return (-1);
    }

//------------------------------------------------------------------------------------------------------------
// EnterString - Enter a numurical value.
//               Return -1 if still entering value
//------------------------------------------------------------------------------------------------------------
int EnterString (char c)
    {
    if ( c == 0x0D )    // CR
        {
        return (1);
        }
    else if ( (c == 0x08) && Str.length () )
        {
        Serial << "\b \b";
        Str.remove (Str.length () - 1);
        }
    else if ( c == '\e' )    // ESC
        return (0);
    else
        {
        Serial << c;
        Str += c;
        if ( Str.length () > Data.Length )
            {
            Serial << "\b \b";
            Str.remove (Str.length () - 1);
            }
        }
    return (-1);
    }

//------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------
// loop - primary program loop
//------------------------------------------------------------------------------------------------------------
void loop (void)
    {

    if ( CurrentItem != NextItem )
        {
        CurrentItem = NextItem;
        Str.remove (0);
        switch ( CurrentItem )
            {
            case I_LENGTH:
                Serial << "    Enter string length in decimal >";
                break;
            case I_DONE:
                Serial << endl << endl;
                Serial << "   Completed " << Data.Count << " entries of " << Data.Length << " characters." << endl;
                Serial << "   A total of " << 4 + (Data.Length * Data.Count) << " bytes used in second half of EEPROM." << endl << endl;
                break;
            default:
                Serial << "    Enter string #" << CurrentItem + 1 << " >";
                break;
            }
        }

    int rc;
    int inbyte = Serial.read ();
    if ( inbyte < 0 )
        {
        delay (100);
        return;
        }
    char c = inbyte & 0xFF;
    switch ( CurrentItem )
        {
        case I_LENGTH:
            rc = EnterNumber (c);
            if ( rc == -1 )
                return;
            if ( rc != 0 )
                {
                Data.Length = rc;               // set maximum string length
                NextItem = 0;
                Serial << endl << endl;
                }
            else
                {
                CurrentItem = -1;               // abort retry string length
                Serial << "\r\e[K\r";
                }
            break;
        case I_DONE:
            break;                              // do nothing because we are done.
        default:
            rc = EnterString (c);
            if ( rc == -1 )
                return;
            if ( rc != 0 )
                {
                if ( Str.length () )
                    {
                    Serial << endl << endl;
                    SaveString ();
                    NextItem = CurrentItem + 1;
                    }
                else
                    {
                    if ( CurrentItem )              // empty string and at least one entry so, we are done
                        {
                        Data.Count = CurrentItem;
                        SaveData ();
                        NextItem = I_DONE;
                        }
                    else
                        {
                        CurrentItem = -1;           // no entries yet so retry
                        Serial << "\r\e[K\r";
                        }
                    }
                }
            else
                {
                CurrentItem = -1;                   // abort retry string
                Serial << "\r\e[K\r";
                }
            break;
        }
    }

//------------------------------------------------------------------------------------------------------------
// setup - primary input output and system setup
//------------------------------------------------------------------------------------------------------------
void setup (void)
    {
    Serial.begin (9600);                // init serial
    Serial << "\e[?25h\e[2J\e[H\n\n Enter EEPROM labels and serial number\r\n\n";
    }




