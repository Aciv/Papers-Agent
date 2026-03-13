#pragma once

#include "blockqueue.h"
#include "pattern.h"
#include "sender.h"
#include <vector>

namespace Aciv::utility{


    class Log {
        private:
            Log();
            virtual ~Log();

            Log(const Log&) = delete;
            Log& operator=(const Log&) = delete;

        public:
            struct Queue_item{
                level log_level;
                std::string log_msg;
            };

            void init(std::size_t _max_queue_size = 1024,
                            std::size_t _message_limit = 512);

            void set_pattern(std::string_view _pattern);


            void record(level _level, const std::string &_log_msg);
            void flush();
            void write_func();
            
            void add_sender(std::unique_ptr<Sender> _sender);

            static Log& get_Instance(){
                static Log instance;
                return instance;
            }

            std::vector<std::unique_ptr<Sender>> m_senders; 
        private:

            mutable std::mutex m_mtx;
            
            bool m_inited{false};

            Buffer m_buffer{};
            std::unique_ptr<BlockDeque<Queue_item>> m_queue;

            bool m_is_Async{};
            std::unique_ptr<std::thread> m_write_thread;
            
            std::size_t m_message_limit{512};
            Pattern<64> m_pattern;
  
    };
}