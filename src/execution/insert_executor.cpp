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

  if (!plan_->IsRawInsert()) {
    while (child_->Next(tuple, rid)) {
      if (tuple != nullptr) {
        table_->table_->InsertTuple(*tuple, rid, exec_ctx_->GetTransaction());
      }
      for (auto index : indexes) {
        index->index_->InsertEntry(tuple->KeyFromTuple(table_->schema_, *index->index_->GetKeySchema(), index->index_->GetKeyAttrs()),
                                   *rid, exec_ctx_->GetTransaction());
      }
    }
    return false;
  }

  for (auto value : plan_->RawValues()) {
    Tuple tup = Tuple(value, &table_->schema_);
    table_->table_->InsertTuple(tup, rid, exec_ctx_->GetTransaction());
    for (auto index : indexes) {
      index->index_->InsertEntry(tup.KeyFromTuple(table_->schema_, *index->index_->GetKeySchema(), index->index_->GetKeyAttrs()),
                                 *rid, exec_ctx_->GetTransaction());
    }
  }
  return false;
}

}  // namespace bustub
