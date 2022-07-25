//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// distinct_plan.h
//
// Identification: src/include/execution/plans/distinct_plan.h
//
// Copyright (c) 2015-2021, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#pragma once

#include <utility>
#include <vector>

#include "common/util/hash_util.h"
#include "execution/plans/abstract_plan.h"
#include "storage/table/tuple.h"

namespace bustub {

/**
 * Distinct removes duplicate rows from the output of a child node.
 */
class DistinctPlanNode : public AbstractPlanNode {
 public:
  /**
   * Construct a new DistinctPlanNode instance.
   * @param child The child plan from which tuples are obtained
   */
  DistinctPlanNode(const Schema *output_schema, const AbstractPlanNode *child)
      : AbstractPlanNode(output_schema, {child}) {}

  /** @return The type of the plan node */
  auto GetType() const -> PlanType override { return PlanType::Distinct; }

  /** @return The child plan node */
  auto GetChildPlan() const -> const AbstractPlanNode * {
    BUSTUB_ASSERT(GetChildren().size() == 1, "Distinct should have at most one child plan.");
    return GetChildAt(0);
  }
};

/** DistinctKey represents a key in an aggregation operation */
struct DistinctKey {
    /** The distinct values */
    std::vector<Value> distinct_val_;

    /**
     * Compares two distinct keys for equality.
     * @param other the other distinct key to be compared with
     * @return `true` if both distinct keys have equivalent distinct values, `false` otherwise
     */
    auto operator==(const DistinctKey &other) const -> bool {
        for (uint32_t i = 0; i < other.distinct_val_.size(); i++) {
            if (distinct_val_[i].CompareEquals(other.distinct_val_[i]) != CmpBool::CmpTrue) {
                return false;
            }
        }
        return true;
    }
};

}  // namespace bustub

namespace std {

/** Implements std::hash on DistinctKey */
    template <>
    struct hash<bustub::DistinctKey> {
        auto operator()(const bustub::DistinctKey &dis_key) const -> std::size_t {
            size_t curr_hash = 0;
            for (const auto &key : dis_key.distinct_val_) {
                if (!key.IsNull()) {
                    curr_hash = bustub::HashUtil::CombineHashes(curr_hash, bustub::HashUtil::HashValue(&key));
                }
            }
            return curr_hash;
        }
    };

}  // namespace std
