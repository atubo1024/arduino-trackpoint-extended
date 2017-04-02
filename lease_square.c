
#include "lease_square.h"

void MLS_Init(
	struct MovingLeaseSquare *self, 
	uint16_t  winsize, 
	uint16_t *buffer_time, 
	int16_t  *buffer_value, 
	int32_t	  timeout_speed)
{
	self->mWinSize = winsize;
	self->mTimeoutSpeed = timeout_speed;
	self->mBufferTime = buffer_time;
	self->mBufferVal = buffer_value;
	self->Count = 0;
	self->mBufferStartIndex = 0;
	self->current_value = 0;
	self->mSumT = 0;
	self->mSumV = 0;
	self->mSumTT = 0;
	self->mSumTV = 0;
	self->mLastUpdateTime = 0;
}

static void RemoveHead(struct MovingLeaseSquare *self)
{
	if (self->Count == 0) return;

	uint16_t time;
	int16_t value;
	uint16_t index;

	index = self->mBufferStartIndex;
	time = self->mBufferTime[index];
	value = self->mBufferVal[index];

	self->mSumT -= time;
	self->mSumTT -= time * time;
	self->mSumV -= value;
	self->mSumTV -= ((int32_t) time) * value;
	self->mBufferStartIndex = (index + 1) % self->mWinSize;
	self->Count--;
}

void MLS_PeriodicHandler(struct MovingLeaseSquare *self, uint16_t now)
{
	if (now - self->mLastUpdateTime > self->mTimeoutSpeed) {
		RemoveHead(self);
		self->mLastUpdateTime = now;
	}
}

void MLS_Append(struct MovingLeaseSquare *self, uint16_t now, int16_t value)
{
	uint32_t sumT, sumTT;
	int32_t sumV, sumTV;
	uint16_t count;

	count = self->Count;
	if (count >= self->mWinSize) {
		RemoveHead(self);
		count--;
	}
	sumT = self->mSumT + (int32_t) now;
	sumTT = self->mSumTT + (int32_t) (now * now);
	sumV = self->mSumV + value;
	sumTV = self->mSumTV + ((int32_t) now) * value;

	self->mSumT = sumT;
	self->mSumV = sumV;
	self->mSumTT += sumTT;
	self->mSumTV += sumTV;
	count++;
	self->count = count;
	self->mLastUpdateTime = now;

	uint16_t index = (self->mBufferStartIndex + count) % self->mWinSize;
	self->mBufferTime[index] = now;
	self->mBufferVal[index] = value;

	if (count > 1) {
		float a, b;
		b = ((float)sumTT) * count - sumT * sumT;
		a = ((float)(sumTT * sumV - sumT * sumTV)) / b;
		b = (sumTV * ((int32_t) count) - sumT * sumV) / b;
		self->current_value = a * now + b;
	} else {
		self->current_value = value;
	}
}


