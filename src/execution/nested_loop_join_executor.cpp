//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// nested_loop_join_executor.cpp
//
// Identification: src/execution/nested_loop_join_executor.cpp
//
// Copyright (c) 2015-2021, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "execution/executors/nested_loop_join_executor.h"

namespace bustub {

NestedLoopJoinExecutor::NestedLoopJoinExecutor(ExecutorContext *exec_ctx, const NestedLoopJoinPlanNode *plan,
                                               std::unique_ptr<AbstractExecutor> &&left_executor,
                                               std::unique_ptr<AbstractExecutor> &&right_executor)
    : AbstractExecutor(exec_ctx) {
  plan_ = plan;
  left_child_ = std::move(left_executor);
  right_child_ = std::move(right_executor);
  index_ = 0;
}

void NestedLoopJoinExecutor::Init() {
  left_child_->Init();
  Tuple left_tuple;
  RID left_rid;
  Tuple right_tuple;
  RID right_rid;
  while (left_child_->Next(&left_tuple, &left_rid)) {
    right_child_->Init();
    while (right_child_->Next(&right_tuple, &right_rid)) {
      if (plan_->Predicate() == nullptr || plan_->Predicate()->EvaluateJoin(&left_tuple, left_child_->GetOutputSchema(),
                                                                            &right_tuple, right_child_->GetOutputSchema()).GetAs<bool>()) {
        std::vector<Value> output;
        for (auto const &column : GetOutputSchema()->GetColumns()) {
          output.emplace_back(column.GetExpr()->EvaluateJoin(&left_tuple, left_child_->GetOutputSchema(),
                                                             &right_tuple, right_child_->GetOutputSchema()));
        }
        result_.emplace_back(Tuple(output, GetOutputSchema()));
      }
    }
  }
}

auto NestedLoopJoinExecutor::Next(Tuple *tuple, RID *rid) -> bool {
  if (index_ == result_.size()) { return false; }

  *tuple = result_[index_++];
  return true;
}

}  // namespace bustub
