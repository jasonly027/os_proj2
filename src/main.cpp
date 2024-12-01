#include <vcruntime_typeinfo.h>

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <utility>

#include "page_table.hpp"
#include "reference_generator.hpp"

using proj2::FifoTable, proj2::LruTable, proj2::OptTable,
    proj2::ReferenceGenerator;
using std::puts, std::pair, std::string_view, std::ifstream, std::string,
    std::getline;

int main() {
    // Part 1b
    puts("Part 1b");

    constexpr pair<int, string_view> kTestCases[] = {
        {3, "70120304230321201701"},
        {4, "453254671"},
        {5, "135732345051740"},
        {6, "104780054214734"}};

    for (const auto [frame_size, ref_str] : kTestCases) {
        printf("\nFrame Size: %d | Reference String: %s\n", frame_size,
               ref_str.data());
        // Print faults on this ref str for each algorithm
        [&](auto... table) {
            (
                [&] {
                    for (const auto& page : ref_str) {
                        table.get(page - '0');
                    }

                    printf("%s: %zu faults\n", typeid(table).name(),
                           table.faults());
                }(),
                ...);
        }(FifoTable(frame_size), LruTable(frame_size),
          OptTable(frame_size, ref_str));
    }

    // Part 3a
    puts("\nPart 3a");

    constexpr auto kPath = "TestingData.txt";
    ReferenceGenerator gen;
    constexpr auto kRefStrs = 50;
    gen.write(kPath);

    for (const auto frame_size : {3, 4, 5, 6}) {
        printf("\nPage Frame: %d\n", frame_size);

        double avg_fifo_faults = 0;
        double avg_lru_faults = 0;
        double avg_opt_faults = 0;

        ifstream ifs(kPath);
        if (!ifs) return EXIT_FAILURE;

        string ref_str;
        while (getline(ifs, ref_str)) {
            FifoTable fifo_table(frame_size);
            for (const auto& page : ref_str) {
                fifo_table.get(page);
            }
            avg_fifo_faults += fifo_table.faults();

            LruTable lru_table(frame_size);
            for (const auto& page : ref_str) {
                lru_table.get(page);
            }
            avg_lru_faults += lru_table.faults();

            OptTable opt_table(frame_size, ref_str);
            for (const auto& page : ref_str) {
                opt_table.get(page);
            }
            avg_opt_faults += opt_table.faults();
        }
        avg_fifo_faults /= kRefStrs;
        avg_lru_faults /= kRefStrs;
        avg_opt_faults /= kRefStrs;

        printf("FIFO avg faults: %f\n", avg_fifo_faults);
        printf("LRU avg faults: %f\n", avg_lru_faults);
        printf("OPT avg faults: %f\n", avg_opt_faults);
    }

    return 0;
}
