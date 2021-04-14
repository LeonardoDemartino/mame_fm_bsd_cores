// license:BSD-3-Clause
// copyright-holders:Aaron Giles

#include "weird-mame-types.h"
#include "ym2610.h"

//*********************************************************
//  YM2610/YM2610B DEVICE
//*********************************************************

//-------------------------------------------------
//  ym2610_device - constructor
//-------------------------------------------------

ym2610_device::ym2610_device(uint32_t clock, u8 fm_mask) :
	m_adpcm_a(8),
	m_adpcm_b(8),
	ay8910_device(clock, PSG_TYPE_YM, 1, 0),
	m_fm(),
	m_address(0),
	m_fm_mask(fm_mask),
	m_eos_status(0x00),
	m_flag_mask(0xbf)
{
}

//-------------------------------------------------
//  ym2610b_device - constructor
//-------------------------------------------------

ym2610b_device::ym2610b_device(uint32_t clock) :
	ym2610_device(clock, 0x3f)
{
}


// defle custom
void ym2610_device::set_mute(int mask, int mask_ch2)
{
	mute_mask = (mask<<2) & 0xFFFFFFF0;
	mute_mask |= (mask&0x03)<<1;
	mute_mask_special_operators = mask_ch2;
}

float ym2610_device::get_volume(int channel)
{
	if(channel>=0)channel++;
	if(channel>=3)channel++;
	if(channel>5)return (float) abs(channelsOutput[channel]) / 15.0f;
	return (float) (abs((float)channelsOutput[channel]) / 10000.0f);
}

void ym2610_device::set_adpcma_memory(unsigned char *ptr, unsigned int size){
	m_adpcm_a.set_memory(ptr, size);
}
//-------------------------------------------------
//  read - handle a read from the device
//-------------------------------------------------

u8 ym2610_device::read(offs_t offset)
{
	u8 result = 0;
	switch (offset & 3)
	{
		case 0:	// status port, YM2203 compatible
			result = m_fm.status() & (fm_engine::STATUS_TIMERA | fm_engine::STATUS_TIMERB | fm_engine::STATUS_BUSY);
			break;

		case 1: // data port (only SSG)
			if (m_address < 0x10)
				result = ay8910_read_ym();
			else if (m_address == 0xff)
				result = 1;  // ID code
			break;

		case 2:	// status port, extended
			result = m_eos_status & m_flag_mask;
			break;

		case 3: // ADPCM-B data
			break;
	}
	return result;
}


//-------------------------------------------------
//  write - handle a write to the register
//  interface
//-------------------------------------------------

void ym2610_device::write(offs_t offset, u8 value)
{
	switch (offset & 3)
	{
		case 0: // address port
			m_address = value;

			// write register to SSG emulator
			if (m_address < 0x10)
				ay8910_write_ym(0, m_address);
			break;

		case 1: // data port

			// ignore if paired with upper address
			if (BIT(m_address, 8))
				break;

			if (m_address < 0x10)
			{
				// write to SSG
				ay8910_write_ym(1, value);
			}
			else if (m_address < 0x1c)
			{
				// write to ADPCM-B
				u8 address = m_address & 0x0f;

				// YM2610 effectively forces external mode on, and disables recording
				if (address == 0)
					value = (value | 0x20) & ~0x40;
				m_adpcm_b.write(address, value);
			}
			else if (m_address == 0x1c)
			{
				// EOS flag reset
				m_flag_mask = ~value;
				m_eos_status &= ~value;
			}
			else
			{
				// write to FM
				m_fm.write(m_address, value);
			}
			break;

		case 2: // upper address port
			m_address = 0x100 | value;
			break;

		case 3: // upper data port

			// ignore if paired with lower address
			if (!BIT(m_address, 8))
				break;

			if (m_address < 0x130)
			{
				// write to ADPCM-A
				m_adpcm_a.write(m_address & 0x3f, value);
			}
			else
			{
				// write to FM
				m_fm.write(m_address, value);
			}
			break;
	}
}

//-------------------------------------------------
//  device_start - start of emulation
//-------------------------------------------------

void ym2610_device::device_start()
{
	// call our parent
	ay8910_device::device_start();
	ay8910_device::set_psg_type(PSG_TYPE_YM);
}

//-------------------------------------------------
//  device_reset - start of emulation
//-------------------------------------------------

void ym2610_device::device_reset()
{
	// call our parent
	ay8910_device::device_reset();

	// reset the engines
	m_fm.reset();
	m_adpcm_a.reset();
	m_adpcm_b.reset();

	// initialize our special interrupt states
	m_eos_status = 0x00;
	m_flag_mask = 0xbf;
}


//-------------------------------------------------
//  device_clock_changed - update if clock changes
//-------------------------------------------------

void ym2610_device::device_clock_changed()
{
	ay_set_clock(clock() / 4);
}

void ym2610_device::sound_stream_update_common_ssg(int *output){
	ay8910_device::sound_stream_update_common(output);
}

void ym2610_device::sound_stream_update_common(int *output){
	// iterate over all target samples
	for (int sampindex = 0; sampindex < 1; sampindex++)
	{
		// clock the FM
		u32 env_counter = m_fm.clock(m_fm_mask);

		// clock the ADPCM-A engine on every envelope cycle
		if (BIT(env_counter, 0, 2) == 0)
			m_eos_status |= m_adpcm_a.clock(0x3f);

		// clock the ADPCM-B engine every cycle
		m_adpcm_b.clock(0x01);
		if ((m_adpcm_b.status() & ymadpcm_b_channel::STATUS_EOS) != 0)
			m_eos_status |= 0x80;

		// update the FM content; YM2610 is 13-bit with no intermediate clipping
		s32 sums[fm_engine::OUTPUTS] = { 0 };
		m_fm.output(sums, 1, 32767, m_fm_mask, mute_mask, 2, mute_mask_special_operators);

		// mix in the ADPCM
		m_adpcm_a.output(sums, 0x3f, mute_mask);
		m_adpcm_b.output(sums, 2, 0x01, mute_mask);

		// YM2608 is stereo
		if(sums[0]>32768)sums[0]=32768;
		if(sums[1]>32768)sums[1]=32768;
		output[0] = sums[0];
		output[1] = sums[1];
	}
}
