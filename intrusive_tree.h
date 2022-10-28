#pragma once

#include <algorithm>

#include "intrusive_node.h"

namespace intrusive {

template <typename T, typename Compare,
          typename Tag = default_tag>
class intrusive_tree {
  using node_t = node<Tag>;
  static_assert(std::is_convertible_v<T*, node_t*>, "invalid value type");

  node_t* sentinel;
  Compare comp;
  size_t n_node = 0;

public:
  explicit intrusive_tree(node_t* sentinel, Compare compare = Compare{})
      : sentinel(sentinel), comp(std::move(compare)) {}

    intrusive_tree(intrusive_tree const& other) = delete;
    intrusive_tree(intrusive_tree&& other) = delete;

  intrusive_tree& operator=(intrusive_tree const& other) {
    if (this != &other)
      Tree(other).swap(*this);
    return *this;
  }

  intrusive_tree& operator=(intrusive_tree&& other) {
    if (this != &other)
      Tree(std::move(other)).swap(*this);
    return *this;
  }

  Compare get_compare() const {
    return this->comp;
  }

  void swap(intrusive_tree& other) {
    std::swap(this->n_node, other.n_node);
    std::swap(this->comp, other.comp);
    sentinel->swap(*other.sentinel);
  }

  bool empty() const {
    assert((sentinel->left == nullptr) == (n_node == 0));
    return sentinel->left == nullptr;
  }

  size_t size() const {
    return n_node;
  }

private:
  template <typename iT>
  class inorder_iterator {
  private:
    node_t* cur = nullptr;

    template <typename tT, typename tCompare, typename tTag>
    friend class intrusive_tree;

    template <typename tT, typename tCompare, typename tTag>
    static inorder_iterator
    begin_iter(const intrusive_tree<tT, tCompare, tTag>* tree) {
      return (node_t::min_node(tree->sentinel));
    }

    template <typename tT, typename tCompare, typename tTag>
    static inorder_iterator
    end_iter(const intrusive_tree<tT, tCompare, tTag>* tree) {
      return (tree->sentinel);
    }

  public:
    inorder_iterator(decltype(cur) cur) : cur(cur) {}

    using difference_type = ptrdiff_t;
    using value_type = iT;
    using pointer = value_type*;
    using reference = value_type&;
    using iterator_category = std::bidirectional_iterator_tag;

    inorder_iterator() = default;

    template <typename eq_iT>
    bool operator==(inorder_iterator<eq_iT> const& other) const {
      return cur == other.cur;
    }

    template <typename eq_iT>
    bool operator!=(inorder_iterator<eq_iT> const& other) const {
      return cur != other.cur;
    }

    reference operator*() const {
      return *(static_cast<iT*>(cur));
    }

    pointer operator->() const {
      return (static_cast<iT*>(cur));
    }

    inorder_iterator& operator++() {
      cur = cur->next();
      return *this;
    }

    inorder_iterator& operator--() {
      cur = cur->prev();
      return *this;
    }

    inorder_iterator operator++(int) {
      inorder_iterator res(*this);
      ++(*this);
      return res;
    }

    inorder_iterator operator--(int) {
      inorder_iterator res(*this);
      --(*this);
      return res;
    }
  };

public:
  using iterator = inorder_iterator<T>;
  using const_iterator = inorder_iterator<const T>;

  iterator begin() const {
    return iterator::template begin_iter<T, Compare, Tag>(this);
  }

  iterator end() const {
    return iterator::template end_iter<T, Compare, Tag>(this);
  }

  inline static const T* make_p(node_t* p) {
    return static_cast<T*>(p);
  }

  inline static const T& make_r(node_t& p) {
    return static_cast<T&>(p);
  }

private:
  enum find_result { THERE_IS, ADD_RIGHT, ADD_LEFT };
  node_t* find(const T& data, find_result& res) const {
    res = ADD_LEFT;
    if (sentinel->left == nullptr)
      return sentinel;

    node_t* cur = sentinel->left;
    while (cur != nullptr) {
      if (comp(make_r(*cur), data)) {
        if (cur->right)
          cur = cur->right;
        else {
          res = ADD_RIGHT;
          break;
        }
      } else if (comp(data, make_r(*cur))) {
        if (cur->left)
          cur = cur->left;
        else {
          res = ADD_LEFT;
          break;
        }
      } else {
        res = THERE_IS;
        break;
      }
    }
    return cur;
  }

public:
  iterator find(const T& x) const {
    find_result res = THERE_IS;
    node_t* cur = find(x, res);
    if (res == THERE_IS)
      return (cur);

    return end();
  }

  iterator find_next(const T& x) const {
    find_result res = THERE_IS;
    node_t* cur = find(x, res);
    if (res == THERE_IS || res == ADD_LEFT)
      return (cur);
    return (cur->next());
  }

  iterator insert(T& data) {
    find_result res = ADD_LEFT;
    node_t* cur = find(data, res);
    if (res == THERE_IS)
      return end();

    static_cast<node_t*>(&data)->parent = cur;
    n_node++;
    if (res == ADD_LEFT) {
      cur->left = &data;
      return (cur->left);
    } else { // if (res == ADD_RIGHT)  {
      cur->right = &data;
      return (cur->right);
    }
  }

  iterator remove(iterator it) {
    iterator it_next(it.cur->next());
    it.cur->unlink();
    n_node--;
    return it_next;
  }

  iterator remove(T& data) {
    find_result res = THERE_IS;
    node_t* cur = find(data, res);
    if (res == THERE_IS)
      return end();
    return remove(iterator(cur));
  }
};
} // namespace intrusive
