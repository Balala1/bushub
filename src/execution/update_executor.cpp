//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// update_executor.cpp
//
// Identification: src/execution/update_executor.cpp
//
// Copyright (c) 2015-2021, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//
#include <memory>

#include "execution/executors/update_executor.h"

namespace bustub {

UpdateExecutor::UpdateExecutor(ExecutorContext *exec_ctx, const UpdatePlanNode *plan,
                               std::unique_ptr<AbstractExecutor> &&child_executor)
    : AbstractExecutor(exec_ctx) {
  plan_ = plan;
  child_executor_ = std::move(child_executor);
}

void UpdateExecutor::Init() {
  table_info_ = exec_ctx_->GetCatalog()->GetTable(plan_->TableOid());
  child_executor_->Init();
}

auto UpdateExecutor::Next([[maybe_unused]] Tuple *tuple, RID *rid) -> bool {
  while (child_executor_->Next(tuple, rid)) {
    if (tuple == nullptr) { continue; }
    auto new_tuple = GenerateUpdatedTuple(*tuple);
    auto lock_mgr = exec_ctx_->GetLockManager();
    auto txn = exec_ctx_->GetTransaction();
    if (lock_mgr != nullptr) {
      if (txn->IsSharedLocked(*rid)) {
        lock_mgr->LockUpgrade(txn, *rid);
      }
      if (!txn->IsExclusiveLocked(*rid)) {
        lock_mgr->LockExclusive(txn, *rid);
      }
    }

    exec_ctx_->GetTransaction()->AppendTableWriteRecord(TableWriteRecord(*rid, WType::UPDATE,
                                                                         new_tuple, table_info_->table_.get()));

    table_info_->table_->UpdateTuple(new_tuple, *rid,
                                     exec_ctx_->GetTransaction());
    auto indexes = exec_ctx_->GetCatalog()->GetTableIndexes(table_info_->name_);
    for (auto index : indexes) {
      exec_ctx_->GetTransaction()->AppendIndexWriteRecord(IndexWriteRecord(*rid, table_info_->oid_, WType::UPDATE,
                                                                           new_tuple, *tuple, index->index_oid_, exec_ctx_->GetCatalog()));
      index->index_->DeleteEntry(
          tuple->KeyFromTuple(table_info_->schema_, *index->index_->GetKeySchema(),
                              index->index_->GetKeyAttrs()),
          *rid, exec_ctx_->GetTransaction());
      index->index_->InsertEntry(
          new_tuple.KeyFromTuple(table_info_->schema_, *index->index_->GetKeySchema(),
                                 index->index_->GetKeyAttrs()),
          new_tuple.GetRid(), exec_ctx_->GetTransaction());
    }
  }
  return false;
}

auto UpdateExecutor::GenerateUpdatedTuple(const Tuple &src_tuple) -> Tuple {
  const auto &update_attrs = plan_->GetUpdateAttr();
  Schema schema = table_info_->schema_;
  uint32_t col_count = schema.GetColumnCount();
  std::vector<Value> values;
  for (uint32_t idx = 0; idx < col_count; idx++) {
    if (update_attrs.find(idx) == update_attrs.cend()) {
      values.emplace_back(src_tuple.GetValue(&schema, idx));
    } else {
      const UpdateInfo info = update_attrs.at(idx);
      Value val = src_tuple.GetValue(&schema, idx);
      switch (info.type_) {
        case UpdateType::Add:
          values.emplace_back(val.Add(ValueFactory::GetIntegerValue(info.update_val_)));
          break;
        case UpdateType::Set:
          values.emplace_back(ValueFactory::GetIntegerValue(info.update_val_));
          break;
      }
    }
  }
  return Tuple{values, &schema};
}

}  // namespace bustub
