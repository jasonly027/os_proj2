#pragma once

#include <random>
#include <string>
#include <string_view>

namespace proj2 {
class ReferenceGenerator {
   public:
    using ref_str_size = std::string::size_type;
    using string = std::string;

    ReferenceGenerator();

    bool write(std::string_view path, ref_str_size size = 30,
               int num_strs = 50);

    string generate(ref_str_size size);

   private:
    std::uniform_int_distribution<> rand_;
    std::mt19937 gen_;
};
}  // namespace proj2
