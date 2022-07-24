//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// hash_join_executor.cpp
//
// Identification: src/execution/hash_join_executor.cpp
//
// Copyright (c) 2015-2021, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "execution/executors/hash_join_executor.h"

namespace bustub {

HashJoinExecutor::HashJoinExecutor(ExecutorContext *exec_ctx, const HashJoinPlanNode *plan,
                                   std::unique_ptr<AbstractExecutor> &&left_child,
                                   std::unique_ptr<AbstractExecutor> &&right_child)
    : AbstractExecutor(exec_ctx) {
  plan_ = plan;
  left_child_ = std::move(left_child);
  right_child_ = std::move(right_child);
  index_ = 0;
}

void HashJoinExecutor::Init() {
  Tuple left_tuple;
  RID left_rid;
  left_child_->Init();
  while (left_child_->Next(&left_tuple, &left_rid)) {
    HashJoinKey hash_key;
    hash_key.value_ = plan_->LeftJoinKeyExpression()->Evaluate(&left_tuple, left_child_->GetOutputSchema());
    if (map_.count(hash_key) != 0) {
      map_[hash_key].emplace_back(left_tuple);
    } else {
      map_[hash_key] = std::vector{left_tuple};
    }
  }

  Tuple right_tuple;
  RID right_rid;
  right_child_->Init();
  while (right_child_->Next(&right_tuple, &right_rid)) {
    HashJoinKey hash_key;
    hash_key.value_ = plan_->RightJoinKeyExpression()->Evaluate(&right_tuple, right_child_->GetOutputSchema());
    if (map_.count(hash_key) == 0) { continue; }

    for (auto &it : map_[hash_key]) {
      std::vector<Value> output;
      for (auto const &column : GetOutputSchema()->GetColumns()) {
        output.push_back(column.GetExpr()->EvaluateJoin(&it, left_child_->GetOutputSchema(), &right_tuple,
                                                        right_child_->GetOutputSchema()));
      }
      result_.emplace_back(Tuple(output, GetOutputSchema()));
    }
  }
}

auto HashJoinExecutor::Next(Tuple *tuple, RID *rid) -> bool {
  if (index_ == result_.size()) { return false; }

  *tuple = result_[index_++];
  return true;
}

}  // namespace bustub
