/**
 * index_iterator.cpp
 */
#include <cassert>

#include "storage/index/index_iterator.h"

namespace bustub {

/*
 * NOTE: you can change the destructor/constructor method here
 * set your own input parameters
 */
INDEX_TEMPLATE_ARGUMENTS
INDEXITERATOR_TYPE::IndexIterator() : cur_page_id_(INVALID_PAGE_ID), leaf_(nullptr) {}

INDEX_TEMPLATE_ARGUMENTS
INDEXITERATOR_TYPE::IndexIterator(Page *page, int site, BufferPoolManager *buffer_pool_manager) {
  cur_site_ = site;
  buffer_pool_manager_ = buffer_pool_manager;
  if (page == nullptr) {
    cur_page_id_ = INVALID_PAGE_ID;
    is_end_ = true;
    return;
  }

  leaf_ = reinterpret_cast<B_PLUS_TREE_LEAF_PAGE_TYPE *>(page->GetData());
  cur_page_id_ = leaf_->GetPageId();
  cur_page_ = page;
  is_end_ = false;
}

INDEX_TEMPLATE_ARGUMENTS
INDEXITERATOR_TYPE::~IndexIterator() = default;  // NOLINT

INDEX_TEMPLATE_ARGUMENTS
auto INDEXITERATOR_TYPE::IsEnd() -> bool { return is_end_; }

INDEX_TEMPLATE_ARGUMENTS
auto INDEXITERATOR_TYPE::operator*() -> const MappingType & {
  assert(leaf_ != nullptr);
  if (is_end_) {
    throw Exception(ExceptionType::OUT_OF_RANGE, "Already get the last iterator, the iterator is out of range");
  }
  return leaf_->GetItem(cur_site_);
}

INDEX_TEMPLATE_ARGUMENTS
auto INDEXITERATOR_TYPE::operator++() -> INDEXITERATOR_TYPE & {
  assert(leaf_ != nullptr);
  if (is_end_) {
    throw Exception(ExceptionType::OUT_OF_RANGE, "Already get the last iterator, the iterator is out of range");
  }
  if (cur_site_ < leaf_->GetSize() - 1) {
    cur_site_++;
  } else if (leaf_->GetNextPageId() != INVALID_PAGE_ID) {
    cur_site_ = 0;
    cur_page_id_ = leaf_->GetNextPageId();
    cur_page_->RUnlatch();
    buffer_pool_manager_->UnpinPage(cur_page_->GetPageId(), false);

    cur_page_ = buffer_pool_manager_->FetchPage(cur_page_id_);
    cur_page_->RLatch();
    leaf_ = reinterpret_cast<B_PLUS_TREE_LEAF_PAGE_TYPE *>(cur_page_->GetData());
  } else {
    cur_page_->RUnlatch();
    buffer_pool_manager_->UnpinPage(cur_page_id_, false);
    cur_page_id_ = INVALID_PAGE_ID;
    cur_site_ = 0;
    is_end_ = true;
  }
  return *this;
}

template class IndexIterator<GenericKey<4>, RID, GenericComparator<4>>;

template class IndexIterator<GenericKey<8>, RID, GenericComparator<8>>;

template class IndexIterator<GenericKey<16>, RID, GenericComparator<16>>;

template class IndexIterator<GenericKey<32>, RID, GenericComparator<32>>;

template class IndexIterator<GenericKey<64>, RID, GenericComparator<64>>;

}  // namespace bustub
