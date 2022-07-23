//===----------------------------------------------------------------------===//
//
//                         CMU-DB Project (15-445/645)
//                         ***DO NO SHARE PUBLICLY***
//
// Identification: src/index/b_plus_tree.cpp
//
// Copyright (c) 2018, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include <string>

#include "common/exception.h"
#include "common/logger.h"
#include "common/rid.h"
#include "storage/index/b_plus_tree.h"
#include "storage/page/header_page.h"

namespace bustub {
INDEX_TEMPLATE_ARGUMENTS
BPLUSTREE_TYPE::BPlusTree(std::string name, BufferPoolManager *buffer_pool_manager, const KeyComparator &comparator,
                          int leaf_max_size, int internal_max_size)
    : index_name_(std::move(name)),
      root_page_id_(INVALID_PAGE_ID),
      buffer_pool_manager_(buffer_pool_manager),
      comparator_(comparator),
      leaf_max_size_(leaf_max_size),
      internal_max_size_(internal_max_size) {}

/*
 * Helper function to decide whether current b+tree is empty
 */
INDEX_TEMPLATE_ARGUMENTS
auto BPLUSTREE_TYPE::IsEmpty() const -> bool { return root_page_id_ == INVALID_PAGE_ID; }
/*****************************************************************************
 * SEARCH
 *****************************************************************************/
/*
 * Return the only value that associated with input key
 * This method is used for point query
 * @return : true means key exists
 */
INDEX_TEMPLATE_ARGUMENTS
auto BPLUSTREE_TYPE::GetValue(const KeyType &key, std::vector<ValueType> *result, Transaction *transaction) -> bool {
  rwlatch_.RLock();

  if (IsEmpty()) {
    rwlatch_.RUnlock();
    return false;
  }

  Page *page = nullptr;
  if (transaction == nullptr) {
    page = FindLeafPage(key, false);
  } else {
    page = FindLeafPage(key, OP_MODE::READ, transaction, false);
  }

  auto node = reinterpret_cast<BPlusTreeLeafPage<KeyType, RID, KeyComparator> *>(page->GetData());
  ValueType value = ValueType{};
  bool isExist = node->Lookup(key, &value, comparator_);
  if (isExist) {
    result->push_back(value);
  }

  if (transaction != nullptr) {
    UnlockPage(transaction, OP_MODE::READ, false);
  } else {
    page->RUnlatch();
    buffer_pool_manager_->UnpinPage(page->GetPageId(), false);
  }

  return isExist;
}

INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREE_TYPE::UnlockPage(Transaction *transaction, OP_MODE op_mode, bool is_dirty) {
  for (const auto &page : *transaction->GetPageSet()) {
    if (op_mode == OP_MODE::READ) {
      page->RUnlatch();
    } else {
      page->WUnlatch();
    }
    buffer_pool_manager_->UnpinPage(page->GetPageId(), is_dirty);
  }
  transaction->GetPageSet()->clear();

  for (const auto &page_id : *transaction->GetDeletedPageSet()) {
    buffer_pool_manager_->DeletePage(page_id);
  }
  transaction->GetDeletedPageSet()->clear();
}

INDEX_TEMPLATE_ARGUMENTS
bool BPLUSTREE_TYPE::IsSafe(OP_MODE op_mode, BPlusTreePage *node) {
  if (op_mode == OP_MODE::INSERT) {
    return node->GetSize() < node->GetMaxSize();
  }
  if (op_mode == OP_MODE::DELETE) {
    if (node->IsRootPage() && node->IsLeafPage()) {
      return node->GetSize() > 1;
    }
    if (node->IsRootPage() && !node->IsLeafPage()) {
      return node->GetSize() > 2;
    }
    if (node->IsLeafPage()) {
      return node->GetSize() >= node->GetMinSize();
    }
    return node->GetSize() > node->GetMinSize();
  }
  return true;
}

/*****************************************************************************
 * INSERTION
 *****************************************************************************/
/*
 * Insert constant key & value pair into b+ tree
 * if current tree is empty, start new tree, update root page id and insert
 * entry, otherwise insert into leaf page.
 * @return: since we only support unique key, if user try to insert duplicate
 * keys return false, otherwise return true.
 */
INDEX_TEMPLATE_ARGUMENTS
auto BPLUSTREE_TYPE::Insert(const KeyType &key, const ValueType &value, Transaction *transaction) -> bool {
  if (transaction == nullptr) { return false; }

  rwlatch_.WLock();
  if (IsEmpty()) {
    StartNewTree(key, value);
    rwlatch_.WUnlock();
    return true;
  }

  return InsertIntoLeaf(key, value, transaction);
}
/*
 * Insert constant key & value pair into an empty tree
 * User needs to first ask for new page from buffer pool manager(NOTICE: throw
 * an "out of memory" exception if returned value is nullptr), then update b+
 * tree's root page id and insert entry directly into leaf page.
 */
INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREE_TYPE::StartNewTree(const KeyType &key, const ValueType &value) {
  Page *page = buffer_pool_manager_->NewPage(&root_page_id_);
  if (page == nullptr) {
    throw Exception(ExceptionType::OUT_OF_MEMORY, "can't find a new page for the tree");
  }

  auto *root = reinterpret_cast<BPlusTreeLeafPage<KeyType, RID, KeyComparator> *>(page->GetData());
  UpdateRootPageId(true);
  root->Init(root_page_id_, INVALID_PAGE_ID, leaf_max_size_);
  root->SetPageType(IndexPageType::LEAF_PAGE);
  root->Insert(key, value, comparator_);
  buffer_pool_manager_->UnpinPage(root_page_id_, true);
}

/*
 * Insert constant key & value pair into leaf page
 * User needs to first find the right leaf page as insertion target, then look
 * through leaf page to see whether insert key exist or not. If exist, return
 * immdiately, otherwise insert entry. Remember to deal with split if necessary.
 * @return: since we only support unique key, if user try to insert duplicate
 * keys return false, otherwise return true.
 */
INDEX_TEMPLATE_ARGUMENTS
auto BPLUSTREE_TYPE::InsertIntoLeaf(const KeyType &key, const ValueType &value, Transaction *transaction) -> bool {
  Page *page = FindLeafPage(key, OP_MODE::INSERT, transaction, false);

  auto *leaf = reinterpret_cast<BPlusTreeLeafPage<KeyType, RID, KeyComparator> *>(page->GetData());
  ValueType old_value;
  if (leaf->Lookup(key, &old_value, comparator_)) {
    UnlockPage(transaction, OP_MODE::INSERT, false);
    return false;
  }

  if (leaf->GetSize() < leaf->GetMaxSize()) {
    leaf->Insert(key, value, comparator_);
  } else {
    leaf->Insert(key, value, comparator_);
    B_PLUS_TREE_LEAF_PAGE_TYPE *new_leaf = Split(leaf);
    InsertIntoParent(leaf, new_leaf->KeyAt(0), new_leaf, transaction);
    buffer_pool_manager_->UnpinPage(new_leaf->GetPageId(), true);
  }

  UnlockPage(transaction, OP_MODE::INSERT, true);
  return true;
}

/*
 * Split input page and return newly created page.
 * Using template N to represent either internal page or leaf page.
 * User needs to first ask for new page from buffer pool manager(NOTICE: throw
 * an "out of memory" exception if returned value is nullptr), then move half
 * of key & value pairs from input page to newly created page
 */
INDEX_TEMPLATE_ARGUMENTS
template <typename N>
auto BPLUSTREE_TYPE::Split(N *node) -> N * {
  page_id_t page_id;
  Page *page = buffer_pool_manager_->NewPage(&page_id);
  if (page == nullptr) {
    throw Exception(ExceptionType::OUT_OF_MEMORY, "can't find a new page for the tree");
    return nullptr;
  }

  N *new_node = reinterpret_cast<N *>(page->GetData());
  new_node->SetPageId(page_id);
  if (node->IsLeafPage()) {
    auto *leaf = reinterpret_cast<BPlusTreeLeafPage<KeyType, RID, KeyComparator> *>(node);
    new_node->SetMaxSize(leaf_max_size_);
    auto *new_leaf = reinterpret_cast<BPlusTreeLeafPage<KeyType, RID, KeyComparator> *>(new_node);
    new_leaf->SetPageType(IndexPageType::LEAF_PAGE);
    new_leaf->SetNextPageId(INVALID_PAGE_ID);
    leaf->MoveHalfTo(new_leaf);
  } else {
    auto *internal = reinterpret_cast<BPlusTreeInternalPage<KeyType, page_id_t, KeyComparator> *>(node);
    new_node->SetMaxSize(internal_max_size_);
    auto *new_internal = reinterpret_cast<BPlusTreeInternalPage<KeyType, page_id_t, KeyComparator> *>(new_node);
    new_internal->SetPageType(IndexPageType::INTERNAL_PAGE);
    internal->MoveHalfTo(new_internal, buffer_pool_manager_);
  }

  return new_node;
}

/*
 * Insert key & value pair into internal page after split
 * @param   old_node      input page from split() method
 * @param   key
 * @param   new_node      returned page from split() method
 * User needs to first find the parent page of old_node, parent node must be
 * adjusted to take info of new_node into account. Remember to deal with split
 * recursively if necessary.
 */
INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREE_TYPE::InsertIntoParent(BPlusTreePage *old_node, const KeyType &key, BPlusTreePage *new_node,
                                      Transaction *transaction) {
  if (old_node->GetParentPageId() == INVALID_PAGE_ID) {
    Page *new_root = buffer_pool_manager_->NewPage(&root_page_id_);
    if (new_root == nullptr) {
      throw Exception(ExceptionType::OUT_OF_MEMORY, "can't find a new page for the tree");
    }

    auto parent = reinterpret_cast<BPlusTreeInternalPage<KeyType, page_id_t, KeyComparator> *>(new_root->GetData());

    parent->Init(root_page_id_, INVALID_PAGE_ID, internal_max_size_);
    parent->PopulateNewRoot(old_node->GetPageId(), key, new_node->GetPageId());
    old_node->SetParentPageId(root_page_id_);
    new_node->SetParentPageId(root_page_id_);

    UpdateRootPageId(false);

    buffer_pool_manager_->UnpinPage(root_page_id_, true);
    return;
  }

  Page *page = buffer_pool_manager_->FetchPage(old_node->GetParentPageId());
  auto parent = reinterpret_cast<BPlusTreeInternalPage<KeyType, page_id_t, KeyComparator> *>(page->GetData());
  new_node->SetParentPageId(parent->GetPageId());
  buffer_pool_manager_->UnpinPage(new_node->GetPageId(), true);
  buffer_pool_manager_->UnpinPage(old_node->GetPageId(), true);
  if (parent->GetSize() < parent->GetMaxSize()) {
    parent->InsertNodeAfter(old_node->GetPageId(), key, new_node->GetPageId());
    buffer_pool_manager_->UnpinPage(parent->GetPageId(), true);
  } else {
    parent->InsertNodeAfter(old_node->GetPageId(), key, new_node->GetPageId());
    auto new_p_node = Split(parent);
    InsertIntoParent(parent, new_p_node->KeyAt(0), new_p_node, transaction);
    buffer_pool_manager_->UnpinPage(parent->GetPageId(), true);
  }
}

/*****************************************************************************
 * REMOVE
 *****************************************************************************/
/*
 * Delete key & value pair associated with input key
 * If current tree is empty, return immdiately.
 * If not, User needs to first find the right leaf page as deletion target, then
 * delete entry from leaf page. Remember to deal with redistribute or merge if
 * necessary.
 */
INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREE_TYPE::Remove(const KeyType &key, Transaction *transaction) {
  rwlatch_.WLock();

  if (IsEmpty()) {
    rwlatch_.WUnlock();
    return;
  }

  Page *page = FindLeafPage(key, OP_MODE::DELETE, transaction, false);
  if (page == nullptr) {
    return;
  }
  auto node = reinterpret_cast<BPlusTreeLeafPage<KeyType, RID, KeyComparator> *>(page->GetData());

  int key_index = node->KeyWhere(key, comparator_);

  if (key_index == node->GetSize()) {
    UnlockPage(transaction, OP_MODE::DELETE, false);
    return;
  }

  node->RemoveAndDeleteRecord(key, comparator_);

  CoalesceOrRedistribute(node, transaction);
  UnlockPage(transaction, OP_MODE::DELETE, true);
}

/*
 * User needs to first find the sibling of input page. If sibling's size + input
 * page's size > page's max size, then redistribute. Otherwise, merge.
 * Using template N to represent either internal page or leaf page.
 * @return: true means target leaf page should be deleted, false means no
 * deletion happens
 */
INDEX_TEMPLATE_ARGUMENTS
template <typename N>
auto BPLUSTREE_TYPE::CoalesceOrRedistribute(N *node, Transaction *transaction) -> bool {
  if (node->IsRootPage()) {
    if (AdjustRoot(node)) {
      transaction->AddIntoDeletedPageSet(node->GetPageId());
      return true;
    }
    return false;
  }

  if (node->IsLeafPage()) {
    if (node->GetSize() >= node->GetMinSize()) {
      return false;
    }
  } else {
    if (node->GetSize() > node->GetMinSize()) {
      return false;
    }
  }

  Page *p_page = buffer_pool_manager_->FetchPage(node->GetParentPageId());
  auto *parent = reinterpret_cast<BPlusTreeInternalPage<KeyType, page_id_t, KeyComparator> *>(p_page->GetData());

  page_id_t sibling_id = INVALID_PAGE_ID;
  int index = parent->ValueIndex(node->GetPageId());
  if (index == 0) {
    sibling_id = parent->ValueAt(index + 1);
  } else {
    sibling_id = parent->ValueAt(index - 1);
  }

  Page *s_page = buffer_pool_manager_->FetchPage(sibling_id);
  N *sibling = reinterpret_cast<N *>(s_page->GetData());
  if (sibling->GetSize() + node->GetSize() > node->GetMaxSize()) {
    Redistribute(sibling, node, index);
    buffer_pool_manager_->UnpinPage(s_page->GetPageId(), true);
    buffer_pool_manager_->UnpinPage(p_page->GetPageId(), true);
    return false;
  }
  if (index == 0) {
    Coalesce(&node, &sibling, &parent, 1, transaction);
    buffer_pool_manager_->UnpinPage(p_page->GetPageId(), true);
    buffer_pool_manager_->UnpinPage(s_page->GetPageId(), false);
    return true;
  }
  Coalesce(&sibling, &node, &parent, index, transaction);
  buffer_pool_manager_->UnpinPage(p_page->GetPageId(), true);
  buffer_pool_manager_->UnpinPage(s_page->GetPageId(), true);
  return true;
}

/*
 * Move all the key & value pairs from one page to its sibling page, and notify
 * buffer pool manager to delete this page. Parent page must be adjusted to
 * take info of deletion into account. Remember to deal with coalesce or
 * redistribute recursively if necessary.
 * Using template N to represent either internal page or leaf page.
 * @param   neighbor_node      sibling page of input "node"
 * @param   node               input from method coalesceOrRedistribute()
 * @param   parent             parent page of input "node"
 * @return  true means parent node should be deleted, false means no deletion
 * happend
 */
INDEX_TEMPLATE_ARGUMENTS
template <typename N>
auto BPLUSTREE_TYPE::Coalesce(N **neighbor_node, N **node,
                              BPlusTreeInternalPage<KeyType, page_id_t, KeyComparator> **parent, int index,
                              Transaction *transaction) -> bool {
  if ((*node)->IsLeafPage()) {
    auto leaf = reinterpret_cast<BPlusTreeLeafPage<KeyType, RID, KeyComparator> *>(*node);
    auto new_leaf = reinterpret_cast<BPlusTreeLeafPage<KeyType, RID, KeyComparator> *>(*neighbor_node);
    leaf->MoveAllTo(new_leaf);
    transaction->AddIntoDeletedPageSet(leaf->GetPageId());
  } else {
    auto internal = reinterpret_cast<BPlusTreeInternalPage<KeyType, page_id_t, KeyComparator> *>(*node);
    auto new_internal = reinterpret_cast<BPlusTreeInternalPage<KeyType, page_id_t, KeyComparator> *>(*neighbor_node);
    internal->MoveAllTo(new_internal, (*parent)->KeyAt(index), buffer_pool_manager_);
    transaction->AddIntoDeletedPageSet(internal->GetPageId());
  }

  (*parent)->Remove(index);

  return CoalesceOrRedistribute(*parent, transaction);
}

/*
 * Redistribute key & value pairs from one page to its sibling page. If index ==
 * 0, move sibling page's first key & value pair into end of input "node",
 * otherwise move sibling page's last key & value pair into head of input
 * "node".
 * Using template N to represent either internal page or leaf page.
 * @param   neighbor_node      sibling page of input "node"
 * @param   node               input from method coalesceOrRedistribute()
 */
INDEX_TEMPLATE_ARGUMENTS
template <typename N>
void BPLUSTREE_TYPE::Redistribute(N *neighbor_node, N *node, int index) {
  Page *p_page = buffer_pool_manager_->FetchPage(node->GetParentPageId());
  auto *parent = reinterpret_cast<BPlusTreeInternalPage<KeyType, page_id_t, KeyComparator> *>(p_page->GetData());

  if (neighbor_node->IsLeafPage()) {
    auto leaf = reinterpret_cast<BPlusTreeLeafPage<KeyType, RID, KeyComparator> *>(node);
    auto new_leaf = reinterpret_cast<BPlusTreeLeafPage<KeyType, RID, KeyComparator> *>(neighbor_node);
    if (index == 0) {
      new_leaf->MoveFirstToEndOf(leaf);
      parent->SetKeyAt(1, new_leaf->KeyAt(0));
    } else {
      new_leaf->MoveLastToFrontOf(leaf);
      parent->SetKeyAt(index, leaf->KeyAt(0));
    }
  } else {
    auto internal = reinterpret_cast<BPlusTreeInternalPage<KeyType, page_id_t, KeyComparator> *>(node);
    auto new_internal = reinterpret_cast<BPlusTreeInternalPage<KeyType, page_id_t, KeyComparator> *>(neighbor_node);
    if (index == 0) {
      new_internal->MoveFirstToEndOf(internal, parent->KeyAt(1), buffer_pool_manager_);
      parent->SetKeyAt(1, new_internal->KeyAt(0));
    } else {
      new_internal->MoveLastToFrontOf(internal, parent->KeyAt(index), buffer_pool_manager_);
      parent->SetKeyAt(index, internal->KeyAt(0));
    }
  }

  buffer_pool_manager_->UnpinPage(p_page->GetPageId(), true);
}
/*
 * Update root page if necessary
 * NOTE: size of root page can be less than min size and this method is only
 * called within coalesceOrRedistribute() method
 * case 1: when you delete the last element in root page, but root page still
 * has one last child
 * case 2: when you delete the last element in whole b+ tree
 * @return : true means root page should be deleted, false means no deletion
 * happend
 */
INDEX_TEMPLATE_ARGUMENTS
auto BPLUSTREE_TYPE::AdjustRoot(BPlusTreePage *old_root_node) -> bool {
  if (old_root_node->IsLeafPage() && old_root_node->GetSize() == 0) {
    root_page_id_ = INVALID_PAGE_ID;
    UpdateRootPageId(false);
    return true;
  }

  if (!old_root_node->IsLeafPage() && old_root_node->GetSize() == 1) {
    auto old_root = reinterpret_cast<BPlusTreeInternalPage<KeyType, page_id_t, KeyComparator> *>(old_root_node);
    root_page_id_ = old_root->ValueAt(0);
    UpdateRootPageId(false);
    Page *page = buffer_pool_manager_->FetchPage(root_page_id_);
    auto new_root = reinterpret_cast<BPlusTreeInternalPage<KeyType, page_id_t, KeyComparator> *>(page->GetData());
    new_root->SetParentPageId(INVALID_PAGE_ID);
    buffer_pool_manager_->UnpinPage(page->GetPageId(), true);
    return true;
  }
  return false;
}

/*****************************************************************************
 * INDEX ITERATOR
 *****************************************************************************/
/*
 * Input parameter is void, find the leaftmost leaf page first, then construct
 * index iterator
 * @return : index iterator
 */
INDEX_TEMPLATE_ARGUMENTS
auto BPLUSTREE_TYPE::Begin() -> INDEXITERATOR_TYPE {
  rwlatch_.RLock();
  if (IsEmpty()) {
    rwlatch_.RUnlock();
    return INDEXITERATOR_TYPE();
  }

  Page *page = FindLeafPage(KeyType{}, true);
  return INDEXITERATOR_TYPE(page, 0, buffer_pool_manager_);
}

/*
 * Input parameter is low key, find the leaf page that contains the input key
 * first, then construct index iterator
 * @return : index iterator
 */
INDEX_TEMPLATE_ARGUMENTS
auto BPLUSTREE_TYPE::Begin(const KeyType &key) -> INDEXITERATOR_TYPE {
  rwlatch_.RLock();
  if (IsEmpty()) {
    rwlatch_.RUnlock();
    return INDEXITERATOR_TYPE();
  }

  Page *page = FindLeafPage(key, false);
  auto node = reinterpret_cast<BPlusTreeLeafPage<KeyType, RID, KeyComparator> *>(page->GetData());
  int index = node->KeyIndex(key, comparator_);
  return INDEXITERATOR_TYPE(page, index, buffer_pool_manager_);
}

/*
 * Input parameter is void, construct an index iterator representing the end
 * of the key/value pair in the leaf node
 * @return : index iterator
 */
INDEX_TEMPLATE_ARGUMENTS
auto BPLUSTREE_TYPE::End() -> INDEXITERATOR_TYPE { return INDEXITERATOR_TYPE(nullptr, 0, buffer_pool_manager_); }

/*****************************************************************************
 * UTILITIES AND DEBUG
 *****************************************************************************/
/*
 * Find leaf page containing particular key, if leftMost flag == true, find
 * the left most leaf page
 */
INDEX_TEMPLATE_ARGUMENTS
auto BPLUSTREE_TYPE::FindLeafPage(const KeyType &key, bool leftMost) -> Page * {
  page_id_t page_id = root_page_id_;
  Page *pre_page = nullptr;
  while (true) {
    Page *page = buffer_pool_manager_->FetchPage(page_id);

    page->RLatch();

    auto node = reinterpret_cast<BPlusTreePage *>(page->GetData());

    if (pre_page != nullptr) {
      pre_page->RUnlatch();
      buffer_pool_manager_->UnpinPage(pre_page->GetPageId(), false);
    } else {
      rwlatch_.RUnlock();
    }
    pre_page = page;

    if (node->IsLeafPage()) {
      return page;
    }
    auto internal = reinterpret_cast<BPlusTreeInternalPage<KeyType, page_id_t, KeyComparator> *>(node);
    if (leftMost) {
      page_id = internal->ValueAt(0);
    } else {
      page_id = internal->Lookup(key, comparator_);
    }
  }
}

INDEX_TEMPLATE_ARGUMENTS
Page *BPLUSTREE_TYPE::FindLeafPage(const KeyType &key, OP_MODE op_mode, Transaction *transaction, bool leftMost) {
  page_id_t page_id = root_page_id_;
  Page *pre_page = nullptr;
  while (true) {
    Page *page = buffer_pool_manager_->FetchPage(page_id);

    if (op_mode == OP_MODE::READ) {
      page->RLatch();
    } else {
      page->WLatch();
    }

    auto node = reinterpret_cast<BPlusTreePage *>(page->GetData());

    if (pre_page == nullptr) {
      if (op_mode == OP_MODE::READ) {
        rwlatch_.RUnlock();
      } else {
        rwlatch_.WUnlock();
      }
    }

    pre_page = page;
    if (IsSafe(op_mode, node)) {
      UnlockPage(transaction, op_mode, false);
    }
    transaction->AddIntoPageSet(page);

    if (node->IsLeafPage()) {
      return page;
    }
    auto internal = reinterpret_cast<BPlusTreeInternalPage<KeyType, page_id_t, KeyComparator> *>(node);
    if (leftMost) {
      page_id = internal->ValueAt(0);
    } else {
      page_id = internal->Lookup(key, comparator_);
    }
  }
}

/*
 * Update/Insert root page id in header page(where page_id = 0, header_page is
 * defined under include/page/header_page.h)
 * Call this method everytime root page id is changed.
 * @parameter: insert_record      defualt value is false. When set to true,
 * insert a record <index_name, root_page_id> into header page instead of
 * updating it.
 */
INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREE_TYPE::UpdateRootPageId(int insert_record) {
  HeaderPage *header_page = static_cast<HeaderPage *>(buffer_pool_manager_->FetchPage(HEADER_PAGE_ID));
  if (insert_record != 0) {
    // create a new record<index_name + root_page_id> in header_page
    header_page->InsertRecord(index_name_, root_page_id_);
  } else {
    // update root_page_id in header_page
    header_page->UpdateRecord(index_name_, root_page_id_);
  }
  buffer_pool_manager_->UnpinPage(HEADER_PAGE_ID, true);
}

/*
 * This method is used for test only
 * Read data from file and insert one by one
 */
INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREE_TYPE::InsertFromFile(const std::string &file_name, Transaction *transaction) {
  int64_t key;
  std::ifstream input(file_name);
  while (input) {
    input >> key;

    KeyType index_key;
    index_key.SetFromInteger(key);
    RID rid(key);
    Insert(index_key, rid, transaction);
  }
}
/*
 * This method is used for test only
 * Read data from file and remove one by one
 */
INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREE_TYPE::RemoveFromFile(const std::string &file_name, Transaction *transaction) {
  int64_t key;
  std::ifstream input(file_name);
  while (input) {
    input >> key;
    KeyType index_key;
    index_key.SetFromInteger(key);
    Remove(index_key, transaction);
  }
}

/**
 * This method is used for debug only, You don't need to modify
 */
INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREE_TYPE::Draw(BufferPoolManager *bpm, const std::string &outf) {
  if (IsEmpty()) {
    LOG_WARN("Draw an empty tree");
    return;
  }
  std::ofstream out(outf);
  out << "digraph G {" << std::endl;
  ToGraph(reinterpret_cast<BPlusTreePage *>(bpm->FetchPage(root_page_id_)->GetData()), bpm, out);
  out << "}" << std::endl;
  out.close();
}

/**
 * This method is used for debug only, You don't need to modify
 */
INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREE_TYPE::Print(BufferPoolManager *bpm) {
  if (IsEmpty()) {
    LOG_WARN("Print an empty tree");
    return;
  }
  ToString(reinterpret_cast<BPlusTreePage *>(bpm->FetchPage(root_page_id_)->GetData()), bpm);
}

/**
 * This method is used for debug only, You don't need to modify
 * @tparam KeyType
 * @tparam ValueType
 * @tparam KeyComparator
 * @param page
 * @param bpm
 * @param out
 */
INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREE_TYPE::ToGraph(BPlusTreePage *page, BufferPoolManager *bpm, std::ofstream &out) const {
  std::string leaf_prefix("LEAF_");
  std::string internal_prefix("INT_");
  if (page->IsLeafPage()) {
    LeafPage *leaf = reinterpret_cast<LeafPage *>(page);
    // Print node name
    out << leaf_prefix << leaf->GetPageId();
    // Print node properties
    out << "[shape=plain color=green ";
    // Print data of the node
    out << "label=<<TABLE BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING=\"0\" CELLPADDING=\"4\">\n";
    // Print data
    out << "<TR><TD COLSPAN=\"" << leaf->GetSize() << "\">P=" << leaf->GetPageId() << "</TD></TR>\n";
    out << "<TR><TD COLSPAN=\"" << leaf->GetSize() << "\">"
        << "max_size=" << leaf->GetMaxSize() << ",min_size=" << leaf->GetMinSize() << ",size=" << leaf->GetSize()
        << "</TD></TR>\n";
    out << "<TR>";
    for (int i = 0; i < leaf->GetSize(); i++) {
      out << "<TD>" << leaf->KeyAt(i) << "</TD>\n";
    }
    out << "</TR>";
    // Print table end
    out << "</TABLE>>];\n";
    // Print Leaf node link if there is a next page
    if (leaf->GetNextPageId() != INVALID_PAGE_ID) {
      out << leaf_prefix << leaf->GetPageId() << " -> " << leaf_prefix << leaf->GetNextPageId() << ";\n";
      out << "{rank=same " << leaf_prefix << leaf->GetPageId() << " " << leaf_prefix << leaf->GetNextPageId() << "};\n";
    }

    // Print parent links if there is a parent
    if (leaf->GetParentPageId() != INVALID_PAGE_ID) {
      out << internal_prefix << leaf->GetParentPageId() << ":p" << leaf->GetPageId() << " -> " << leaf_prefix
          << leaf->GetPageId() << ";\n";
    }
  } else {
    InternalPage *inner = reinterpret_cast<InternalPage *>(page);
    // Print node name
    out << internal_prefix << inner->GetPageId();
    // Print node properties
    out << "[shape=plain color=pink ";  // why not?
    // Print data of the node
    out << "label=<<TABLE BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING=\"0\" CELLPADDING=\"4\">\n";
    // Print data
    out << "<TR><TD COLSPAN=\"" << inner->GetSize() << "\">P=" << inner->GetPageId() << "</TD></TR>\n";
    out << "<TR><TD COLSPAN=\"" << inner->GetSize() << "\">"
        << "max_size=" << inner->GetMaxSize() << ",min_size=" << inner->GetMinSize() << ",size=" << inner->GetSize()
        << "</TD></TR>\n";
    out << "<TR>";
    for (int i = 0; i < inner->GetSize(); i++) {
      out << "<TD PORT=\"p" << inner->ValueAt(i) << "\">";
      if (i > 0) {
        out << inner->KeyAt(i);
      } else {
        out << " ";
      }
      out << "</TD>\n";
    }
    out << "</TR>";
    // Print table end
    out << "</TABLE>>];\n";
    // Print Parent link
    if (inner->GetParentPageId() != INVALID_PAGE_ID) {
      out << internal_prefix << inner->GetParentPageId() << ":p" << inner->GetPageId() << " -> " << internal_prefix
          << inner->GetPageId() << ";\n";
    }
    // Print leaves
    for (int i = 0; i < inner->GetSize(); i++) {
      auto child_page = reinterpret_cast<BPlusTreePage *>(bpm->FetchPage(inner->ValueAt(i))->GetData());
      ToGraph(child_page, bpm, out);
      if (i > 0) {
        auto sibling_page = reinterpret_cast<BPlusTreePage *>(bpm->FetchPage(inner->ValueAt(i - 1))->GetData());
        if (!sibling_page->IsLeafPage() && !child_page->IsLeafPage()) {
          out << "{rank=same " << internal_prefix << sibling_page->GetPageId() << " " << internal_prefix
              << child_page->GetPageId() << "};\n";
        }
        bpm->UnpinPage(sibling_page->GetPageId(), false);
      }
    }
  }
  bpm->UnpinPage(page->GetPageId(), false);
}

/**
 * This function is for debug only, you don't need to modify
 * @tparam KeyType
 * @tparam ValueType
 * @tparam KeyComparator
 * @param page
 * @param bpm
 */
INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREE_TYPE::ToString(BPlusTreePage *page, BufferPoolManager *bpm) const {
  if (page->IsLeafPage()) {
    LeafPage *leaf = reinterpret_cast<LeafPage *>(page);
    std::cout << "Leaf Page: " << leaf->GetPageId() << " parent: " << leaf->GetParentPageId()
              << " next: " << leaf->GetNextPageId() << std::endl;
    for (int i = 0; i < leaf->GetSize(); i++) {
      std::cout << leaf->KeyAt(i) << ",";
    }
    std::cout << std::endl;
    std::cout << std::endl;
  } else {
    InternalPage *internal = reinterpret_cast<InternalPage *>(page);
    std::cout << "Internal Page: " << internal->GetPageId() << " parent: " << internal->GetParentPageId() << std::endl;
    for (int i = 0; i < internal->GetSize(); i++) {
      std::cout << internal->KeyAt(i) << ": " << internal->ValueAt(i) << ",";
    }
    std::cout << std::endl;
    std::cout << std::endl;
    for (int i = 0; i < internal->GetSize(); i++) {
      ToString(reinterpret_cast<BPlusTreePage *>(bpm->FetchPage(internal->ValueAt(i))->GetData()), bpm);
    }
  }
  bpm->UnpinPage(page->GetPageId(), false);
}

template class BPlusTree<GenericKey<4>, RID, GenericComparator<4>>;
template class BPlusTree<GenericKey<8>, RID, GenericComparator<8>>;
template class BPlusTree<GenericKey<16>, RID, GenericComparator<16>>;
template class BPlusTree<GenericKey<32>, RID, GenericComparator<32>>;
template class BPlusTree<GenericKey<64>, RID, GenericComparator<64>>;

}  // namespace bustub
