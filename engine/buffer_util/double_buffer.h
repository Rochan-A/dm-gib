#pragma once

#include <atomic>
#include <cassert>
#include <cstdlib>
#include <memory>
#include <mutex>

#include "util/report/macros.h"

namespace gib {

template <class T> class DoubleBuffer {
public:
  struct ReadLockBuffer {
    std::unique_lock<std::mutex> buf_lock;
    const T *data;
    bool changed;
  };

  struct WriteLockBuffer {
    std::unique_lock<std::mutex> buf_lock;
    T *data;
  };

  ReadLockBuffer GetReadLockedBuffer() {
    bool use_primary = reader_on_primary_.load(std::memory_order_acquire);

    std::mutex &m = use_primary ? primary_mutex_ : secondary_mutex_;
    T &b = use_primary ? primary_buf_ : secondary_buf_;

    return {std::unique_lock<std::mutex>(m), &b,
            data_changed_.exchange(false, std::memory_order_relaxed)};
  }

  WriteLockBuffer GetWriteLockedBuffer() {
    bool reader_primary = reader_on_primary_.load(std::memory_order_relaxed);

    std::mutex &m = reader_primary ? secondary_mutex_ : primary_mutex_;
    T &b = reader_primary ? secondary_buf_ : primary_buf_;

    return {std::unique_lock<std::mutex>(m), &b};
  }

  // Call when the writer is done filling its buffer.
  void SwapBuffers() {
    reader_on_primary_.store(
        !reader_on_primary_.load(std::memory_order_relaxed),
        std::memory_order_release);
    data_changed_.store(true, std::memory_order_relaxed);
  }

private:
  // Buffer the reader should lock next.
  std::atomic<bool> reader_on_primary_{true};

  // True if the writer publish fresh data since the last read.
  std::atomic<bool> data_changed_{false};

  std::mutex primary_mutex_, secondary_mutex_;
  T primary_buf_{}, secondary_buf_{};
};

} // namespace gib