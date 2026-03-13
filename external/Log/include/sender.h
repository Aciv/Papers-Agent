#pragma once

#include <mutex>
#include <string_view>
#include "level.h"

namespace Aciv::utility{
    class Sender {
        private:
        
        public:

            virtual ~Sender() = default;
            virtual void send(std::string_view _message) = 0;

            void set_level_filter(level _level_filter);
            bool should_sink(level log_level);

        private:
            std::mutex m_mtx;   
            level m_level{ level::all };
            level m_level_filter{ level::all };
            
    };
}