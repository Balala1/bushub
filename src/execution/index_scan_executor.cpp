//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// index_scan_executor.cpp
//
// Identification: src/execution/index_scan_executor.cpp
//
// Copyright (c) 2015-19, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//
#include "execution/executors/index_scan_executor.h"
#include "storage/index/b_plus_tree_index.h"

namespace bustub {
IndexScanExecutor::IndexScanExecutor(ExecutorContext *exec_ctx, const IndexScanPlanNode *plan)
    : AbstractExecutor(exec_ctx) {
  plan_ = plan;
}

void IndexScanExecutor::Init() {
  auto index = exec_ctx_->GetCatalog()->GetIndex(plan_->GetIndexOid());
  b_index_ = reinterpret_cast<BPlusTreeIndex<GenericKey<8>, RID, GenericComparator<8>> *>(index->index_.get());
  iter_ = b_index_->GetBeginIterator();

  table_ = exec_ctx_->GetCatalog()->GetTable(index->table_name_);
}

auto IndexScanExecutor::Next(Tuple *tuple, RID *rid) -> bool {
  if (iter_ == b_index_->GetEndIterator()) {
    return false;
  }

  auto lock_mgr = exec_ctx_->GetLockManager();
  auto txn = exec_ctx_->GetTransaction();
  if (lock_mgr != nullptr && txn->GetIsolationLevel() != IsolationLevel::READ_UNCOMMITTED) {
    lock_mgr->LockShared(txn, (*iter_).second);
  }

  table_->table_->GetTuple((*iter_).second, tuple, txn);
  if (plan_->GetPredicate() != nullptr &&
      !plan_->GetPredicate()->Evaluate(tuple, plan_->OutputSchema()).GetAs<bool>()) {
    if (lock_mgr != nullptr) {
      lock_mgr->Unlock(txn, (*iter_).second);
    }
    ++iter_;
    return false;
  }

  *rid = (*iter_).second;
  if (lock_mgr != nullptr && txn->GetIsolationLevel() == IsolationLevel::READ_COMMITTED) {
    lock_mgr->Unlock(txn, (*iter_).second);
  }
  ++iter_;
  return true;
}
}  // namespace bustub
