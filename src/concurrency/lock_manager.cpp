//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// lock_manager.cpp
//
// Identification: src/concurrency/lock_manager.cpp
//
// Copyright (c) 2015-2019, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "concurrency/lock_manager.h"
#include "concurrency/transaction_manager.h"

#include <utility>
#include <vector>
#include <algorithm>

namespace bustub {

auto LockManager::LockPrepare(Transaction *txn, const RID &rid) -> bool {
  if (txn->GetIsolationLevel() != IsolationLevel::READ_UNCOMMITTED && txn->GetState() == TransactionState::SHRINKING) {
    txn->SetState(TransactionState::ABORTED);
    throw TransactionAbortException(txn->GetTransactionId(), AbortReason::LOCK_ON_SHRINKING);
    return false;
  }

  if (lock_table_.count(rid) == 0) {
    lock_table_.emplace(std::piecewise_construct, std::forward_as_tuple(rid), std::forward_as_tuple());
  }

  return true;
}

std::list<LockManager::LockRequest>::iterator LockManager::FindIterInRequestQueue(std::list<LockRequest> &request_queue, txn_id_t txn_id) {
  for (auto iter = request_queue.begin(); iter != request_queue.end(); ++iter) {
    if (iter->txn_id_ == txn_id) { return iter; }
  }
  return request_queue.end();
}

void LockManager::check_aborted(Transaction *txn, LockRequestQueue* request_queue) {
  if (txn->GetState() == TransactionState::ABORTED) {
    auto iter = FindIterInRequestQueue(request_queue->request_queue_, txn->GetTransactionId());
    request_queue->request_queue_.erase(iter);
    throw TransactionAbortException(txn->GetTransactionId(), AbortReason::DEADLOCK);
  }
}

auto LockManager::LockShared(Transaction *txn, const RID &rid) -> bool {
  std::unique_lock<std::mutex> lock(latch_);

  if (txn->GetIsolationLevel() == IsolationLevel::READ_UNCOMMITTED) {
    txn->SetState(TransactionState::ABORTED);
    throw TransactionAbortException(txn->GetTransactionId(), AbortReason::LOCKSHARED_ON_READ_UNCOMMITTED);
    return false;
  }

  if (!LockPrepare(txn, rid)) {
    return false;
  }

  auto request_queue = &lock_table_.find(rid)->second;
  request_queue->request_queue_.emplace_back(txn->GetTransactionId(), LockMode::SHARED);

  if (request_queue->is_writing_) {
    request_queue->cv_.wait(lock, [request_queue, txn]() -> bool {
      return txn->GetState() == TransactionState::ABORTED || !request_queue->is_writing_;
    });
  }

  check_aborted(txn, request_queue);

  txn->GetSharedLockSet()->emplace(rid);
  request_queue->share_cnt_++;
  auto iter = FindIterInRequestQueue(request_queue->request_queue_, txn->GetTransactionId());
  iter->granted_ = true;

  return true;
}

auto LockManager::LockExclusive(Transaction *txn, const RID &rid) -> bool {
  std::unique_lock<std::mutex> lock(latch_);

  if (!LockPrepare(txn, rid)) {
    return false;
  }

  auto request_queue = &lock_table_.find(rid)->second;
  request_queue->request_queue_.emplace_back(txn->GetTransactionId(), LockMode::EXCLUSIVE);

  if (request_queue->is_writing_ || request_queue->share_cnt_ > 0) {
    request_queue->cv_.wait(lock, [request_queue, txn]() -> bool {
      return (!request_queue->is_writing_ && request_queue->share_cnt_ == 0) || txn->GetState() == TransactionState::ABORTED;
    });
  }

  check_aborted(txn, request_queue);

  txn->GetExclusiveLockSet()->emplace(rid);
  request_queue->is_writing_ = true;
  auto iter = FindIterInRequestQueue(request_queue->request_queue_, txn->GetTransactionId());
  iter->granted_ = true;
  return true;
}

auto LockManager::LockUpgrade(Transaction *txn, const RID &rid) -> bool {
  std::unique_lock<std::mutex> lock(latch_);

  if (txn->GetIsolationLevel() != IsolationLevel::READ_UNCOMMITTED && txn->GetState() == TransactionState::SHRINKING) {
    txn->SetState(TransactionState::ABORTED);
    throw TransactionAbortException(txn->GetTransactionId(), AbortReason::LOCK_ON_SHRINKING);
    return false;
  }

  auto request_queue = &lock_table_.find(rid)->second;

  if (request_queue->upgrading_ != INVALID_TXN_ID) {
    txn->SetState(TransactionState::ABORTED);
    throw TransactionAbortException(txn->GetTransactionId(), AbortReason::DEADLOCK);
    return false;
  }

  request_queue->upgrading_ = txn->GetTransactionId();
  txn->GetSharedLockSet()->erase(rid);
  request_queue->share_cnt_--;
  auto iter = FindIterInRequestQueue(request_queue->request_queue_, txn->GetTransactionId());
  iter->lock_mode_ = LockMode::EXCLUSIVE;
  iter->granted_ = false;

  if (request_queue->is_writing_ || request_queue->share_cnt_ > 0) {
    request_queue->cv_.wait(lock, [request_queue, txn]() -> bool {
      return (!request_queue->is_writing_ && request_queue->share_cnt_ == 0) || txn->GetState() == TransactionState::ABORTED;
    });
  }

  check_aborted(txn, request_queue);

  txn->GetExclusiveLockSet()->emplace(rid);
  request_queue->upgrading_ = INVALID_TXN_ID;
  request_queue->is_writing_ = true;
  iter->granted_ = true;

  return true;
}

auto LockManager::Unlock(Transaction *txn, const RID &rid) -> bool {
  std::unique_lock<std::mutex> lock(latch_);

  auto request_queue = &lock_table_.find(rid)->second;

  txn->GetSharedLockSet()->erase(rid);
  txn->GetExclusiveLockSet()->erase(rid);

  auto iter = FindIterInRequestQueue(request_queue->request_queue_, txn->GetTransactionId());

  switch (iter->lock_mode_) {
    case LockMode::SHARED:
      if (txn->GetIsolationLevel() == IsolationLevel::REPEATABLE_READ && txn->GetState() == TransactionState::GROWING) {
        txn->SetState(TransactionState::SHRINKING);
      }
      if (--request_queue->share_cnt_ == 0) {
        request_queue->cv_.notify_all();
      }
      break;
    case LockMode::EXCLUSIVE:
      if ((txn->GetIsolationLevel() == IsolationLevel::READ_COMMITTED || txn->GetIsolationLevel() == IsolationLevel::REPEATABLE_READ)
      && txn->GetState() == TransactionState::GROWING) {
        txn->SetState(TransactionState::SHRINKING);
      }
      request_queue->is_writing_ = false;
      request_queue->cv_.notify_all();
      break;
  }

  return true;
}

// return the first index that txns[index] >= target
size_t LockManager::getInsertIndex(std::vector<txn_id_t> &txns, txn_id_t target) {
  size_t l = 0, r = txns.size() - 1;
  while (l <= r) {
    size_t mid = (l + r) / 2;
    if (txns[mid] == target) {
      return target;
    } else if (txns[mid] < target) {
      l = mid + 1;
    } else {
      r = mid - 1;
    }
  }
  return l;
}

void LockManager::AddEdge(txn_id_t t1, txn_id_t t2) {
  txn_set_.insert(t1);
  txn_set_.insert(t2);

  if (waits_for_.count(t1) == 0) {
    waits_for_[t1].emplace_back(t2);
    return;
  }

  size_t index = getInsertIndex(waits_for_[t1], t2);
  if (index < waits_for_[t1].size() && waits_for_[t1][index] == t2) {
    // already exist
    return;
  }

  if (index == waits_for_[t1].size()) {
    waits_for_[t1].push_back(t2);
    return;
  }
  waits_for_[t1].insert(waits_for_[t1].begin() + index, t2);
}

void LockManager::RemoveEdge(txn_id_t t1, txn_id_t t2) {
  if (waits_for_.count(t1) == 0) {
    return;
  }

  size_t index = getInsertIndex(waits_for_[t1], t2);
  if (index < waits_for_[t1].size() && waits_for_[t1][index] == t2) {
    // exist
    waits_for_[t1].erase(waits_for_[t1].begin() + index);
    if (waits_for_[t1].size() == 0) { txn_set_.erase(t1); }
    return;
  }
}

bool LockManager::HasCycle(txn_id_t *txn_id) {
  std::unordered_set<txn_id_t> visit;
  for (auto start_txn_id : txn_set_) {
    std::unordered_set<txn_id_t> path_set;
    std::vector<txn_id_t> path;
    if (dfs(start_txn_id, visit, path, path_set)) {
      txn_id_t youngest = -1;
      size_t index = 0;
      for (; index < path.size() && path[index] != path[path.size() - 1]; index++);
      for (; index < path.size(); index++) {
        youngest = std::max(youngest, path[index]);
      }
      *txn_id = youngest;
      return true;
    }
  }
  return false;
}

bool LockManager::dfs(txn_id_t start_txn_id, std::unordered_set<txn_id_t> &visit, std::vector<txn_id_t> &path,
                      std::unordered_set<txn_id_t> &path_set) {
  if (visit.count(start_txn_id) > 0) {
    return false;
  }

  path_set.insert(start_txn_id);
  visit.insert(start_txn_id);
  path.emplace_back(start_txn_id);
  for (auto next : waits_for_[start_txn_id]) {
    if (path_set.count(next) != 0) {
      path.emplace_back(next);
      return true;
    }

    if (dfs(next, visit, path, path_set)) {
      return true;
    }
  }

  path_set.erase(start_txn_id);
  path.pop_back();
  return false;
}

std::vector<std::pair<txn_id_t, txn_id_t>> LockManager::GetEdgeList() {
  std::vector<std::pair<txn_id_t, txn_id_t>> result;
  for (auto t1 : txn_set_) {
    for (auto t2 : waits_for_[t1]) {
      result.emplace_back(std::pair<txn_id_t, txn_id_t>(t1, t2));
    }
  }
  return result;
}

void LockManager::RunCycleDetection() {
  while (enable_cycle_detection_) {
    std::this_thread::sleep_for(cycle_detection_interval);
    {
      std::unique_lock<std::mutex> l(latch_);
      // TODO(student): remove the continue and add your cycle detection and abort code here
      std::unordered_map<txn_id_t, RID> record_list;
      for (auto &[rid, queue] : lock_table_) {
        for (auto not_grant_lr : queue.request_queue_) {
          if (not_grant_lr.granted_) { continue; }
          record_list[not_grant_lr.txn_id_] = rid;
          for (auto grant_lr : queue.request_queue_) {
            if (!grant_lr.granted_) { continue; }
            AddEdge(not_grant_lr.txn_id_, grant_lr.txn_id_);
          }
        }
      }

      txn_id_t txn_id;
      while (HasCycle(&txn_id)) {
        LOG_INFO("get txn_id %d to break cycle", txn_id);
        auto txn = TransactionManager::GetTransaction(txn_id);
        for (auto t1 : txn_set_) {
          if (t1 == txn_id) { continue; }
          RemoveEdge(t1, txn_id);
        }
        waits_for_.erase(txn_id);
        txn_set_.erase(txn_id);

        txn->SetState(TransactionState::ABORTED);
        lock_table_[record_list[txn_id]].cv_.notify_all();
      }
    }

    waits_for_.clear();
    txn_set_.clear();
  }
}

}  // namespace bustub
