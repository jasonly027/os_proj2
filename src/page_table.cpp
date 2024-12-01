#include "page_table.hpp"

#include <algorithm>
#include <cassert>
#include <limits>

using std::string_view, std::numeric_limits;

namespace proj2 {
FifoTable::FifoTable(frame_size frame_size) : capacity_(frame_size) {
    assert(frame_size > 0 && "frame_size must be greater than 0");
}

Page& FifoTable::get_impl(page_id id) {
    // Return the page if it is already in the frame
    // else, add it to the frame first.
    if (auto it = frame_.find(id); it != frame_.end()) return it->second;

    ++faults_;
    add_page(id);
    return frame_[id];
}

void FifoTable::add_page(page_id id) {
    if (full_impl()) remove_page();

    frame_[id] = fetch_page(id);
    queue_.push(id);
}

void FifoTable::remove_page() noexcept {
    assert(!queue_.empty() && "remove_page called on an empty container");

    // Remove the first page in the queue
    page_id id = queue_.front();
    queue_.pop();
    frame_.erase(id);
}

LruTable::LruTable(frame_size frame_size) : capacity_(frame_size) {
    assert(frame_size > 0 && "frame_size must be greater than 0");
}

Page& LruTable::get_impl(page_id id) {
    /* If the page is already in the frame,
       move its node to the end (the end represents the most
       recently used page) and return the page.

      Otherwise, add the page first.
    */
    if (auto it = frame_.find(id); it != frame_.end()) {
        list_.splice(list_.end(), list_, it->second);
        return *it->second;
    }

    ++faults_;
    add_page(id);
    return *frame_[id];
}

void LruTable::add_page(page_id id) {
    if (full_impl()) remove_page();

    // Fetch the page and append it to the end of the list
    frame_[id] = list_.insert(list_.end(), fetch_page(id));
}

void LruTable::remove_page() noexcept {
    assert(!list_.empty() && "remove_page called on an empty container");

    // Remove the first page in the list (least recently used)
    frame_.erase(list_.front().id);
    list_.pop_front();
}

OptTable::OptTable(frame_size frame_size, string_view ref_str)
    : capacity_(frame_size), max_requests_(ref_str.size()) {
    for (std::string::size_type i = 0; i < ref_str.size(); ++i) {
        future_[ref_str[i] - '0'].push_back(i);
    }
}

Page& OptTable::get_impl(page_id id) {
    assert(request_ < max_requests_ &&
           "get() was called more than the foreseen amount");

    // Return the page if it is already in the frame
    // else, add it to the frame first.
    if (auto it = frame_.find(id); it != frame_.end()) {
        ++request_;
        return it->second;
    }

    ++faults_;
    add_page(id);
    ++request_;
    return frame_[id];
}

void OptTable::add_page(page_id id) {
    if (full_impl()) remove_page();

    frame_[id] = fetch_page(id);
}

void OptTable::remove_page() {
    assert(!frame_.empty() && "remove_page called on an empty container");

    /* Find the page in frame that's next request is furthest from now
       and remove it.

       Early returns are done if a page in frame is never requested again.
    */

    page_id farthest_page_id = frame_.cbegin()->first;
    request farthest_req = next_request_distance(future_[farthest_page_id]);

    if (farthest_req == numeric_limits<request>::max()) {
        frame_.erase(farthest_page_id);
        return;
    }

    for (const auto& [id, page] : frame_) {
        if (request dist = next_request_distance(future_[id]);
            dist > farthest_req) {
            farthest_page_id = id;
            farthest_req = dist;

            if (farthest_req == numeric_limits<request>::max()) break;
        }
    }
    frame_.erase(farthest_page_id);
}

auto OptTable::next_request_distance(const request_idxs& idxs) const
    -> request {
    // Binary search for the first request of this page that is
    // after the current request
    auto upper = std::upper_bound(idxs.cbegin(), idxs.cend(), request_);
    if (upper == idxs.cend()) return numeric_limits<request>::max();

    return *upper - request_;
}

}  // namespace proj2
