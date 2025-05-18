#include <cassert>
#include <cstdlib>
#include <memory>
#include <mutex>
#include <shared_mutex>

#include "util/report/macros.h"

namespace gib {

class DoubleBuffer {
public:
  DoubleBuffer() = default;
  ~DoubleBuffer() = default;

  DoubleBuffer(const DoubleBuffer &) = delete;
  DoubleBuffer(DoubleBuffer &&) = delete;
  DoubleBuffer &operator=(const DoubleBuffer &) = delete;
  DoubleBuffer &operator=(DoubleBuffer &&) = delete;

  struct RawBuffer {
    ~RawBuffer() { std::free(buffer_ptr); }

    void Resize(size_t size) {
      if (size > buffer_size) {
        void *new_ptr = std::realloc(buffer_ptr, size);
        ASSERT(new_ptr != nullptr, "Failed to realloc buffer");
        buffer_ptr = new_ptr;
        buffer_size = size;
      }
    }

    void *buffer_ptr{nullptr};
    size_t buffer_size{0};
  };

  struct ReadLockBuffer {
    std::unique_lock<std::mutex> lock;
    const RawBuffer *data;
    bool changed;
  };

  struct WriteLockBuffer {
    std::unique_lock<std::mutex> lock;
    RawBuffer *data;
  };

  ReadLockBuffer GetReadLockedBuffer() {
    std::shared_lock<std::shared_mutex> state_lock(state_mutex_);
    bool read_primary = read_from_primary_buffer_;
    std::mutex &buf_mutex = read_primary ? primary_mutex_ : secondary_mutex_;
    RawBuffer &buf = read_primary ? primary_buf_ : secondary_buf_;
    bool was_changed = data_changed_;
    data_changed_ = false; // clear change flag
    return {std::unique_lock<std::mutex>(buf_mutex), &buf, was_changed};
  }

  WriteLockBuffer GetWriteLockedBuffer() {
    std::shared_lock<std::shared_mutex> state_lock(state_mutex_);
    bool write_primary = !read_from_primary_buffer_;
    std::mutex &buf_mutex = write_primary ? primary_mutex_ : secondary_mutex_;
    RawBuffer &buf = write_primary ? primary_buf_ : secondary_buf_;
    return {std::unique_lock<std::mutex>(buf_mutex), &buf};
  }

  void SwapBuffers() {
    std::unique_lock<std::shared_mutex> state_lock(state_mutex_);
    read_from_primary_buffer_ = !read_from_primary_buffer_;
    data_changed_ = true;
  }

private:
  std::shared_mutex state_mutex_;
  bool read_from_primary_buffer_{true};
  bool data_changed_{false};

  std::mutex primary_mutex_;
  std::mutex secondary_mutex_;
  RawBuffer primary_buf_;
  RawBuffer secondary_buf_;
};

} // namespace gib