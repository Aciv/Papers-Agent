#pragma once

#include "sender.h"
#include <filesystem>

namespace Aciv::utility{
    class File_sender: public Sender {
        private:
        
        public:
            File_sender(std::string_view _file_path);
            ~File_sender() override;
            void send(std::string_view _message) override;

        private:
              std::filesystem::path m_file_path;
              std::FILE* m_stream{ nullptr };
    };
}