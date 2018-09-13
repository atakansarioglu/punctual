/**
 * @file      punctual.h
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

#ifndef _H_PUNCTUAL
#define _H_PUNCTUAL

#include <porty.h>

// Settings.
#ifndef PUNCTUAL_MAX_TASKS
#define PUNCTUAL_MAX_TASKS    	(4) 											///< Maximum numeber of scheduled tasks.
#endif
#ifndef PUNCTUAL_ISR_PERIOD_US
#define PUNCTUAL_ISR_PERIOD_US 	(TICKER_PERIOD_US)								///< ISR calling period in [us].
#endif
#ifndef PUNCTUAL_ISR_POSTSCALER
#define PUNCTUAL_ISR_POSTSCALER (1) 											///< Scheduled task ISR prescaler.
#endif
#ifndef PUNCTUAL_TO_TICKTIME_US
#define PUNCTUAL_TO_TICKTIME_US	(TICKER_TICKTIME_US)							///< SysTick time in [us]
#endif

// Don't touch.
#define PUNCTUAL_PERIOD_US     	(PUNCTUAL_ISR_PERIOD_US*PUNCTUAL_ISR_POSTSCALER)///< Task calling period in [us].
#define PUNCTUAL_TO_MAXTIME_MS	(0x10000000 / TO_TICKS_PER1MS)					///< This is the maximum timeout period in [ms].
#define PUNCTUAL_TO_MAXTIME_SEC	(0x10000000 / TO_TICKS_PER1S)					///< This is the maximum timeout period in [s].
#ifdef _H_TICKER
#undef mTickerHook_ms
#define mTickerHook_ms()		PunctualISR()					                ///< Setup hook to Ticker.
#endif

// Type definition.
typedef struct{
    uint32_t (*handle)(void *);													///< Handle of task body.
    uint32_t result;															///< return value of the last call.
    void *param;																///< parameter to passs to the task.
}tPunctualItem;
typedef uint32_t tTime;															///< Keeps time.
typedef struct{
    tTime due;																	///< Timestamp of the next timeout.
    tTime period;																///< The period for reloading the timeout.
}tTimeout;

/**
 * @brief Init.
 */
void PunctualInit(void);

/**
 * @brief Punctual task scheduler.
 * @param handle handle of the task body to be called.
 * @return pointer of the created task.
 */
tPunctualItem *PunctualCreate(uint32_t (*handle)(void *));

/**
 * @bried Destroys a task.
 * @param ptr task pointer.
 */
void PunctualDestroy(tPunctualItem *ptr);

/**
 * @brief The interrupt routine of the scheduler.
 */
void PunctualISR(void);

/**
 * @brief Send data to the task.
 * @param ptr task pointer.
 * @param data pointer to the data or data itself.
 */
void PunctualSend(tPunctualItem *ptr, void *data);

/**
 * @brief Thread-safe read the last result.
 * @param ptr task pointer.
 * @return gives the return value of the scheduled task.
 */
uint32_t PunctualReceive(tPunctualItem *ptr);

/**
 * @brief Sets a timeout object with the given period.
 * @param to the handle of the created timeout.
 * @param ms desired period in [ms].
 */
void PunctualTimeoutSet(tTimeout *to, tTime ms);

/**
 * @brief Edits a timeout object with the given period without restarting.
 * @param to the handle of the created timeout.
 * @param ms new period in [ms].
 */
void PunctualTimeoutEdit(tTimeout *to, tTime ms);

/**
 * @brief Gets the timeout state of the provided handle.
 * @param to the handle of the created timeout.
 * @return true if the timeout has been reached.
 */
uint8_t PunctualTimeoutCheck(tTimeout *to);

/**
 * @brief Gets the timeout state of the provided handle.
 * @param to the handle of the created timeout.
 * @return true if the timeout has been reached.
 */
#define isPunctualTimeoutExpired(to) PunctualTimeoutCheck(to)

/**
 * @brief Macro for reading the time from SysTick.
 * @return tTime time in [ms].
 */
#define PunctualGetTime() TickerRead_ms()

#endif
