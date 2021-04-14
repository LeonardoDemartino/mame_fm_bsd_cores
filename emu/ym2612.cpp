// license:BSD-3-Clause
// copyright-holders:Aaron Giles

#include "weird-mame-types.h"
#include "ym2612.h"

// the YM2612/YM3438 just timeslice the output among all channels
// instead of summing them; turn this on to simulate (may create
// audible issues)
#define MULTIPLEX_OUTPUT (0)

//*********************************************************
//  YM2612 DEVICE
//*********************************************************

//-------------------------------------------------
//  ym2612_device - constructor
//-------------------------------------------------

ym2612_device::ym2612_device(uint32_t clock) :
	m_fm(),
	m_address(0),
	m_dac_data(0),
	m_dac_enable(0),
	m_channel(0)
{
}


//-------------------------------------------------
//  read - handle a read from the device
//-------------------------------------------------

u8 ym2612_device::read(offs_t offset)
{
	u8 result = 0;
	switch (offset & 3)
	{
		case 0:	// status port, YM2203 compatible
			result = m_fm.status();
			break;

		case 1: // data port (unused)
		case 2: // status port, extended
		case 3: // data port (unused)
			printf("Unexpected read from YM2612 offset %d\n", offset & 3);
			break;
	}
	return result;
}

// defle custom
void ym2612_device::set_mute(int mask, int mask_ch3)
{
	mute_mask = mask;
	mute_mask_special_operators = mask_ch3;
}

float ym2612_device::get_volume(int channel)
{
	return (float) abs(channelsOutput[channel]) / 10000.0f;
}

//-------------------------------------------------
//  write - handle a write to the register
//  interface
//-------------------------------------------------

void ym2612_device::write(offs_t offset, u8 value)
{
	switch (offset & 3)
	{
		case 0: // address port
			m_address = value;
			break;

		case 1: // data port

			// ignore if paired with upper address
			if (BIT(m_address, 8))
				break;

			if (m_address == 0x2a)
			{
				// DAC data
				m_dac_data = (m_dac_data & ~0x1fe) | ((value ^ 0x80) << 1);
			}
			else if (m_address == 0x2b)
			{
				// DAC enable
				m_dac_enable = BIT(value, 7);
			}
			else if (m_address == 0x2c)
			{
				// test/low DAC bit
				m_dac_data = (m_dac_data & ~1) | BIT(value, 3);
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

			// write to FM
			m_fm.write(m_address, value);
			break;
	}
}


//-------------------------------------------------
//  device_start - start of emulation
//-------------------------------------------------

void ym2612_device::device_start()
{
	// call this for the variants that need to adjust the rate
	device_clock_changed();

	// save our data
	/*save_item(YMFM_NAME(m_address));
	save_item(YMFM_NAME(m_dac_data));
	save_item(YMFM_NAME(m_dac_enable));
	save_item(YMFM_NAME(m_channel));

	// save the engines
	m_fm.save(*this);*/
}


//-------------------------------------------------
//  device_reset - start of emulation
//-------------------------------------------------

void ym2612_device::device_reset()
{
	// reset the engines
	m_fm.reset();

	// reset our internal state
	m_dac_enable = 0;
	m_channel = 0;
}


//-------------------------------------------------
//  device_clock_changed - update if clock changes
//-------------------------------------------------

void ym2612_device::device_clock_changed()
{
}


//-------------------------------------------------
//  sound_stream_update_common - shared stream
//  update function among subclasses
//-------------------------------------------------

void ym2612_device::sound_stream_update_common(int *output, bool discontinuity)
{
	u32 const sample_divider = (discontinuity ? 260 : 256) * (MULTIPLEX_OUTPUT ? 1 : fm_engine::CHANNELS);

	// iterate over all target samples
	s32 sums[2] = { 0 };
	for (int sampindex = 0; sampindex < 1; )
	{
		// clock the FM when we hit channel 0
		if (m_channel == 0)
			m_fm.clock(fm_engine::ALL_CHANNELS);

		// update the current FM channel; YM2612 is 9-bit with intermediate clipping
		s32 outputs[2] = { 0 };
		if (m_channel != 5 || !m_dac_enable)
			m_fm.output(outputs, 5, 256, 1 << m_channel, mute_mask, 2, mute_mask_special_operators);
		else
			outputs[0] = outputs[1] = s16(m_dac_data << 7) >> 7;

		// hiccup in the internal YM2612 DAC means that there is a rather large
		// step between 0 and -1 (close to 6x the normal step); the approximation
		// below gives a reasonable estimation of this discontinuity, which was
		// fixed in the YM3438
		if (discontinuity)
		{
			if (outputs[0] < 0)
				outputs[0] -= 2;
			else
				outputs[0] += 3;
			if (outputs[1] < 0)
				outputs[1] -= 2;
			else
				outputs[1] += 3;
		}

		// if multiplexing, just scale to 16 bits and output
		if (MULTIPLEX_OUTPUT)
		{
			output[0] = outputs[0];
			output[1] = outputs[1];
			sampindex++;
		}

		// if not, accumulate the sums
		else
		{
			sums[0] += outputs[0];
			sums[1] += outputs[1];

			// on the last channel, output the average and reset the sums
			if (m_channel == 5)
			{
				output[0] = sums[0];
				output[1] = sums[1];
				sampindex++;
				sums[0] = sums[1] = 0;
			}
		}

		// advance to the next channel
		m_channel++;
		if (m_channel >= 6)
			m_channel = 0;
	}
}



//*********************************************************
//  YM3438 DEVICE
//*********************************************************

//-------------------------------------------------
//  ym3438_device - constructor
//-------------------------------------------------

ym3438_device::ym3438_device(uint32_t clock) :
	ym2612_device(clock)
{
}


//*********************************************************
//  YMF276 DEVICE
//*********************************************************

//-------------------------------------------------
//  ymf276_device - constructor
//-------------------------------------------------

ymf276_device::ymf276_device(uint32_t clock) :
	ym2612_device(clock)
{
}


//-------------------------------------------------
//  device_clock_changed - update if clock changes
//-------------------------------------------------

void ymf276_device::device_clock_changed()
{
}