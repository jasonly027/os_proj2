#pragma once

#include <cstddef>
#include <cstdint>
#include <list>
#include <queue>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace proj2 {

using page_id = int32_t;

struct Page {
    page_id id;
};

// CRTP is used for compile-time polymorphism
template <typename ReplacementAlg>
class PageTable {
   public:
    using fault = std::size_t;

    /* Get a page specified by its id. If the page is not
       found in the page frame, it will fetch the page from
       the TLB or secondary storage, removing a different
       page if the frame is full
    */
    Page& get(page_id id) {
        return static_cast<ReplacementAlg*>(this)->get_impl(id);
    }

    // Get how many pages are in the page frame
    auto size() const {
        return static_cast<const ReplacementAlg*>(this)->size_impl();
    }

    // Get if the page frame is full
    bool full() const {
        return static_cast<const ReplacementAlg*>(this)->full_impl();
    }

    // Get how many page faults have occured
    fault faults() const {
        return static_cast<const ReplacementAlg*>(this)->faults_impl();
    }

   protected:
    // Hidden because it should be instantiated through its subclasses
    PageTable() = default;

    // Simulated call of retrieving the page from TLB or secondary storage
    inline constexpr Page fetch_page(page_id id) { return Page{id}; }
};

class FifoTable : public PageTable<FifoTable> {
   public:
    /* A queue is used to determine the first page that came,
       because that will be the first page to have to leave
       when the frame is full and a new page is needed
    */
    using queue = std::queue<page_id>;
    // Address translation is simulated by mapping page_id with page
    using frame = std::unordered_map<page_id, Page>;
    using frame_size = frame::size_type;

    explicit FifoTable(frame_size frame_size);

   private:
    friend PageTable<FifoTable>;

    Page& get_impl(page_id id);

    inline frame_size size_impl() const noexcept { return frame_.size(); }

    inline bool full_impl() const noexcept { return size_impl() >= capacity_; }

    inline fault faults_impl() const noexcept { return faults_; }

    void add_page(page_id id);

    void remove_page() noexcept;

    frame_size capacity_;
    fault faults_ = 0;
    queue queue_;
    frame frame_;
};

class LruTable : public PageTable<LruTable> {
   public:
    /* A linked list is used to contain the pages because
       quick node reordering is needed to move accessed pages
       to the end of the list.

       The front of the list is least recently used.
       The end of the list is most recently used.
    */
    using list = std::list<Page>;
    // Address translation is simulated by mapping page id
    // to a reference to the node in the list containing the page
    using frame = std::unordered_map<page_id, list::iterator>;
    using frame_size = frame::size_type;

    explicit LruTable(frame_size frame_size);

   private:
    friend PageTable<LruTable>;

    Page& get_impl(page_id id);

    inline frame_size size_impl() const noexcept { return frame_.size(); }

    inline bool full_impl() const noexcept { return size_impl() >= capacity_; }

    inline fault faults_impl() const noexcept { return faults_; }

    void add_page(page_id id);

    void remove_page() noexcept;

    frame_size capacity_;
    fault faults_ = 0;
    list list_;
    frame frame_;
};

class OptTable : public PageTable<OptTable> {
   public:
    using request = std::string::size_type;
    using request_idxs = std::vector<request>;

    /* A map is used to access a page's list. A list contains
       the associated page's indices in the reference string when
       it will be requested
    */
    using future = std::unordered_map<page_id, request_idxs>;
    // Address translation is simulated by mapping page id with page
    using frame = std::unordered_map<page_id, Page>;
    using frame_size = frame::size_type;

    // Takes in the reference str to populate the future map
    OptTable(frame_size frame_size, std::string_view ref_str);

   private:
    friend PageTable<OptTable>;

    Page& get_impl(page_id id);

    inline frame_size size_impl() const noexcept { return frame_.size(); }

    inline bool full_impl() const noexcept { return size_impl() >= capacity_; }

    inline fault faults_impl() const noexcept { return faults_; }

    void add_page(page_id id);

    void remove_page();

    /* For a given page's request list, calculate the distance from the current
       request to this page's next request.

       If this page isn't requested again in the future. A supremely large
       distance is returned.
    */
    request next_request_distance(const request_idxs& idxs) const;

    frame_size capacity_;
    fault faults_ = 0;
    // Expected number of get() calls aka the length of the ref str
    request max_requests_;
    // Current request # being processed
    request request_ = 0;
    future future_;
    frame frame_;
};

}  // namespace proj2
