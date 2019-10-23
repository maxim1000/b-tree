#pragma once
#include <functional>
#include <vector>
template<typename Value>
class SortedArraySet
{
public:
    class Iterator;
    void insert(const Value&);
    void erase(const Value&);
    bool contains(const Value&) const;
    void enumerate(const std::function<void(const Value&)>&) const;
private:
    std::vector<Value> sortedArray_;
    size_t findIndexForValue(const Value &value) const;//returns first element>=value
};
///////////////////////////////////////////////////////////////////////////////
template<typename Value>
void SortedArraySet<Value>::insert(const Value &value)
{
    const auto index=findIndexForValue(value);
    if(index==sortedArray_.size() || sortedArray_[index]!=value)
        sortedArray_.insert(sortedArray_.begin()+index,value);
}
template<typename Value>
void SortedArraySet<Value>::erase(const Value &value)
{
    const auto index=findIndexForValue(value);
    if(index<sortedArray_.size() && sortedArray_[index]==value)
        sortedArray_.erase(sortedArray_.begin()+index);
}
template<typename Value>
bool SortedArraySet<Value>::contains(const Value &value) const
{
    const auto index=findIndexForValue(value);
    return (index<sortedArray_.size() && sortedArray_[index]==value);
}
template<typename Value>
void SortedArraySet<Value>::enumerate(const std::function<void(const Value&)> &processor) const
{
    for(const auto &value:sortedArray_)
        processor(value);
}
template<typename Value>
size_t SortedArraySet<Value>::findIndexForValue(const Value &value) const
{
    size_t current=sortedArray_.size();
    size_t step=sortedArray_.size();
    while(step>0)
    {
        if(current<step || sortedArray_[current-step]<value)
            step/=2;
        else
            current-=step;
    }
    return current;
}
