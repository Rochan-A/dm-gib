#pragma once

#include <atomic>
#include <memory>

namespace gib {

// Lock-free single producer-single consumer triple buffer.
template <typename DataType> class LockFreeTripleBuffer {
public:
  // All buffers will be default initialized.
  LockFreeTripleBuffer() = default;

  // Initializes front buffer with given data, other buffers are default
  // initialized
  LockFreeTripleBuffer(DataType &&initial_front_buf)
      : buffers_{{}, {}, {std::forward<DataType>(initial_front_buf)}} {}

  // Initializes all 3 buffers
  LockFreeTripleBuffer(DataType &&initial_back_buf,
                       DataType &&initial_middle_buf,
                       DataType &&initial_front_buf)
      : buffers_{{std::forward<DataType>(initial_back_buf)},
                 {std::forward<DataType>(initial_middle_buf)},
                 {std::forward<DataType>(initial_front_buf)}} {}

  // Returns reference to the latest front buffer.
  std::pair<DataType &, bool> Read() {
    // If there's a new buffer recently written -> will swap buffers and return
    // the latest value. Otherwise, will return stale value.
    uintptr_t dirty_ptr = middle_buffer_.load(std::memory_order_relaxed);
    if ((dirty_ptr & kDirtyBit) == 0) {
      return {*front_buffer_, false};
    }
    uintptr_t prev = middle_buffer_.exchange(
        reinterpret_cast<uintptr_t>(front_buffer_), std::memory_order_acq_rel);
    front_buffer_ = reinterpret_cast<DataType *>(prev & kDirtyBitMask);
    return {*front_buffer_, true};
  }

  // Returns reference to back buffer, producer can use it to fill the buffer.
  DataType &Write() {
    // Once finished filling the buffer Commit() must be called to propagate
    // changes to the front buffer.
    return *back_buffer_;
  }

  // Propagates pending changes from the back buffer to the middle buffer.
  void Commit() {
    // The next read will get those changes unless the writer is faster and will
    // overwrite those changes before the next Read() call.
    const uintptr_t dirty_ptr =
        kDirtyBit | reinterpret_cast<uintptr_t>(back_buffer_);
    uintptr_t prev =
        middle_buffer_.exchange(dirty_ptr, std::memory_order_acq_rel) &
        kDirtyBitMask;
    back_buffer_ = reinterpret_cast<DataType *>(prev);
  }

private:
  static constexpr size_t kNoSharing = 64; // cache-line size.
  static constexpr uintptr_t kDirtyBit = 1;
  static constexpr uintptr_t kDirtyBitMask = ~uintptr_t() ^ kDirtyBit;

  struct alignas(kNoSharing) Buffer {
    DataType data{};
  };

  Buffer buffers_[3];

  std::atomic<uintptr_t> middle_buffer_{
      reinterpret_cast<uintptr_t>(&buffers_[1].data)};
  alignas(kNoSharing) DataType *back_buffer_{&buffers_[0].data};

  alignas(kNoSharing) DataType *front_buffer_{&buffers_[2].data};
};

} // namespace gib