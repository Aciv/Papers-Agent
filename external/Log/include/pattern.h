#pragma once
#include <string_view>
#include <string>
#include <vector>
#include <cstring>
#include <chrono>
#include <ctime>

#include "buffer.h"
#include "level.h"



namespace Aciv::utility{


    template <std::size_t PATTERN_LIMIT>
    class Pattern{
        private:
            constexpr static char week[][5] = {
                {"Sun."},
                {"Mon."},
                {"Tue."},
                {"Wes."},
                {"Thu."},
                {"Fri."},
                {"Sat."}
            };

        public:
            void genrate(level _level, const char* const _message, 
                    Buffer &_buffer);
            void set_pattern(std::string_view _pattern);
            void set_delimiter(std::string_view _delimiter);
        //private:
            char m_pattern[PATTERN_LIMIT];
            std::size_t m_pattern_len{0};
            std::vector<std::pair<int, char>> m_pattern_flag;
            std::string m_delimiter;
    };

    /*
        l -> level
        v -> message
        y -> year
        m -> month
        d -> day
        w -> weekday
        T -> year/month/day hours:minte:second
        t -> hours:minte:second
    */




    template <std::size_t PATTERN_LIMIT>
    void Pattern<PATTERN_LIMIT>::genrate(level _level, const char* const _message,
                        Buffer &_buffer){
        
        _buffer.append(m_pattern, m_pattern_len);

        const char* handle_string = nullptr;
        int string_len = 0;
        int change_size = 0;

        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        std::tm* local_tm = std::localtime(&time_t);
        
        char time_info[20] = {0};
        snprintf(time_info, 20, "%04d/%02d/%02d %02d:%02d:%02d", 
                local_tm->tm_year + 1900, local_tm->tm_mon + 1, local_tm->tm_mday,
                local_tm->tm_hour, local_tm->tm_min, local_tm->tm_sec);

        const char *week_info = week[local_tm->tm_wday];
        

        for(auto flag_pair : m_pattern_flag){
            
            handle_string = nullptr;
            string_len = 0;

            // sample ignores unknow flag
            switch (flag_pair.second)
            {
            case 'l':
                handle_string = to_string(_level);
                
                if(handle_string != nullptr){
                    string_len = strlen(handle_string);
                }
                else
                    string_len = 0;

                break;
            case 'v':
                handle_string = _message;
                string_len = strlen(handle_string);
                break;
            case 'y': 
                handle_string = time_info;
                string_len = 4;
                break; 
            case 'm': 
                handle_string = time_info + 5;
                string_len = 2;
                break;
            case 'd': 
                handle_string = time_info + 8;
                string_len = 2;
                break;
            case 'w':
                handle_string = week_info;
                string_len = 4;
                break;
            case 'T': 
                handle_string = time_info;
                string_len = 19;
                break;
            case 't': 
                handle_string = time_info + 11;
                string_len = 8;
                break; 
            }
            
            if(handle_string != nullptr && string_len != 0){
                _buffer.replace(flag_pair.first + change_size, 2, handle_string, 
                                                    string_len);

                
            }
            else{
                _buffer.erase(flag_pair.first + change_size, 2);
            }
            change_size += string_len - 2;
            

        }

    }

    template <std::size_t PATTERN_LIMIT>
    void Pattern<PATTERN_LIMIT>::set_pattern(std::string_view _pattern){
        if(_pattern.size() >= PATTERN_LIMIT)
            return;
        
        m_pattern_len = _pattern.size();

        std::memcpy((void *)m_pattern, (void *)_pattern.data(), _pattern.size());

        for(std::size_t i = 0; i < _pattern.size(); i++){
            if(_pattern[i] == '%' && i + 1 < _pattern.size()){

                m_pattern_flag.push_back({i, _pattern[i+1]});
                ++i;
            }
        }
    }

    template <std::size_t PATTERN_LIMIT>
    void Pattern<PATTERN_LIMIT>::set_delimiter(std::string_view _delimiter){
        m_delimiter = _delimiter;
    }
}