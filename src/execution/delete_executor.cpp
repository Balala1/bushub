//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// delete_executor.cpp
//
// Identification: src/execution/delete_executor.cpp
//
// Copyright (c) 2015-2021, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include <memory>

#include "execution/executors/delete_executor.h"
#include "concurrency/transaction_manager.h"
#include "concurrency/lock_manager.h"
#include "concurrency/transaction.h"

namespace bustub {

DeleteExecutor::DeleteExecutor(ExecutorContext *exec_ctx, const DeletePlanNode *plan,
                               std::unique_ptr<AbstractExecutor> &&child_executor)
    : AbstractExecutor(exec_ctx) {
  plan_ = plan;
  child_executor_ = std::move(child_executor);
}

void DeleteExecutor::Init() {
  table_info_ = exec_ctx_->GetCatalog()->GetTable(plan_->TableOid());
  child_executor_->Init();
}

auto DeleteExecutor::Next([[maybe_unused]] Tuple *tuple, RID *rid) -> bool {
  auto indexes = exec_ctx_->GetCatalog()->GetTableIndexes(table_info_->name_);
  Tuple del_tup;
  RID del_rid;
  auto lock_mgr = exec_ctx_->GetLockManager();
  while (child_executor_->Next(&del_tup, &del_rid)) {
    if (lock_mgr != nullptr) {
      if (exec_ctx_->GetTransaction()->IsSharedLocked(del_rid)) {
        lock_mgr->LockUpgrade(exec_ctx_->GetTransaction(), del_rid);
      }
      if (!exec_ctx_->GetTransaction()->IsExclusiveLocked(del_rid)) {
        lock_mgr->LockExclusive(exec_ctx_->GetTransaction(),del_rid);
      }
    }

    exec_ctx_->GetTransaction()->AppendTableWriteRecord(TableWriteRecord(del_rid, WType::DELETE,
      Tuple{}, table_info_->table_.get()));
    table_info_->table_->MarkDelete(del_rid, exec_ctx_->GetTransaction());
    for (auto index : indexes) {
      exec_ctx_->GetTransaction()->AppendIndexWriteRecord(IndexWriteRecord(del_rid, table_info_->oid_, WType::DELETE,
                                                                           del_tup, Tuple{}, index->index_oid_, exec_ctx_->GetCatalog()));
      index->index_->DeleteEntry(del_tup.KeyFromTuple(table_info_->schema_, *index->index_->GetKeySchema(), index->index_->GetKeyAttrs()),
                                 del_rid, exec_ctx_->GetTransaction());
    }
  }
  return false;
}

}  // namespace bustub
