// license:BSD-3-Clause
// copyright-holders:Aaron Giles

#ifndef MAME_SOUND_YM2151_H
#define MAME_SOUND_YM2151_H

#pragma once

#include "ymfm.h"
#include "weird-mame-types.h"

class ym2151_device
{
public:
	// YM2151 is OPM
	using fm_engine = ymopm_engine;

	// constructor
	ym2151_device(uint32_t clock);

	// read/write access
	u8 read(offs_t offset);
	virtual void write(offs_t offset, u8 data);

	u8 status_r() { return read(1); }
	void register_w(u8 data) { write(0, data); }
	void data_w(u8 data) { write(1, data); }

	// defle custom
	void set_mute(int mask);
	float get_volume(int channel);
	// device-level s
	virtual void device_start() ;
	virtual void device_reset() ;
	virtual void device_clock_changed() ;

	void sound_stream_update_common(int *output);

	// internal state
	fm_engine m_fm;                  // core FM engine
	u8 m_address;                    // address register
	u8 m_reset_state;                // reset state
};

class ym2164_device : public ym2151_device
{
public:
	// constructor
	ym2164_device(uint32_t clock);

	// read/write access
	virtual void write(offs_t offset, u8 data) ;
};

class ym2414_device : public ym2151_device
{
public:
	// constructor
	ym2414_device(uint32_t clock);
};


#endif // MAME_SOUND_YM2151_H
