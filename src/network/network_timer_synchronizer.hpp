//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2018 SuperTuxKart-Team
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 3
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#ifndef HEADER_NETWORK_TIMER_SYNCHRONIZER_HPP
#define HEADER_NETWORK_TIMER_SYNCHRONIZER_HPP

#include "config/user_config.hpp"
#include "utils/log.hpp"
#include "utils/time.hpp"
#include "utils/types.hpp"
#include "main_loop.hpp"

#include <cstdlib>
#include <deque>
#include <numeric>
#include <tuple>

class NetworkTimerSynchronizer
{
private:
    std::deque<std::tuple<uint32_t, uint64_t, uint64_t> > m_times;

    bool m_synchronised = false;
public:
    // ------------------------------------------------------------------------
    bool isSynchronised() const                      { return m_synchronised; }
    // ------------------------------------------------------------------------
    void addAndSetTime(uint32_t ping, uint64_t server_time)
    {
        if (m_synchronised)
            return;

        const uint64_t cur_time = StkTime::getRealTimeMs();
        // Take max 20 averaged samples from m_times, the next addAndGetTime
        // is used to determine that server_time if it's correct, if not
        // clear half in m_times until it's correct
        if (m_times.size() >= 20)
        {
            uint64_t sum = std::accumulate(m_times.begin(), m_times.end(),
                (uint64_t)0, [cur_time](const uint64_t previous,
                const std::tuple<uint32_t, uint64_t, uint64_t>& b)->uint64_t
                {
                    return previous + (uint64_t)(std::get<0>(b) / 2) +
                        std::get<1>(b) + cur_time - std::get<2>(b);
                });
            const int64_t averaged_time = sum / 20;
            const int64_t server_time_now = server_time + (uint64_t)(ping / 2);
            int difference = (int)std::abs(averaged_time - server_time_now);
            if (std::abs(averaged_time - server_time_now) <
                UserConfigParams::m_timer_sync_tolerance)
            {
                main_loop->setNetworkTimer(averaged_time);
                m_synchronised = true;
                Log::info("NetworkTimerSynchronizer", "Network "
                    "timer synchronized, difference: %dms", difference);
                return;
            }
            m_times.erase(m_times.begin(), m_times.begin() + 10);
        }
        m_times.emplace_back(ping, server_time, cur_time);
    }
};

#endif // HEADER_NETWORK_TIMER_SYNCHRONIZER_HPP
