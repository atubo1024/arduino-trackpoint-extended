
#include "lease_square.h"

void MLS_Init(
	struct MovingLeaseSquare *self, 
	uint16_t  winsize, 
	uint16_t *buffer_time, 
	int16_t  *buffer_value)
{
	self->mWinSize = winsize;
	self->mBufferTime = buffer_time;
	self->mBufferVal = buffer_value;
	self->count = 0;
	self->mBufferStartIndex = 0;
	self->current_value = 0;
	self->mSumT = 0.0f;
	self->mSumV = 0.0f;
	self->mSumTT = 0.0f;
	self->mSumTV = 0.0f;
	self->last_update_time = 0;
}

static void RemoveHead(struct MovingLeaseSquare *self)
{
	if (self->count == 0) return;

	uint16_t time;
	int16_t value;
	uint16_t index;
	float tempf32;

	index = self->mBufferStartIndex;
	time = self->mBufferTime[index];
	value = self->mBufferVal[index];

	self->mSumT -= time;
	self->mSumV -= value;
	tempf32 = (float)time;
	self->mSumTT -= tempf32 * tempf32;
	self->mSumTV -= tempf32 * value;
	self->mBufferStartIndex = (index + 1) % self->mWinSize;
	self->count--;
}

void MLS_Append(struct MovingLeaseSquare *self, uint16_t now, int16_t value)
{
	float sumT, sumTT, sumV, sumTV;
	uint16_t count;

	count = self->count;
	if (count >= self->mWinSize) {
		RemoveHead(self);
		count--;
	}
	sumT = self->mSumT + now;
	sumV = self->mSumV + value;
	sumTV = (float) now;
	sumTT = self->mSumTT + sumTV * sumTV;
	sumTV = self->mSumTV + sumTV * ((float) value);


	self->mSumT = sumT;
	self->mSumV = sumV;
	self->mSumTT = sumTT;
	self->mSumTV = sumTV;
	count++;
	self->count = count;
	self->last_update_time = now;

	uint16_t index = (self->mBufferStartIndex + count) % self->mWinSize;
	self->mBufferTime[index] = now;
	self->mBufferVal[index] = value;

	if (count > 1) {
		float a, b;
		a = sumTT * count - sumT * sumT;
		if (a < 0.001f) {
			self->current_value = value;
		} else {
			b = (sumTT * sumV - sumT * sumTV) / a;
			a = (sumTV * count - sumT * sumV) / a;
			self->current_value = (int16_t)(a * now + b);
		}
	} else {
		self->current_value = value;
	}
}


