/******************************************************************************
**
** Copyright (C) 2020 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Quick Ultralite module.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
******************************************************************************/
#pragma once

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qul API. It exists purely as an
// implementation detail for purpose of auto-generated code.
// This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <qul/private/flagpointer.h>

namespace Qul {
namespace Private {

struct DirtyListNode
{
    DirtyListNode()
        : next()
    {}

    FlagPointer<DirtyListNode> next;
};

class DirtyListBase
{
public:
    DirtyListBase()
        : head(NULL)
        , tail(NULL)
        , pending(NULL)
    {}

    /*
     *  Marks node dirty and appends it to the list with O(1) complexity.
     */
    void dirty(DirtyListNode *node);

    /*
     * Removes node from the list with O(n) complexity.
     */
    void remove(DirtyListNode *node);

    inline bool isEmpty() const { return !head; }

protected:
    DirtyListNode *head;
    DirtyListNode *tail;
    DirtyListNode *pending;
};

/* A dirty list is an intrusive singly-linked list, optimized for dirty handling.
 * Relies on Qul::Private::offsetOf() optimization to reduce the required memory.
 *
 * Elements must have a DirtyListNode member
 *
 *     struct Element {
 *         DirtyListNode node;
 *     };
 *
 * and can then be stored in a dirty list
 *
 *     DirtyList<Element, &Element::node> list;
 *
 * DirtyLists can be processed, which empties the list.
 * Reinsertions during processing are supported.
 *
 *     list.process(myfunction);
 */
template<typename T, DirtyListNode T::*ListNode>
class DirtyList : public DirtyListBase
{
public:
    template<typename Functor>
    inline void process(Functor f)
    {
        int dirtyListRecursionCounter = 0;
        while (head) {
            DirtyListNode *node = head;
            head = NULL;
            tail = NULL;

            while (node) {
                ptrdiff_t offset = offsetOf(ListNode);
                void *t = reinterpret_cast<char *>(node) - offset;
                pending = node->next.data();
                node->next.clearFlag();
                f(reinterpret_cast<T *>(t));
                node = pending;
            }

            pending = NULL;

            if (++dirtyListRecursionCounter > 1000)
                assert(!"Binding loop detected");
        }
    }
};

struct SinglyLinkedListNode
{
    SinglyLinkedListNode *next = nullptr;
};

class SinglyLinkedListBase
{
public:
    /*
     * Prepends node to the list with O(1) complexity.
     */
    void prepend(SinglyLinkedListNode *node);

    /*
     * Removes node from the list with O(n) complexity.
     */
    void remove(SinglyLinkedListNode *node);

    inline bool isEmpty() const { return !head; }

protected:
    SinglyLinkedListNode *head = nullptr;
};

/* General-purpose singly linked list. Relies on Qul::Private::offsetOf()
 * optimization to reduce the required memory.
 *
 * Elements must have a SinglyLinkedListNode member
 *
 *     struct Element {
 *         SinglyLinkedListNode node;
 *     };
 *
 * and can then be stored in a list
 *
 *     SinglyLinkedList<Element, &Element::node> list;
 *
 * SinglyLinkedList can be iterated
 *
 *     list.iterate([](Element *) {
 *         // process the element
 *     });
 */
template<typename T, SinglyLinkedListNode T::*ListNode>
class SinglyLinkedList : public SinglyLinkedListBase
{
public:
    template<typename Functor>
    inline void iterate(Functor f)
    {
        for (auto it = head; it; it = it->next) {
            ptrdiff_t offset = offsetOf(ListNode);
            void *t = reinterpret_cast<char *>(it) - offset;
            f(reinterpret_cast<T *>(t));
        }
    }
};

} // namespace Private
} // namespace Qul
