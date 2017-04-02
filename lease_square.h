#ifndef __HAVE_LEASE_SQUARE_H
#define __HAVE_LEASE_SQUARE_H

#include <stdint.h>

struct MovingLeaseSquare
{
	uint16_t count;
	int16_t  current_value;

	uint16_t mWinSize;
	uint16_t reverse;
	uint16_t last_update_time;
	uint16_t mBufferStartIndex;
	uint16_t *mBufferTime;
	int16_t  *mBufferVal;

	float    mSumT;
	float    mSumV;
	float    mSumTT;
	float    mSumTV;
};

void MLS_Init(
	struct MovingLeaseSquare *self, 
	uint16_t  winsize, 
	uint16_t *buffer_time, 
	int16_t  *buffer_value);
void MLS_Append(struct MovingLeaseSquare *self, uint16_t now, int16_t value);

#endif		/* #ifndef __HAVE_LEASE_SQUARE_H */

