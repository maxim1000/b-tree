#pragma once
#include <algorithm>
#include <functional>
#include <variant>
#include <vector>
template<typename Value>
class BTree
{
public:
    class Iterator;
    BTree(size_t minChunkSize,size_t maxChunkSize);
    void insert(const Value&);
    void erase(const Value&);
    bool contains(const Value&) const;
    void enumerate(const std::function<void(const Value&)>&) const;
private:
    struct Node
    {
        std::vector<Value> values_;
        std::vector<Node> children_;
    };
    size_t minChunkSize_,maxChunkSize_;
    Node root_;
    void increaseDepthIfNeeded();
    void decreaseDepthIfNeeded();
    static void insert(const Value&,Node&,size_t maxChunkSize);
    static void erase(const Value&,Node&,size_t minChunkSize,size_t maxChunkSize);
    static bool contains(const Node&,const Value&);
    static void enumerate(const Node&,const std::function<void(const Value&)>&);
    static void splitChild(Node&,size_t childIndex);
    static void mergeChild(Node&,size_t childIndex);
    static size_t findIndexForValue(const std::vector<Value>&,const Value&);//returns first element>=value
    static const Value &getMinValue(const Node&);
    static void eraseFromChildWithRebalancing(const Value&,Node&,size_t childIndex,size_t minChunkSize,size_t maxChunkSize);
};
///////////////////////////////////////////////////////////////////////////////
template<typename Value>
BTree<Value>::BTree(size_t minChunkSize,size_t maxChunkSize)
    :minChunkSize_(minChunkSize)
    ,maxChunkSize_(maxChunkSize)
{}
template<typename Value>
void BTree<Value>::insert(const Value &value)
{
    insert(value,root_,maxChunkSize_);
    increaseDepthIfNeeded();
}
template<typename Value>
void BTree<Value>::erase(const Value &value)
{
    erase(value,root_,minChunkSize_,maxChunkSize_);
    decreaseDepthIfNeeded();
}
template<typename Value>
bool BTree<Value>::contains(const Value &value) const
{
    return contains(root_,value);
}
template<typename Value>
void BTree<Value>::enumerate(const std::function<void(const Value&)> &processor) const
{
    enumerate(root_,processor);
}
template<typename Value>
void BTree<Value>::increaseDepthIfNeeded()
{
    if(root_.values_.size()<=maxChunkSize_)
        return;
    Node newRoot;
    newRoot.children_.push_back(std::move(root_));
    root_=std::move(newRoot);
    splitChild(root_,0);
}
template<typename Value>
void BTree<Value>::decreaseDepthIfNeeded()
{
    if(root_.children_.size()!=1)
        return;
    auto newRoot=std::move(root_.children_.front());
    root_=std::move(newRoot);
}
template<typename Value>
void BTree<Value>::insert(const Value &value,Node &node,size_t maxChunkSize)
{
    auto &values=node.values_;
    const auto index=findIndexForValue(values,value);
    if(index<values.size() && values[index]==value)
        return;//the value is already in the container
    if(node.children_.empty())
    {//insert into the sorted array
        values.insert(values.begin()+index,value);
    }
    else
    {//insert into one of children
        insert(value,node.children_[index],maxChunkSize);
        if(node.children_[index].values_.size()>maxChunkSize)
            splitChild(node,index);
    }
}
template<typename Value>
void BTree<Value>::erase(const Value &value,Node &node,size_t minChunkSize,size_t maxChunkSize)
{
    auto &values=node.values_;
    if(node.children_.empty())
    {//erase it from the sorted array
        const auto index=findIndexForValue(values,value);
        if(index<values.size() && values[index]==value)
            values.erase(values.begin()+index);
    }
    else
    {
        const auto index=findIndexForValue(values,value);
        if(index<values.size() && values[index]==value)
        {//it's a separator, replace it with min value from the right child (and erase it from there)
            values[index]=getMinValue(node.children_[index+1]);
            eraseFromChildWithRebalancing(values[index],node,index+1,minChunkSize,maxChunkSize);
        }
        else
        {//erase the value from the corresponding child
            eraseFromChildWithRebalancing(value,node,index,minChunkSize,maxChunkSize);
        }
    }
}
template<typename Value>
bool BTree<Value>::contains(const Node &node,const Value &value)
{
    const auto &values=node.values_;
    const auto index=findIndexForValue(values,value);
    if(index<values.size() && values[index]==value)
        return true;
    else if(!node.children_.empty())
        return contains(node.children_[index],value);
    else
        return false;
}
template<typename Value>
void BTree<Value>::enumerate(const Node &node,const std::function<void(const Value&)> &processor)
{
    for(size_t index=0;index<node.values_.size();++index)
    {
        if(!node.children_.empty())
            enumerate(node.children_[index],processor);
        processor(node.values_[index]);
    }
    if(!node.children_.empty())
        enumerate(node.children_.back(),processor);
}
template<typename Value>
void BTree<Value>::splitChild(Node &node,size_t childIndex)
{
    node.children_.emplace(node.children_.begin()+childIndex+1);//do this at the start to not invalidate references later
    auto &child=node.children_[childIndex];
    size_t leftHalfSize=child.values_.size()/2;
    node.values_.insert(node.values_.begin()+childIndex,child.values_[leftHalfSize]);
    auto &secondChild=node.children_[childIndex+1];
    for(size_t index=leftHalfSize+1;index<child.values_.size();++index)
        secondChild.values_.push_back(std::move(child.values_[index]));
    child.values_.resize(leftHalfSize);
    if(!child.children_.empty())
    {
        for(size_t index=leftHalfSize+1;index<child.children_.size();++index)
            secondChild.children_.push_back(std::move(child.children_[index]));
        child.children_.resize(leftHalfSize+1);
    }
}
template<typename Value>
void BTree<Value>::mergeChild(Node &node,size_t childIndex)
{
    if(childIndex+1>=node.children_.size())
        --childIndex;
    else if(childIndex>0 && node.children_[childIndex-1].values_.size()>node.children_[childIndex+1].values_.size())
        --childIndex;
    auto &target=node.children_[childIndex];
    auto &source=node.children_[childIndex+1];
    target.values_.push_back(std::move(node.values_[childIndex]));
    node.values_.erase(node.values_.begin()+childIndex);
    for(auto &value:source.values_)
        target.values_.push_back(std::move(value));
    for(auto &child:source.children_)
        target.children_.push_back(std::move(child));
    node.children_.erase(node.children_.begin()+childIndex+1);
}
template<typename Value>
size_t BTree<Value>::findIndexForValue(const std::vector<Value> &values,const Value &value)
{
    size_t current=values.size();
    size_t step=values.size();
    while(step>0)
    {
        if(current<step || values[current-step]<value)
            step/=2;
        else
            current-=step;
    }
    return current;
}
template<typename Value>
const Value &BTree<Value>::getMinValue(const Node &node)
{
    if(node.children_.empty())
        return node.values_.front();
    else
        return getMinValue(node.children_.front());
}
template<typename Value>
void BTree<Value>::eraseFromChildWithRebalancing(
    const Value &value,
    Node &node,
    size_t childIndex,
    size_t minChunkSize,
    size_t maxChunkSize)
{
    erase(value,node.children_[childIndex],minChunkSize,maxChunkSize);
    if(node.children_[childIndex].values_.size()<minChunkSize && node.children_.size()>1)
    {
        mergeChild(node,childIndex);
        if(childIndex<node.children_.size() && node.children_[childIndex].values_.size()>maxChunkSize)
            splitChild(node,childIndex);
        else if(childIndex>0 && node.children_[childIndex-1].values_.size()>maxChunkSize)
            splitChild(node,childIndex-1);
    }
}
