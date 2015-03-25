#ifndef INCLUDE_STATICLIST_H
#define INCLUDE_STATICLIST_H

template<typename T> class StaticListNode;

template<typename T>
class StaticList
{
public:
    StaticList() : m_head(NULL) { }

    StaticListNode<T>* setHead(StaticListNode<T>* node) {
        StaticListNode<T>* oldHead = m_head;
        m_head = node;
        return oldHead;
    }

    StaticListNode<T>* head() { return m_head; }

private:
    StaticListNode<T>* m_head;
};

template<typename T>
class StaticListNode
{
public:
    StaticListNode(StaticList<T>& list, T item)
    : m_item(item), m_next(list.setHead(this)) { }

    T& item() { return m_item; }
    StaticListNode<T>* next() { return m_next; }

private:
    T m_item;
    StaticListNode<T>* m_next;
};

#endif // INCLUDE_STATICLIST_H
