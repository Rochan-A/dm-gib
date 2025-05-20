#pragma once

#include <atomic>
#include <memory>

namespace gib {

// Lock-free single producer-single consumer double buffer.
template <typename DataType> class LockFreeDoubleBuffer {
public:
  // Buffers will be default initialized.
  LockFreeDoubleBuffer() = default;

  // Initializes front buffer with given data, other buffer is default
  // initialized.
  LockFreeDoubleBuffer(DataType &&initial_front_buf)
      : buffers_{{}, {std::forward<DataType>(initial_front_buf)}} {}

  // Initializes both the buffers.
  LockFreeDoubleBuffer(DataType &&initial_back_buf,
                       DataType &&initial_front_buf)
      : buffers_{{std::forward<DataType>(initial_back_buf)},
                 {std::forward<DataType>(initial_front_buf)}} {}

  // Returns reference to the latest front buffer.
  std::pair<DataType &, bool> Read() {
    // If there's a new buffer recently written -> will swap buffers and return
    // the latest value. Otherwise, will return stale value.
    uintptr_t dirty_ptr = back_buffer_.load(std::memory_order_relaxed);
    if ((dirty_ptr & kDirtyBit) == 0) {
      return {*front_buffer_, false};
    }
    uintptr_t prev = back_buffer_.exchange(
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

  // Propagates pending changes from the back buffer to the front buffer.
  void Commit() {
    // The next read will get those changes unless the writer is faster and will
    // overwrite those changes before the next Read() call.
    const uintptr_t dirty_ptr =
        kDirtyBit | reinterpret_cast<uintptr_t>(back_buffer_);
    uintptr_t prev =
        front_buffer_.exchange(dirty_ptr, std::memory_order_acq_rel) &
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

  Buffer buffers_[2];

  std::atomic<uintptr_t> back_buffer_{
      reinterpret_cast<uintptr_t>(&buffers_[0].data)};
  std::atomic<uintptr_t> front_buffer_{
      reinterpret_cast<uintptr_t>(&buffers_[1].data)};
};

} // namespace gib