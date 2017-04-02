#ifndef __HAVE_LEASE_SQUARE_H
#define __HAVE_LEASE_SQUARE_H

#include <stdint.h>

struct MovingLeaseSquare
{
	uint16_t count;
	int16_t  current_value;
	uint16_t mWinSize;
	uint16_t mTimeoutSpeed;
	uint16_t *mBufferTime;
	int16_t  *mBufferVal;
	uint16_t mLastUpdateTime;
	uint16_t mBufferStartIndex;
	int32_t  mSumT;
	int32_t  mSumV;
	int32_t  mSumTT;
	int32_t  mSumTV;
};

void MLS_Init(
	struct MovingLeaseSquare *self, 
	uint16_t  winsize, 
	uint16_t *buffer_time, 
	int16_t  *buffer_value, 
	int32_t	  timeout_speed);
void MLS_PeriodicHandler(struct MovingLeaseSquare *self, uint16_t now);
void MLS_Append(struct MovingLeaseSquare *self, uint16_t now, int16_t value);

#endif		/* #ifndef __HAVE_LEASE_SQUARE_H */

