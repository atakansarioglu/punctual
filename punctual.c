/**
 * @file      punctual.c
 * @author    Atakan S.
 * @date      01/01/2018
 * @version   1.1
 * @brief     Scheduled task and timeout framework.
 *
 * @copyright Copyright (c) 2018 Atakan SARIOGLU ~ www.atakansarioglu.com
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a
 *  copy of this software and associated documentation files (the "Software"),
 *  to deal in the Software without restriction, including without limitation
 *  the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *  and/or sell copies of the Software, and to permit persons to whom the
 *  Software is furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 *  DEALINGS IN THE SOFTWARE.
 */
 
#include "porty.h"
#ifdef _H_PUNCTUAL // Exclude from compile when not included in projech.h

#include "punctual.h"
#include <string.h>

// Variables.
tPunctualItem puncList[PUNCTUAL_MAX_TASKS];	///< Array of punctual task items.
uint32_t puncPostscaler = 0;				///< Postscaler to slow down the ISR.
uint8_t puncCnt = 0;						///< Dummy counter for thread safe read write operations.

// Init.
void PunctualInit(void) {
	// Clear the divider.
	puncPostscaler = 0;

	// Clear the list.
	memset((uint8_t *) puncList, 0, sizeof(puncList));
}

// Punctual task scheduler.
tPunctualItem *PunctualCreate(uint32_t (*handle)(void *)) {
	uint8_t i;
	uint8_t intEn;
	tPunctualItem *ptr = NULL;

	// Save state and disable the interrupts.
	intEn = mIsIntEnabled();
	mIntDisable();

	// Find a slot.
	for (i = 0; i < PUNCTUAL_MAX_TASKS; i++) {
		if (puncList[i].handle == NULL) {
			// Place the handle.
			puncList[i].handle = handle;
			ptr = &(puncList[i]);

			break;
		}
	}

	// Re-enable interrupts.
	if (intEn) {
		mIntEnable();
	}

	return ptr;
}

// Destroys a task.
void PunctualDestroy(tPunctualItem *ptr) {
	uint8_t intEn;

	// Consistency check.
	if ((ptr >= puncList) && (ptr <= &puncList[PUNCTUAL_MAX_TASKS])) {
		// Save state and disable the interrupts.
		intEn = mIsIntEnabled();
		mIntDisable();

		// Disable ISR operation.
		ptr->handle = NULL;

		// Re-enable interrupts.
		if (intEn) {
			mIntEnable();
		}
	}
}

// The interrupt routine of the scheduler.
void PunctualISR(void) {
	tPunctualItem *ptr;

	// Increase the divider.
	if ((++puncPostscaler) >= PUNCTUAL_ISR_POSTSCALER) {
		puncPostscaler = 0;

		// Increase the counter.
		puncCnt++;

		// Scan all slots.
		for (ptr = puncList; ptr < &puncList[PUNCTUAL_MAX_TASKS]; ptr++) {
			if (ptr->handle != NULL) {
				// Call it.
				ptr->result = (ptr->handle)(ptr->param);

				// Clear the param.
				ptr->param = NULL;
			}
		}
	}
}

// Send data to the task.
void PunctualSend(tPunctualItem *ptr, void *data) {
	uint8_t intEn;

	// Consistency check.
	if ((ptr >= puncList) && (ptr <= &puncList[PUNCTUAL_MAX_TASKS])) {
		// Save state and disable the interrupts.
		intEn = mIsIntEnabled();
		mIntDisable();

		// Assign the data.
		ptr->param = data;

		// Re-enable interrupts.
		if (intEn) {
			mIntEnable();
		}
	}
}

// Thread-safe read the last result.
uint32_t PunctualReceive(tPunctualItem *ptr) {
	uint8_t cnt;
	uint32_t result;

	// Thread-safe operation.
	do{
		cnt = puncCnt;
		result = ptr->result;
	}while(cnt != puncCnt);
	return result;
}

// Creates a timeout handle.
void PunctualTimeoutSet(tTimeout *to, tTime ms){
    to->period = ms;
    to->due = PunctualGetTime() + ms;
}

// Edits a timeout handle.
void PunctualTimeoutEdit(tTimeout *to, tTime ms){
	tTime timeNew = to->due - to->period;
	to->period = ms;
    to->due = timeNew + ms;
}

// Check timeout.
uint8_t PunctualTimeoutCheck(tTimeout *to){
    int32_t timeDiff;

    // Get signed time difference.
    timeDiff = to->due - PunctualGetTime();

    // Expiration check.
    if(timeDiff <= 0){
        // Increase due time.
        to->due = to->due + to->period;

        // Check the new due.
        timeDiff = to->due - PunctualGetTime();
        if(timeDiff <= 0){
            // If already expired, reset the timeout.
			to->due = PunctualGetTime() + to->period;
        }

        // Expired.
        return true;
    }else{
        // Not expired.
        return false;
    }
}

#endif // Exclude from compile when not included in projech.h
