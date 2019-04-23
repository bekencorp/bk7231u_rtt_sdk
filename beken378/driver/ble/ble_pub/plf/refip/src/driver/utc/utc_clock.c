/**
****************************************************************************************
* @addtogroup utc_clock
* @ingroup beken corp
* @brief utc_clock
* @author Alen
*
* This is the driver block for utc_clock
* @{
****************************************************************************************
*/


#include <stdint.h>        // standard integer definition
#include <string.h>        // string manipulation
#include <stddef.h>        // standard definition
#include "lld_evt.h"
#include "utc_clock.h"




static void utc_clock_update( UTCTime elapsedMSec );
static void utc_set_clock( uint32_t newTime );
static uint32_t utc_get_clock( void );
static void utc_convert_utc_time(UTCTimeStruct *tm, uint32_t secTime );
static uint8_t monthLength( uint8_t lpyr, uint8_t mon );
static uint32_t utc_convert_utc_secs( UTCTimeStruct *tm );

/*********************************************************************
 * MACROS
 */

#define	IsLeapYear(yr)	(!((yr) % 400) || (((yr) % 100) && !((yr) % 4)))
#define	YearLength(yr)	(IsLeapYear(yr) ? 366 : 365)

#define	BEGYEAR	        2000     // UTC started at 00:00:00 January 1, 2000

#define	DAY             86400UL  // 24 hours * 60 minutes * 60 seconds

// (MAXCALCTICKS * 5) + (max remainder) must be <= (RW MAX basetimecnt)[26 - 0], 0x7FFFFFF
// so: (0x1999998 * 5) + 7 <= 0x7FFFFFF
#define MAXCALCTICKS  ((uint32_t)(0x1999998))


/*********************************************************************
 * LOCAL VARIABLES
 */
static uint32_t previousRwTimerTick = 0;
static uint32_t remUsTicks = 0;
static uint32_t timeMSec = 0;
uint32_t rtc_timeSeconds = 0; 



void utc_set_time(UTCTimeStruct *tm)
{
    UTCTimeStruct myTime;
	
    myTime.year  = tm->year + 2000;
    myTime.month = tm->month;
    if ( myTime.month > 0 )
    {
        myTime.month--;
    }
    myTime.day = tm->day;
    if ( myTime.day > 0 )
    {
        myTime.day--;
    }
    myTime.hour  = tm->hour;
    myTime.minutes = tm->minutes;
    myTime.seconds = tm->seconds;

	//set clock
	utc_set_clock( utc_convert_utc_secs( &myTime ) );
	
}


void utc_get_time(UTCTimeStruct *tm)
{
    utc_convert_utc_time( tm, utc_get_clock());     
    tm->month ++;
    tm->day ++;	
	tm->year= (uint16_t)( tm->year -2000 );
}


static void utc_set_clock( UTCTime newTime )
{
  	rtc_timeSeconds = newTime;
}


static uint32_t utc_get_clock( void )
{
  return ( rtc_timeSeconds );
}


void utc_update( void )
{
  uint32_t tmp;
  uint32_t ticks625us;
  uint32_t elapsedMSec = 0;

  // Get the free-running count of 625us timer ticks
  tmp = lld_evt_time_get();
  //UART_PRINTF("tmp->%lu\r\n", tmp);

  if ( tmp != previousRwTimerTick )
  {
    // Calculate the elapsed ticks of the free-running timer.
    ticks625us = tmp - previousRwTimerTick;

    // Store the LL Timer tick count for the next time through this function.
    previousRwTimerTick = tmp;

    /* It is necessary to loop to convert the usecs to msecs in increments so as
     * not to overflow the 32-bit variables.
     */
    while ( ticks625us > MAXCALCTICKS )
    {
      ticks625us -= MAXCALCTICKS;
      elapsedMSec += MAXCALCTICKS * 5 / 8;
      remUsTicks += MAXCALCTICKS * 5 % 8;
    }

    // update converted number with remaining ticks from loop and the
    // accumulated remainder from loop
    tmp = (ticks625us * 5) + remUsTicks;

    // Convert the 625 us ticks into milliseconds and a remainder
    elapsedMSec += tmp / 8;
    remUsTicks = tmp % 8;

    // Update Clock
    if ( elapsedMSec )
    {
      utc_clock_update( elapsedMSec );
    }
  }
}



/*********************************************************************
 * @fn      utc_clock_update
 *
 * @brief   Updates the OSAL Clock time with elapsed milliseconds.
 *
 * @param   elapsedMSec - elapsed milliseconds
 *
 * @return  none
 */
static void utc_clock_update( UTCTime elapsedMSec )
{
  // Add elapsed milliseconds to the saved millisecond portion of time
  timeMSec += elapsedMSec;

  // Roll up milliseconds to the number of seconds
  if ( timeMSec >= 1000 )
  {
    rtc_timeSeconds += timeMSec / 1000;
    timeMSec = timeMSec % 1000;
  }
}


/*********************************************************************
 * @fn      utc_convert_utc_time
 *
 * @brief   Converts UTCTime to UTCTimeStruct
 *
 * @param   tm - pointer to breakdown struct
 *
 * @param   secTime - number of seconds since 0 hrs, 0 minutes,
 *          0 seconds, on the 1st of January 2000 UTC
 *
 * @return  none
 */
static void utc_convert_utc_time(UTCTimeStruct *tm, UTCTime secTime )
{
  // calculate the time less than a day - hours, minutes, seconds
  {
    uint32_t day = secTime % DAY;
	tm->hour = day / 3600UL;
    tm->seconds = day % 60UL;
    tm->minutes = (day % 3600UL) / 60UL;
  }
	
  // Fill in the calendar - day, month, year
  {
    uint16_t numDays = secTime / DAY;
    tm->year = BEGYEAR;
    while ( numDays >= YearLength( tm->year ) )
    {
      numDays -= YearLength( tm->year );
      tm->year++;
    }

    tm->month = 0;
    while ( numDays >= monthLength( IsLeapYear( tm->year ), tm->month ) )
    {
      numDays -= monthLength( IsLeapYear( tm->year ), tm->month );
      tm->month++;
    }

    tm->day = numDays;
  }
}

/*********************************************************************
 * @fn      monthLength
 *
 * @param   lpyr - 1 for leap year, 0 if not
 *
 * @param   mon - 0 - 11 (jan - dec)
 *
 * @return  number of days in specified month
 */
static uint8_t monthLength( uint8_t lpyr, uint8_t mon )
{
  uint8_t days = 31;

  if ( mon == 1 ) // feb
  {
    days = ( 28 + lpyr );
  }
  else
  {
    if ( mon > 6 ) // aug-dec
    {
      mon--;
    }

    if ( mon & 1 )
    {
      days = 30;
    }
  }

  return ( days );
}



/*********************************************************************
 * @fn      utc_convert_utc_secs
 *
 * @brief   Converts a UTCTimeStruct to UTCTime
 *
 * @param   tm - pointer to provided struct
 *
 * @return  number of seconds since 00:00:00 on 01/01/2000 (UTC)
 */
static UTCTime utc_convert_utc_secs( UTCTimeStruct *tm )
{
  uint32_t seconds;

  /* Seconds for the partial day */
  seconds = (((tm->hour * 60UL) + tm->minutes) * 60UL) + tm->seconds;

  /* Account for previous complete days */
  {
    /* Start with complete days in current month */
    uint16_t days = tm->day;

    /* Next, complete months in current year */
    {
      int8_t month = tm->month;
      while ( --month >= 0 )
      {
        days += monthLength( IsLeapYear( tm->year ), month );
      }
    }

    /* Next, complete years before current year */
    {
      uint16_t year = tm->year;
      while ( --year >= BEGYEAR )
      {
        days += YearLength( year );
      }
    }

    /* Add total seconds before partial day */
    seconds += (days * DAY);
  }

  return ( seconds );
}

