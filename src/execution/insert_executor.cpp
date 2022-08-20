//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// insert_executor.cpp
//
// Identification: src/execution/insert_executor.cpp
//
// Copyright (c) 2015-2021, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include <memory>

#include "execution/executors/insert_executor.h"

namespace bustub {

InsertExecutor::InsertExecutor(ExecutorContext *exec_ctx, const InsertPlanNode *plan,
                               std::unique_ptr<AbstractExecutor> &&child_executor)
    : AbstractExecutor(exec_ctx) {
  plan_ = plan;
  child_ = std::move(child_executor);
  index_ = 0;
}

void InsertExecutor::Init() {
  if (!plan_->IsRawInsert()) {
    child_->Init();
  }

  table_ = exec_ctx_->GetCatalog()->GetTable(plan_->TableOid());
}

auto InsertExecutor::Next([[maybe_unused]] Tuple *tuple, RID *rid) -> bool {
  auto indexes = exec_ctx_->GetCatalog()->GetTableIndexes(table_->name_);

  auto lock_mgr = exec_ctx_->GetLockManager();
  auto txn = exec_ctx_->GetTransaction();
  if (!plan_->IsRawInsert()) {
    while (child_->Next(tuple, rid)) {
      if (tuple != nullptr) {
        if (lock_mgr != nullptr) {
          if (txn->IsSharedLocked(*rid)) {
            lock_mgr->LockUpgrade(txn, *rid);
          }
          if (!txn->IsExclusiveLocked(*rid)) {
            lock_mgr->LockExclusive(txn, *rid);
          }
        }

        exec_ctx_->GetTransaction()->AppendTableWriteRecord(TableWriteRecord(*rid, WType::INSERT,
                                                                             *tuple, table_->table_.get()));

        table_->table_->InsertTuple(*tuple, rid, exec_ctx_->GetTransaction());

        for (auto index: indexes) {
          exec_ctx_->GetTransaction()->AppendIndexWriteRecord(IndexWriteRecord(*rid, table_->oid_, WType::INSERT,
                                                                               *tuple, Tuple{}, index->index_oid_, exec_ctx_->GetCatalog()));
          index->index_->InsertEntry(
                  tuple->KeyFromTuple(table_->schema_, *index->index_->GetKeySchema(), index->index_->GetKeyAttrs()),
                  *rid, exec_ctx_->GetTransaction());
        }
      }
    }
    return false;
  }

  for (auto value : plan_->RawValues()) {
    Tuple tup = Tuple(value, &table_->schema_);
    if (lock_mgr != nullptr) {
      if (txn->IsSharedLocked(*rid)) {
        lock_mgr->LockUpgrade(txn, *rid);
      }
      if (!txn->IsExclusiveLocked(*rid)) {
        lock_mgr->LockExclusive(txn, *rid);
      }
    }

    exec_ctx_->GetTransaction()->AppendTableWriteRecord(TableWriteRecord(*rid, WType::INSERT,
                                                                         tup, table_->table_.get()));
    table_->table_->InsertTuple(tup, rid, exec_ctx_->GetTransaction());
    for (auto index : indexes) {
      exec_ctx_->GetTransaction()->AppendIndexWriteRecord(IndexWriteRecord(*rid, table_->oid_, WType::INSERT,
                                                                           tup, Tuple{}, index->index_oid_, exec_ctx_->GetCatalog()));
      index->index_->InsertEntry(tup.KeyFromTuple(table_->schema_, *index->index_->GetKeySchema(), index->index_->GetKeyAttrs()),
                                 *rid, exec_ctx_->GetTransaction());
    }
  }
  return false;
}

}  // namespace bustub
