#pragma once

#include <cstddef>
#include <atomic>
#include <string_view>
#include <string>
#include <memory>
#include <cstring>
#include <assert.h>

namespace Aciv::utility{

    class Buffer{
        private:
            void adapt_len(std::size_t _len);
            int resize(std::size_t _update_size);

            char* buffer_start_ptr() const;
            
            char* read_end_ptr() const;
            char* read_start_ptr() const;

            std::size_t writably_len_at_end() const;
            std::size_t writably_len_at_start() const;
            std::size_t readably_len() const;

            void erase(char* _start, size_t _len);
        public:
            static constexpr std::size_t cache_line_size{ 64 };
            
            Buffer(std::size_t _init_size = 1024);
            ~Buffer();
            
            std::size_t capacity() const { return m_capacity; }
            std::size_t size() const { return readably_len(); }
            void reset();


            // danger function 
            const char* read(std::size_t _len);


            void append(const char* _in_data, std::size_t _data_size);
            void append(std::string_view _in_data);
            void append(char _in_data);
            void append(Buffer &_in_buffer);
            
            void erase_start(std::size_t _len);
            void erase_end(std::size_t _len);

            void erase(std::size_t pos, std::size_t _len);

            void insert(std::size_t _insert_pos, const char* const __restrict _in_data, std::size_t _data_size);
            void insert(std::size_t _insert_pos, std::string_view _in_data);
            void insert(std::size_t _insert_pos, char _in_char);
            
            void replace(std::size_t _start_pos, size_t _replace_len,
                         const char* const __restrict _replace_data, std::size_t _replace_data_size);


            std::string retrieve_to_string(std::size_t _len);

            
        private:
            char* m_data{};
            std::size_t m_capacity{};
            
            alignas(cache_line_size) std::atomic<std::size_t> m_ReadPos{};
            alignas(cache_line_size) std::atomic<std::size_t> m_WritePos{};
    };

}