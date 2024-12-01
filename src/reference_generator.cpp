#include "reference_generator.hpp"

#include <fstream>
#include <stdexcept>

using std::random_device, std::ofstream, std::runtime_error;

namespace proj2 {
ReferenceGenerator::ReferenceGenerator()
    : rand_(0, 8), gen_(random_device{}()) {}

bool ReferenceGenerator::write(std::string_view path, ref_str_size size,
                               int num_strs) {
    ofstream ofs(path.data());
    if (!ofs) throw runtime_error("Failed to open file");

    for (int i = 0; i < num_strs; ++i) {
        ofs << generate(size) << '\n';
    }

    if (!ofs) throw runtime_error("Failed to write dataset");
    return ofs.fail();
}

auto ReferenceGenerator::generate(ref_str_size size) -> string {
    string res;
    res.reserve(size);

    for (ref_str_size i = 0; i < size; ++i) {
        res += static_cast<char>('0' + rand_(gen_));
    }

    return res;
}
}  // namespace proj2
