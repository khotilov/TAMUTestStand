/*
 * BasicTable.h
 *
 *  Created on: Sep 5, 2012
 *      Author: Austin
 */

#ifndef BASICTABLE_H_
#define BASICTABLE_H_

#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <iomanip>


namespace emu { namespace pc {

/** \class BasicTable
 *
 * Table container organized with labeled columns. Supports sorting by individual columns.
 * Sorting is stable although only comparisons using operator< are supported
 *
 * \author Austin Schneider
 *
 */

template <class T, class U  = std::string>
class BasicTable {

private:
  std::vector<U> columnLabels_;
  std::map<U, std::vector<T> > columns_;

  typedef typename std::vector<T>::iterator vec_T_iterator;
  typedef typename std::vector<T>::reverse_iterator vec_T_reverse_iterator;


  T * getElementPointer(size_t column, size_t row)
  {
    return getElementPointer(columnLabels_[column], row);
  }

  T * getElementPointer(U column, size_t row)
  {
    return & columns_[column][row];
  }

public:
  BasicTable() {};

  void setElement(size_t column, size_t row, T data);
  void setElement(U column, size_t row, T data);

  std::vector<U> getColumnLabels()
  {
    return columnLabels_;
  }

  struct row_iterator
  {
    BasicTable<T, U> * table;
    size_t row;

    row_iterator():table(),row(){}
    row_iterator(const row_iterator & i):table(i.table),row(i.row){}
    row_iterator & operator=(const row_iterator & i)
    {
      table = i.table;
      row = i.row;
      return *this;
    }
    bool operator==(const row_iterator & i)
    {
      return table==i.table && row==i.row;
    }
    bool operator!=(row_iterator i)
    {
      return !operator==(i);
    }
    T operator[](U column)
    {
      return table->getElement(column, row);
    }
    T operator[](size_t column)
    {
      return table->getElement(column, row);
    }
    row_iterator & operator++()
    {
      ++row;
      return *this;
    }
    row_iterator operator++(int)
    {
      row_iterator temp(*this);
      ++row;
      return temp;
    }
    row_iterator & operator--()
    {
      --row;
      return *this;
    }
    row_iterator operator--(int)
    {
      row_iterator temp(*this);
      --row;
      return temp;
    }
    row_iterator & operator+(size_t n)
    {
      row_iterator temp(*this);
      temp.row+=n;
      return temp;
    }
    row_iterator & operator+(row_iterator i)
    {
      row_iterator temp(*this);
      temp.row+=i.row;
      return temp;
    }
    row_iterator & operator-(size_t n)
    {
      row_iterator temp(*this);
      temp.row-=n;
      return temp;
    }
    row_iterator & operator-(row_iterator i)
    {
      row_iterator temp(*this);
      temp.row-=i.row;
      return temp;
    }
    bool operator<(row_iterator i)
    {
      return row < i.row;
    }
    bool operator>(row_iterator i)
    {
      return row > i.row;
    }
    bool operator<=(row_iterator i)
    {
      return row <= i.row;
    }
    bool operator>=(row_iterator i)
    {
      return row >= i.row;
    }
    row_iterator & operator+=(size_t n)
    {
      row+=n;
      return *this;
    }
    row_iterator & operator-=(size_t n)
    {
      row-=n;
      return *this;
    }
  };

  row_iterator getRowIterator(size_t row)
  {
    row_iterator it;
    it.table = this;
    it.row = row;
    return it;
  }

  T getElement(size_t column, size_t row)
  {
    return getElement(columnLabels_[column], row);
  }

  T getElement(U column, size_t row)
  {
    return columns_[column][row];
  }

  row_iterator begin()
  {
    return getRowIterator(0);
  }

  row_iterator end()
  {
    return getRowIterator(columns_[columnLabels_[0]].size());
  }

  /*
  vec_T_reverse_iterator columnRBegin(size_t column)
  {
    return columnRBegin(columnLabels_[column]);
  }

  vec_T_reverse_iterator columnREnd(size_t column)
  {
    return columnREnd(columnLabels_[column]);
  }
  */

  /*
  vec_T_reverse_iterator columnRBegin(U column)
  {
    return columns_[column].rbegin();
  }

  vec_T_reverse_iterator columnREnd(U column)
  {
    return columns_[column].rend();
  }
  */

  void addColumn(U label)
  {
    columns_.insert(std::make_pair(label, std::vector<T>()));
    columnLabels_.push_back(label);
  }

  void addColumn(U label, T* begin, T* end)
  {
    columns_.insert(std::make_pair(label, std::vector<T>()));

    for(T* it = begin; it!=begin; ++it)
    {
      columns_[label].push_back(*it);
    }
    columnLabels_.push_back(label);
  }

  void addColumn(U label, std::vector<T> contents)
  {
    columns_.insert(std::make_pair(label, contents));
  }

  int numColumns()
  {
    return columnLabels_.size();
  }

  void addRow()
  {
    typedef typename std::vector<U>::iterator iter;
    for(iter it = columnLabels_.begin(); it != columnLabels_.end(); ++it)
    {
      columns_[*it].push_back(T());
    }
  }
  void addRow(T * begin, T * end)
  {
    typedef typename std::vector<U>::iterator iter;
    T * p = begin;
    bool has_elements = true;

    for(iter it = columnLabels_.begin(); it != columnLabels_.end(); ++it, ++p)
    {
      has_elements &= p!=end;
      if(has_elements)
      {
        columns_[*it].push_back(*p);
      }
      else
      {
        columns_[*it].push_back(T());
      }
    }
  }

  void addRow(std::vector<T> contents)
  {
    typedef typename std::vector<U>::iterator iter;
    int i = 0;

    for(iter it = columnLabels_.begin(); it != columnLabels_.end(); ++it, ++i)
    {
      if(i<contents.size())
      {
        columns_[*it].push_back(contents[i]);
      }
      else
      {
        columns_[*it].push_back(T());
      }
    }
  }

  void removeColumn(size_t column)
  {
    columns_.erase(columnLabels_[column]);
    columnLabels_.erase(columnLabels_.begin()+column);
  }

  void removeColumn(U column)
  {
    typedef typename std::vector<U>::iterator iter;
    columns_.erase(columnLabels_[column]);
    for(iter it = columnLabels_.begin(); it != columnLabels_.end(); ++it)
    {
      if(*it == column)
      {
        columnLabels_.erase(it);
        break;
      }
    }
  }

  void removeRow(size_t row)
  {
    typedef typename std::map<U, std::vector<T> >::iterator iter;
    for(iter it = columns_.begin(); it != columns_.end(); ++it)
    {
      it->second.erase(it->begin() + row);
    }
  }

  int numRows()
  {
    if(numColumns() > 0)
      return columns_[columnLabels_[0]].size();
    else
      return 0;
  }

  void sortByColumn(size_t column, bool reverse = false)
  {
    sortByColumn(columnLabels_[column], reverse);
  }

  void sortByColumn(U column, bool reverse = false)
  {
    std::vector<std::pair<size_t, vec_T_iterator> > reference;

    vec_T_iterator it = columns_[column].begin();
    for(unsigned int i=0; i< columns_[column].size(); ++i, ++it)
    {
      reference.push_back(std::make_pair(i, it));
    }
    if(reverse)
      std::stable_sort(reference.begin(), reference.end(), reverse_ordering());
    else
      std::stable_sort(reference.begin(), reference.end(), ordering());

    typedef typename std::map<U, std::vector<T> >::iterator iter;
    for(iter it = columns_.begin(); it != columns_.end(); ++it)
    {
      it->second = sort_from_ref(it->second, reference);
    }
  }

private:
  struct ordering
  {
      bool operator()(std::pair<size_t, vec_T_iterator> const& a, std::pair<size_t, vec_T_iterator> const& b)
      {
          return *(a.second) < *(b.second);
      }
  };

  struct reverse_ordering
  {
      bool operator()(std::pair<size_t, vec_T_iterator> const& a, std::pair<size_t, vec_T_iterator> const& b)
      {
          return *(b.second) < *(a.second);
      }
  };

  std::vector<T> sort_from_ref(std::vector<T> const& in, std::vector<std::pair<size_t, vec_T_iterator> > const& reference)
  {
      std::vector<T> ret(in.size());

      size_t const size = in.size();
      for (size_t i = 0; i < size; ++i)
          ret[i] = in[reference[i].first];

      return ret;
  }

};

template<class T, class U>
std::ostream & operator<<(std::ostream & os, BasicTable<T, U> & table)
{
  typedef typename BasicTable<T, U>::row_iterator iterator;
  int num_columns = table.numColumns();
  os << std::setw(25) << table.getColumnLabels()[0];
  for(int i=1; i<num_columns; ++i)
  {
      os << '|' << std::setw(25) << table.getColumnLabels()[i];
  }
  os << std::endl;
  if(num_columns > 0)
  {
    iterator row = table.begin();
    iterator row_end = table.end();
    while(row!=row_end)
    {
      os << std::setw(25) << row[0];
      for(int i=1; i<num_columns; ++i)
      {
          os << '|' << std::setw(25) << row[i];
      }
      os << std::endl;
      ++row;
    }
  }
  return os;
}

} } //namespaces

#endif /* BASICTABLE_H_ */
