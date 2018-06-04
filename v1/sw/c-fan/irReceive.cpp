/*
Author:  Stefano Di Paolo
License: MIT, https://en.wikipedia.org/wiki/MIT_License
Date:    2018-03-18

Ceiling Fan (http://www.microentropie.com)
Arduino ESP8266 based.

Sources repository: https://github.com/microentropie/
*/

/*
 * IRremoteESP8266: IRrecvDumpV2 - dump details of IR codes with IRrecv
 * An IR detector/demodulator must be connected to the input RECV_PIN.
 * Example circuit diagram:
 *  https://github.com/markszabo/IRremoteESP8266/wiki#ir-receiving
 * Changes:
 *   Version 0.2 April, 2017
 *     - Decode from a copy of the data so we can start capturing faster thus
 *       reduce the likelihood of miscaptures.
 * Based on Ken Shirriff's IrsendDemo Version 0.1 July, 2009, Copyright 2009 Ken Shirriff, http://arcfn.com
 */
#include "debugUtl.h"
#ifdef VERBOSE_SERIAL
#define VERBOSE_IR_SERIAL
#endif //VERBOSE_SERIAL
//#define VERBOSE_IR_SERIAL

#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#ifdef VERBOSE_IR_SERIAL
#include <IRutils.h>
#endif //VERBOSE_SERIAL
#include "common_io.h"

// As this program is a special purpose capture/decoder, let us use a larger
// than normal buffer so we can handle Air Conditioner remote codes.
uint16_t CAPTURE_BUFFER_SIZE = 1024;

// Nr. of milli-Seconds of no-more-data before we consider a message ended.
// NOTE: Don't exceed MAX_TIMEOUT_MS. Typically 130ms.
#define TIMEOUT 15U  // Suits most messages, while not swallowing repeats.
// #define TIMEOUT 90U  // Suits messages with big gaps like XMP-1 & some aircon
                        // units, but can accidently swallow repeated messages
                        // in the rawData[] output.

// Set the smallest sized "UNKNOWN" message packets we actually care about.
// This value helps reduce the false-positive detection rate of IR background
// noise as real messages. The chances of background IR noise getting detected
// as a message increases with the length of the TIMEOUT value. (See above)
// The downside of setting this message too large is you can miss some valid
// short messages for protocols that this library doesn't yet decode.
//
// Set higher if you get lots of random short UNKNOWN messages when nothing
// should be sending a message.
// Set lower if you are sure your setup is working, but it doesn't see messages
// from your device. (e.g. Other IR remotes work.)
// NOTE: Set this value very high to effectively turn off UNKNOWN detection.
#define MIN_UNKNOWN_SIZE 12


static IRrecv *pIrRecv = NULL;

bool IsIrEnabled()
{
  return pIrRecv != NULL;
}

void IrRxStart()
{
  /*
  // Give the 'save' copy the same sized buffer.
  save.rawbuf = new uint16_t[irrecv.getBufSize()];
  if (save.rawbuf == NULL) {  // Check we allocated the memory successfully.
    Serial.printf("Could not allocate a %d buffer size for the save buffer.\n"
                  "Try a smaller size for CAPTURE_BUFFER_SIZE.\nRebooting!",
                  irrecv.getBufSize());
    ESP.restart();
  }
  */
  pIrRecv = new IRrecv(IR_RECEIVER_PIN, CAPTURE_BUFFER_SIZE, TIMEOUT, true);
  // Ignore messages with less than minimum on or off pulses.
  pIrRecv->setUnknownThreshold(MIN_UNKNOWN_SIZE);
  pIrRecv->enableIRIn();  // Start the receiver
  Serial.println("IR receiver up and running.");
}

void IrRxStop()
{
  Serial.println("IR receiver is disabled.");
  if (!pIrRecv) return; // IR not enabled
  pIrRecv->disableIRIn();
  pIrRecv->resume();
  delete pIrRecv;
  pIrRecv = NULL;
}

// The repeating section of the code
//
uint64_t IrRxLoop()
{
  if (!pIrRecv) return 0ULL; // IR not enabled

  decode_results results;  // Somewhere to store the results

  // Check if the IR code has been received.
  if (pIrRecv->decode(&results))
  {
#ifdef VERBOSE_IR_SERIAL
    Serial.println(resultToHumanReadableBasic(&results)); // Output the results
    Serial.println(resultToTimingInfo(&results));         // Output the results in RAW format
    Serial.println(resultToSourceCode(&results));         // Output the results as source code
#endif //VERBOSE_IR_SERIAL
    pIrRecv->resume();            // Receive the next value
    return results.value;
  }
  return 0ULL;
}
