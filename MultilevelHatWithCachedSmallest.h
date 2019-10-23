#pragma once
#include <algorithm>
#include <functional>
#include <variant>
#include <vector>
template<typename Value>
class MultilevelHatWithCachedSmallest
{
public:
    class Iterator;
    MultilevelHatWithCachedSmallest(size_t minChunkSize,size_t maxChunkSize);
    void insert(const Value&);
    void erase(const Value&);
    bool contains(const Value&) const;
    void enumerate(const std::function<void(const Value&)>&) const;
private:
    using Leaf=std::vector<Value>;
    struct Node
    {
        std::variant<Leaf,std::vector<Node>> content_;
        Value smallest_;
    };
    size_t minChunkSize_,maxChunkSize_;
    Node root_;
    void insert(const Value&,Node&);
    void increaseDepthIfNeeded();
    void erase(const Value&,Node&);
    void decreaseDepthIfNeeded();
    static size_t findIndexForValue(const Leaf&,const Value &value);//returns first element>=value
    static size_t findChildIndexForValue(const std::vector<Node>&,const Value &value);//last child with smallest<=value
    static size_t getNodeSize(const Node&);
    static void splitChild(std::vector<Node>&,size_t childIndex);
    static void mergeChild(std::vector<Node>&,size_t childIndex);
    static bool contains(const Node&,const Value&);
    static void enumerate(const Node&,const std::function<void(const Value&)>&);
};
///////////////////////////////////////////////////////////////////////////////
template<typename Value>
MultilevelHatWithCachedSmallest<Value>::MultilevelHatWithCachedSmallest(size_t minChunkSize,size_t maxChunkSize)
    :minChunkSize_(minChunkSize)
    ,maxChunkSize_(maxChunkSize)
{}
template<typename Value>
void MultilevelHatWithCachedSmallest<Value>::insert(const Value &value)
{
    insert(value,root_);
    increaseDepthIfNeeded();
}
template<typename Value>
void MultilevelHatWithCachedSmallest<Value>::erase(const Value &value)
{
    erase(value,root_);
    decreaseDepthIfNeeded();
}
template<typename Value>
bool MultilevelHatWithCachedSmallest<Value>::contains(const Value &value) const
{
    return contains(root_,value);
}
template<typename Value>
void MultilevelHatWithCachedSmallest<Value>::enumerate(const std::function<void(const Value&)> &processor) const
{
    enumerate(root_,processor);
}
template<typename Value>
void MultilevelHatWithCachedSmallest<Value>::insert(const Value &value,Node &node)
{
    if(auto *leaf=std::get_if<Leaf>(&node.content_))
    {
        leaf->insert(leaf->begin()+findIndexForValue(*leaf,value),value);
        node.smallest_=leaf->front();
    }
    else if(auto *children=std::get_if<std::vector<Node>>(&node.content_))
    {
        const auto index=findChildIndexForValue(*children,value);
        insert(value,(*children)[index]);
        node.smallest_=children->front().smallest_;
        if(getNodeSize((*children)[index])>maxChunkSize_)
            splitChild(*children,index);
    }
}
template<typename Value>
void MultilevelHatWithCachedSmallest<Value>::increaseDepthIfNeeded()
{
    if(getNodeSize(root_)<=maxChunkSize_)
        return;
    auto smallest=root_.smallest_;
    std::vector<Node> newRootChildren;
    newRootChildren.push_back(std::move(root_));
    splitChild(newRootChildren,0);
    root_.content_=std::move(newRootChildren);
    root_.smallest_=std::move(smallest);
}
template<typename Value>
void MultilevelHatWithCachedSmallest<Value>::erase(const Value &value,Node &node)
{
    if(auto *leaf=std::get_if<Leaf>(&node.content_))
    {
        const auto index=findIndexForValue(*leaf,value);
        if(index!=leaf->size() && (*leaf)[index]==value)
            leaf->erase(leaf->begin()+index);
        if(!leaf->empty())
            node.smallest_=leaf->front();
    }
    else if(auto *children=std::get_if<std::vector<Node>>(&node.content_))
    {
        const auto index=findChildIndexForValue(*children,value);
        erase(value,(*children)[index]);
        node.smallest_=children->front().smallest_;
        if(getNodeSize((*children)[index])<minChunkSize_)
        {
            mergeChild(*children,index);
            if(index<children->size() && getNodeSize((*children)[index])>maxChunkSize_)
                splitChild(*children,index);
            else//we might merge to the previous node
                if(index>0 && getNodeSize((*children)[index-1])>maxChunkSize_)
                    splitChild(*children,index-1);
        }
    }
}
template<typename Value>
void MultilevelHatWithCachedSmallest<Value>::decreaseDepthIfNeeded()
{
    if(auto *children=std::get_if<std::vector<Node>>(&root_.content_))
    {
        if(children->size()==1)
        {
            auto newRoot=Node(std::move(children->front()));
            root_=std::move(newRoot);
        }
    }
}
template<typename Value>
size_t MultilevelHatWithCachedSmallest<Value>::findIndexForValue(const Leaf &leaf,const Value &value)
{
    size_t current=leaf.size();
    size_t step=leaf.size();
    while(step>0)
    {
        if(current<step || leaf[current-step]<value)
            step/=2;
        else
            current-=step;
    }
    return current;
}
template<typename Value>
size_t MultilevelHatWithCachedSmallest<Value>::findChildIndexForValue(const std::vector<Node> &nodes,const Value &value)
{
    size_t current=0;
    size_t step=nodes.size();
    while(step>0)
    {
        if(current+step>=nodes.size() || value<nodes[current+step].smallest_)
            step/=2;
        else
            current+=step;
    }
    return current;
}
template<typename Value>
size_t MultilevelHatWithCachedSmallest<Value>::getNodeSize(const Node &node)
{
    if(auto *leaf=std::get_if<Leaf>(&node.content_))
        return leaf->size();
    else if(auto *children=std::get_if<std::vector<Node>>(&node.content_))
        return children->size();
    else
        throw std::logic_error("hmmmm... unknown node type");
}
template<typename Value>
void MultilevelHatWithCachedSmallest<Value>::splitChild(std::vector<Node> &nodes,size_t childIndex)
{
    Node newChild1,newChild2;
    if(auto *leaf=std::get_if<Leaf>(&nodes[childIndex].content_))
    {
        const auto middle=leaf->begin()+leaf->size()/2;
        Leaf firstHalf,secondHalf;
        std::move(leaf->begin(),middle,std::back_inserter(firstHalf));
        std::move(middle,leaf->end(),std::back_inserter(secondHalf));
        newChild1.smallest_=firstHalf.front();
        newChild1.content_=std::move(firstHalf);
        newChild2.smallest_=secondHalf.front();
        newChild2.content_=std::move(secondHalf);
    }
    else if(auto *children=std::get_if<std::vector<Node>>(&nodes[childIndex].content_))
    {
        const auto middle=children->begin()+children->size()/2;
        std::vector<Node> firstHalf,secondHalf;
        std::move(children->begin(),middle,std::back_inserter(firstHalf));
        std::move(middle,children->end(),std::back_inserter(secondHalf));
        newChild1.smallest_=firstHalf.front().smallest_;
        newChild1.content_=std::move(firstHalf);
        newChild2.smallest_=secondHalf.front().smallest_;
        newChild2.content_=std::move(secondHalf);
    }
    nodes[childIndex]=std::move(newChild1);
    nodes.insert(nodes.begin()+childIndex+1,std::move(newChild2));
}
template<typename Value>
void MultilevelHatWithCachedSmallest<Value>::mergeChild(std::vector<Node> &nodes,size_t childIndex)
{
    if(childIndex>0)
    {//consider merging to the left node
        if(childIndex+1>=nodes.size())
            --childIndex;
        else if(getNodeSize(nodes[childIndex-1])<getNodeSize(nodes[childIndex+1]))
            --childIndex;
    }
    if(auto *sourceLeaf=std::get_if<Leaf>(&nodes[childIndex+1].content_))
    {
        auto *targetLeaf=std::get_if<Leaf>(&nodes[childIndex].content_);
        if(!targetLeaf)
            throw std::logic_error("AAAAAAAA!!!! PANIC!!!!!");
        std::move(sourceLeaf->begin(),sourceLeaf->end(),std::back_inserter(*targetLeaf));
    }
    if(auto *sourceChildren=std::get_if<std::vector<Node>>(&nodes[childIndex+1].content_))
    {
        auto *targetChildren=std::get_if<std::vector<Node>>(&nodes[childIndex].content_);
        if(!targetChildren)
            throw std::logic_error("AAAAAAAA!!!! PANIC!!!!!");
        std::move(sourceChildren->begin(),sourceChildren->end(),std::back_inserter(*targetChildren));
    }
    nodes.erase(nodes.begin()+childIndex+1);
}
template<typename Value>
bool MultilevelHatWithCachedSmallest<Value>::contains(const Node &node,const Value &value)
{
    if(auto *leaf=std::get_if<Leaf>(&node.content_))
    {
        const auto index=findIndexForValue(*leaf,value);
        return (index<leaf->size() && (*leaf)[index]==value);
    }
    else if(auto *children=std::get_if<std::vector<Node>>(&node.content_))
    {
        const auto index=findChildIndexForValue(*children,value);
        return contains((*children)[index],value);
    }
    else
        throw std::logic_error("hmmm... unknown node type");
}
template<typename Value>
void MultilevelHatWithCachedSmallest<Value>::enumerate(const Node &node,const std::function<void(const Value&)> &processor)
{
    if(auto *leaf=std::get_if<Leaf>(&node.content_))
    {
        for(const auto &value:*leaf)
            processor(value);
    }
    else if(auto *children=std::get_if<std::vector<Node>>(&node.content_))
    {
        for(const auto &node:*children)
            enumerate(node,processor);
    }
}
