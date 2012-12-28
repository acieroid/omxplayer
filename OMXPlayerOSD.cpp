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

#include "OMXPlayerOSD.h"
#include "OMXOverlayText.h"
#include "SubtitleRenderer.h"
#include "utils/LockBlock.h"
#include "utils/Enforce.h"
#include "utils/log.h"

#include <boost/algorithm/string.hpp>
#include <typeinfo>
#include <iomanip>
#include <sstream>

constexpr int RENDER_LOOP_SLEEP = 500;

OMXPlayerOSD::OMXPlayerOSD() BOOST_NOEXCEPT
:
  m_thread_stopped(true),
  m_flush(),
  m_font_size(),
  m_av_clock(),
  m_total_time(),
  m_show(false)
{}

OMXPlayerOSD::~OMXPlayerOSD() BOOST_NOEXCEPT
{
  Close();
}

bool OMXPlayerOSD::
Open(const std::string& font_path, float font_size, OMXClock* clock, int total_time) BOOST_NOEXCEPT
{
  m_thread_stopped.store(false, std::memory_order_relaxed);
  m_flush.store(false, std::memory_order_relaxed);
  m_font_path = font_path;
  m_font_size = font_size;
  m_av_clock  = clock;
  m_total_time    = total_time;

  if(!Create())
    return false;

  return true;
}

void OMXPlayerOSD::Close() BOOST_NOEXCEPT
{
  if(Running())
    StopThread();
}

void OMXPlayerOSD::Process()
{
  try
  {
    RenderLoop(m_font_path, m_font_size, m_av_clock);
  }
  catch(Enforce_error& e)
  {
    if(!e.user_friendly_what().empty())
      printf("Error: %s\n", e.user_friendly_what().c_str());
    CLog::Log(LOGERROR, "OMXPlayerOSD::RenderLoop threw %s (%s)",
              typeid(e).name(), e.what());
  }
  catch(std::exception& e)
  {
    CLog::Log(LOGERROR, "OMXPlayerOSD::RenderLoop threw %s (%s)",
              typeid(e).name(), e.what());
  }
  m_thread_stopped.store(true, std::memory_order_relaxed);
}

void OMXPlayerOSD::
RenderLoop(const std::string& font_path, float font_size, OMXClock* clock)
{
  bool centered = false;
  int lines = 1;
  SubtitleRenderer renderer(1,
                            font_path,
                            font_size,
                            0.00f, 0.8f,
                            centered,
                            0xDD,
                            0x80,
                            lines);
  int total_sec = m_total_time/1000;
  int total_min = total_sec/60;
  int total_hours = total_min/60;

  while(!m_bStop)
  {
    int now = clock->OMXMediaTime()/1000;
    int sec = now/1000;
    int min = sec/60;
    int hours = min/60;

    if (m_show) {
      std::ostringstream stream;
      std::vector<std::string> vec;
      stream << hours << ":"
             << std::setw(2) << std::setfill('0') << min % 60 << ":"
             << std::setw(2) << std::setfill('0') << sec % 60 << "/"
             << total_hours << ":"
             << std::setw(2) << std::setfill('0') << total_min % 60 << ":"
             << std::setw(2) << std::setfill('0') << total_sec % 60;
      vec.push_back(stream.str());
      renderer.prepare(vec);
      renderer.show_next();
    } else {
      renderer.unprepare();
      renderer.hide();
    }

    OMXClock::OMXSleep(RENDER_LOOP_SLEEP);
  }
}

void OMXPlayerOSD::Flush() BOOST_NOEXCEPT
{
  m_flush.store(true, std::memory_order_release);
}

void OMXPlayerOSD::Toggle() BOOST_NOEXCEPT
{
  m_show = !m_show;
}
