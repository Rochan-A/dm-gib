#include <cassert>
#include <cstdlib>
#include <memory>
#include <mutex>
#include <shared_mutex>

#include "util/report/macros.h"

namespace gib {

// Double buffer class. Each buffer is protected with read-write locks.
class DoubleBuffer {
public:
  DoubleBuffer() = default;
  ~DoubleBuffer() = default;

  DoubleBuffer(const DoubleBuffer &) = delete;
  DoubleBuffer(DoubleBuffer &&) = delete;
  DoubleBuffer &operator=(const DoubleBuffer &) = delete;
  DoubleBuffer &operator=(DoubleBuffer &&) = delete;

  // Raw buffer containing pointer and buffer size.
  struct RawBuffer {
    ~RawBuffer() {
      if (buffer_ptr) {
        std::free(buffer_ptr);
      }
    }

    void Resize(const size_t size) {
      void *new_ptr = std::realloc(buffer_ptr, size);
      ASSERT(new_ptr != nullptr, "Failed to realloc buffer");
      buffer_ptr = new_ptr;
      buffer_size = size;
    }

    void *buffer_ptr{nullptr};
    size_t buffer_size{0};
  };

  // Locked buffer for reading. Multiple threads may hold this object.
  struct ReadLockBuffer {
    std::unique_lock<std::mutex> _lock;
    const RawBuffer *data{nullptr};
    bool changed{false};
  };

  // Locked buffer for writing. Only one thread can hold this object.
  struct WriteLockBuffer {
    std::unique_lock<std::mutex> _lock;
    RawBuffer *data;
  };

  // Get buffer for reading. Multiple callers may hold the returned object.
  ReadLockBuffer GetReadLockedBuffer() {
    std::shared_lock<std::shared_mutex> state_lock(state_mutex_);
    std::mutex &buf_mutex =
        read_from_primary_buffer_ ? primary_mutex_ : secondary_mutex_;
    RawBuffer &buf = read_from_primary_buffer_ ? primary_buf_ : secondary_buf_;
    bool was_changed = data_changed_;
    data_changed_ = false; // clear change flag
    return {std::unique_lock<std::mutex>(buf_mutex), &buf, was_changed};
  }

  // Get buffer for writing. Only one thread can hold the object returned.
  WriteLockBuffer GetWriteLockedBuffer() {
    std::unique_lock<std::shared_mutex> state_lock(state_mutex_);
    std::mutex &buf_mutex =
        !read_from_primary_buffer_ ? primary_mutex_ : secondary_mutex_;
    RawBuffer &buf = !read_from_primary_buffer_ ? primary_buf_ : secondary_buf_;
    return {std::unique_lock<std::mutex>(buf_mutex), &buf};
  }

  // Exchange the two buffers. Only one thread may call this at a time.
  void SwapBuffers() {
    std::unique_lock<std::shared_mutex> state_lock(state_mutex_);
    read_from_primary_buffer_ = !read_from_primary_buffer_;
    data_changed_ = true;
  }

private:
  // Protect member functions of this class with this read-write lock.
  std::shared_mutex state_mutex_;

  // Read from primary buffer if true. Otherwise, read from secondary buffer and
  // use primary for writing.
  bool read_from_primary_buffer_{true};

  // Had data changed in current primary buffer being read.
  bool data_changed_{false};

  std::mutex primary_mutex_;
  std::mutex secondary_mutex_;
  RawBuffer primary_buf_;
  RawBuffer secondary_buf_;
};

} // namespace gib