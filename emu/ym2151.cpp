// license:BSD-3-Clause
// copyright-holders:Aaron Giles

#include <vector>
using offs_t = unsigned int;
#include "ym2151.h"

//*********************************************************
//  YM2151 DEVICE
//*********************************************************

//-------------------------------------------------
//  ym2151_device - constructor
//-------------------------------------------------

ym2151_device::ym2151_device(uint32_t clock) :
	m_fm(),
	m_address(0),
	m_reset_state(1)
{
}

// defle custom
void ym2151_device::set_mute(int mask)
{
	mute_mask = mask;
}

float ym2151_device::get_volume(int channel)
{
	return (float) abs(channelsOutput[channel]) / 10000.0f;
}

//-------------------------------------------------
//  read - handle a read from the device
//-------------------------------------------------

u8 ym2151_device::read(offs_t offset)
{
	u8 result = 0xff;
	switch (offset & 1)
	{
		case 0: // data port (unused)
			printf("Unexpected read from YM2151 offset %d\n", offset & 3);
			break;

		case 1:	// status port, YM2203 compatible
			result = m_fm.status();
			break;
	}
	return result;
}


//-------------------------------------------------
//  write - handle a write to the register
//  interface
//-------------------------------------------------

void ym2151_device::write(offs_t offset, u8 value)
{
	// ignore writes when the reset is active (low)
	if (m_reset_state == 0)
		return;

	switch (offset & 1)
	{
		case 0: // address port
			m_address = value;
			break;

		case 1: // data port
			// write to FM
			m_fm.write(m_address, value);

			// special cases
			if (m_address == 0x01 && BIT(value, 1))
			{
				// writes to the test register can reset the LFO
				m_fm.reset_lfo();
			}
			else if (m_address == 0x1b)
			{

			}
			break;
	}
}

//-------------------------------------------------
//  device_start - start of emulation
//-------------------------------------------------

void ym2151_device::device_start()
{
	// call this for the variants that need to adjust the rate
	device_clock_changed();
}


//-------------------------------------------------
//  device_reset - start of emulation
//-------------------------------------------------

void ym2151_device::device_reset()
{
	// reset the engines
	m_fm.reset();
}


//-------------------------------------------------
//  device_clock_changed - update if clock changes
//-------------------------------------------------

void ym2151_device::device_clock_changed()
{

}


void ym2151_device::sound_stream_update_common(int *output){
	// iterate over all target samples
	for (int sampindex = 0; sampindex < 1; sampindex++)
	{
		// clock the system
		m_fm.clock(fm_engine::ALL_CHANNELS);

		// update the FM content; YM2151 is full 14-bit with no intermediate clipping
		s32 sums[fm_engine::OUTPUTS] = { 0 };
		m_fm.output(sums, 0, 32767, fm_engine::ALL_CHANNELS, mute_mask, 0xFF, 0xFF);

		// convert to 10.3 floating point value for the DAC and back
		// YM2151 is stereo
		if(sums[0]>32768)sums[0]=32768;
		if(sums[1]>32768)sums[1]=32768;
		output[0]=ymfm_roundtrip_fp(sums[0]);
		output[1]=ymfm_roundtrip_fp(sums[1]);
	}
}

//*********************************************************
//  YM2164 DEVICE
//*********************************************************

//-------------------------------------------------
//  ym2164_device - constructor
//-------------------------------------------------

ym2164_device::ym2164_device(uint32_t clock) :
	ym2151_device(clock)
{
}


//-------------------------------------------------
//  write - handle a write to the register
//  interface
//-------------------------------------------------

void ym2164_device::write(offs_t offset, u8 value)
{
	// ignore writes when the reset is active (low)
	if (m_reset_state == 0)
		return;

	switch (offset & 1)
	{
		case 0: // address port
			m_address = value;
			break;

		case 1: // data port
			// write to FM
			m_fm.write(m_address, value);
			break;
	}
}



//*********************************************************
//  YM2151 DEVICE
//*********************************************************

//-------------------------------------------------
//  ym2414_device - constructor
//-------------------------------------------------

ym2414_device::ym2414_device(uint32_t clock) :
	ym2151_device(clock)
{
}
