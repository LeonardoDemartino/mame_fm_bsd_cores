// license:BSD-3-Clause
// copyright-holders:Aaron Giles

#ifndef MAME_SOUND_YM2610_H
#define MAME_SOUND_YM2610_H

#pragma once

#include "weird-mame-types.h"
#include "ymfm.h"
#include "ymadpcm.h"
#include "ay8910.h"

class ym2610_device : public ay8910_device
{
public:
	// YM2610 is OPNA
	using fm_engine = ymopna_engine;

	// constructor
	ym2610_device(uint32_t clock, u8 fm_mask = 0x36);

	// read/write access
	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);

	// defle custom
	void set_mute(int mask, int mask_ch3);
	float get_volume(int channel);

	// device-level s
	virtual void device_start() ;
	virtual void device_reset() ;
	virtual void device_clock_changed() ;

	void sound_stream_update_common(int *output);
	void sound_stream_update_common_ssg(int *output);

	void set_adpcma_memory(unsigned char *ptr, unsigned int size);
private:
	// ADPCM read/write callbacks
	u8 adpcm_a_read(offs_t address);
	u8 adpcm_b_read(offs_t address);

	// internal state
	fm_engine m_fm;                  // core FM engine
	ymadpcm_a_engine m_adpcm_a;      // ADPCM-A engine
	ymadpcm_b_engine m_adpcm_b;      // ADPCM-B engine
	u16 m_address;                   // address register
	u8 const m_fm_mask;              // FM channel mask
	u8 m_eos_status;                 // end-of-sample signals
	u8 m_flag_mask;                  // flag mask control
};

class ym2610b_device : public ym2610_device
{
public:
	// constructor
	ym2610b_device(uint32_t clock);
};


#endif // MAME_SOUND_YM2610_H
