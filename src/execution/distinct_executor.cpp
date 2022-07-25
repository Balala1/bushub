//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// distinct_executor.cpp
//
// Identification: src/execution/distinct_executor.cpp
//
// Copyright (c) 2015-2021, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//
#include <unordered_map>

#include "execution/executors/distinct_executor.h"

namespace bustub {

DistinctExecutor::DistinctExecutor(ExecutorContext *exec_ctx, const DistinctPlanNode *plan,
                                   std::unique_ptr<AbstractExecutor> &&child_executor)
    : AbstractExecutor(exec_ctx) {
    plan_ = plan;
    child_executor_ = std::move(child_executor);
}

void DistinctExecutor::Init() {
    child_executor_->Init();

    Tuple tup;
    RID rid;
    while (child_executor_->Next(&tup, &rid)) {
        DistinctKey key;
        int len = GetOutputSchema()->GetColumnCount();
        for (int i = 0; i < len; i++) {
            key.distinct_val_.emplace_back(tup.GetValue(GetOutputSchema(), i));
        }
        auto t = key;
        // std::cout << tup. << std::endl;
        if (map_.count(key) == 0) {
            map_.insert({key, true});
        }
    }
    std::cout << map_.size() << std::endl;
    iter_ = map_.begin();
}

auto DistinctExecutor::Next(Tuple *tuple, RID *rid) -> bool {
    if (iter_ == map_.end()) { return false; }

    *tuple = Tuple(iter_->first.distinct_val_, GetOutputSchema());
    iter_++;
    return true;
}

}  // namespace bustub
