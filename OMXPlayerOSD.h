#pragma once

/*
 * Author: Quentin 'acieroid' Stievenart (2012)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "OMXThread.h"
#include "OMXReader.h"
#include "OMXClock.h"
#include "OMXOverlayCodecText.h"

#include <boost/config.hpp>
#include <boost/circular_buffer.hpp>
#include <atomic>
#include <string>
#include <deque>
#include <mutex>

class OMXPlayerOSD : public OMXThread
{
public:
  OMXPlayerOSD(const OMXPlayerOSD&) = delete;
  OMXPlayerOSD& operator=(const OMXPlayerOSD&) = delete;
  OMXPlayerOSD() BOOST_NOEXCEPT;
  ~OMXPlayerOSD() BOOST_NOEXCEPT;
  bool Open(const std::string& font_path, float font_size, OMXClock* clock, int total_time) BOOST_NOEXCEPT;
  void Close() BOOST_NOEXCEPT;
  void Flush() BOOST_NOEXCEPT;
  void Toggle() BOOST_NOEXCEPT;
  bool AddPacket(OMXPacket *pkt) BOOST_NOEXCEPT;

private:

  void Process();
  void RenderLoop(const std::string& font_path, float font_size, OMXClock* clock);
  std::vector<std::string> GetTextLines(OMXPacket *pkt);

  COMXOverlayCodecText                   m_osd_codec;
  std::atomic<bool>                      m_thread_stopped;
  std::atomic<bool>                      m_flush;
  std::string                            m_font_path;
  float                                  m_font_size;
  OMXClock*                              m_av_clock;
  int                                    m_total_time;
  bool                                   m_show;
  OMXReader*                             m_omx_reader;
};

