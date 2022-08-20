//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// seq_scan_executor.cpp
//
// Identification: src/execution/seq_scan_executor.cpp
//
// Copyright (c) 2015-2021, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "execution/executors/seq_scan_executor.h"
#include "execution/execution_engine.h"
#include "storage/table/table_iterator.h"
#include "storage/table/table_heap.h"

namespace bustub {

SeqScanExecutor::SeqScanExecutor(ExecutorContext *exec_ctx, const SeqScanPlanNode *plan) : AbstractExecutor(exec_ctx), iter_(TableIterator(nullptr, RID(), nullptr)) {
  plan_ = plan;
}

void SeqScanExecutor::Init() {
  auto catalog = exec_ctx_->GetCatalog();
  table_ = catalog->GetTable(plan_->GetTableOid());
  if (table_ == nullptr) { return; }

  iter_ = table_->table_->Begin(exec_ctx_->GetTransaction());
}

auto SeqScanExecutor::Next(Tuple *tuple, RID *rid) -> bool {
  auto lock_mgr = exec_ctx_->GetLockManager();
  while (iter_ != table_->table_->End()) {
    if (lock_mgr != nullptr && exec_ctx_->GetTransaction()->GetIsolationLevel() != IsolationLevel::READ_UNCOMMITTED) {
      lock_mgr->LockShared(exec_ctx_->GetTransaction(), iter_->GetRid());
    }

    if (plan_->GetPredicate() == nullptr ||
        plan_->GetPredicate()->Evaluate(&*iter_, plan_->OutputSchema()).GetAs<bool>()) {
      *tuple = *iter_;
      *rid = tuple->GetRid();
      if (lock_mgr != nullptr && exec_ctx_->GetTransaction()->GetIsolationLevel() == IsolationLevel::READ_COMMITTED) {
        lock_mgr->Unlock(exec_ctx_->GetTransaction(), iter_->GetRid());
      }
      iter_++;
      return true;
    }
    if (lock_mgr != nullptr) {
      lock_mgr->Unlock(exec_ctx_->GetTransaction(), iter_->GetRid());
    }
    iter_++;
  }
  return false;
}

}  // namespace bustub
