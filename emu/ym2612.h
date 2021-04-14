// license:BSD-3-Clause
// copyright-holders:Aaron Giles

#ifndef MAME_SOUND_YM2612_H
#define MAME_SOUND_YM2612_H

#pragma once

#include "weird-mame-types.h"
#include "ymfm.h"


class ym2612_device
{
public:
	// YM2612 is OPNA
	using fm_engine = ymopna_engine;

	// constructor
	ym2612_device(uint32_t clock);

	// read/write access
	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);

	// defle custom
	void set_mute(int mask, int mask_ch3);
	float get_volume(int channel);

	// device-level s
	virtual void device_start();
	virtual void device_reset();
	virtual void device_clock_changed();

	// internal helpers
	void sound_stream_update_common(int *output, bool discontinuity);

	// internal state
	fm_engine m_fm;                  // core FM engine
	u16 m_address;                   // address register
	u16 m_dac_data;                  // 9-bit DAC data
	u8 m_dac_enable;                 // DAC enabled?
	u8 m_channel;                    // current multiplexed channel
};


class ym3438_device : public ym2612_device
{
public:
	// constructor
	ym3438_device(uint32_t clock);
};


class ymf276_device : public ym2612_device
{
public:
	// constructor
	ymf276_device(uint32_t clock);

protected:
	// device-level s
	virtual void device_clock_changed() ;
};


#endif // MAME_SOUND_YM2612_H
