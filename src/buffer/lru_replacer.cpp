//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// lru_replacer.cpp
//
// Identification: src/buffer/lru_replacer.cpp
//
// Copyright (c) 2015-2019, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "buffer/lru_replacer.h"

namespace bustub {

LRUReplacer::LRUReplacer(size_t num_pages) { page_num_ = num_pages; }

LRUReplacer::~LRUReplacer() = default;

auto LRUReplacer::Victim(frame_id_t *frame_id) -> bool {
  lock_.lock();
  if (hash_.empty()) {
    lock_.unlock();
    return false;
  }
  *frame_id = unpin_list_.back();
  unpin_list_.pop_back();
  hash_.erase(*frame_id);
  lock_.unlock();
  return true;
}

void LRUReplacer::Pin(frame_id_t frame_id) {
  lock_.lock();
  auto it = hash_.find(frame_id);
  if (it != hash_.end()) {
    unpin_list_.erase(it->second);
    hash_.erase(it);
  }
  lock_.unlock();
}

void LRUReplacer::Unpin(frame_id_t frame_id) {
  lock_.lock();
  auto it = hash_.find(frame_id);
  if (it == hash_.end()) {
    while (hash_.size() >= page_num_) {
      frame_id_t f = unpin_list_.back();
      unpin_list_.pop_back();
      hash_.erase(f);
    }
    unpin_list_.push_front(frame_id);
    hash_[frame_id] = unpin_list_.begin();
  }
  lock_.unlock();
}

auto LRUReplacer::Size() -> size_t {
  lock_.lock();
  size_t s = unpin_list_.size();
  lock_.unlock();
  return s;
}

}  // namespace bustub
