/*
    Copyright (c) 2018-2019 Piotr Doan. All rights reserved.
*/

#pragma once

/*
    Event Queue
*/

namespace Event
{
    // Event queue class.
    class Queue
    {
    public:
        // Type declarations.
        using EventList = std::queue<std::any>;

    public:
        Queue() = default;
        ~Queue() = default;

        Queue(const Queue& other) = delete;
        Queue& operator=(const Queue& other) = delete;

        Queue(Queue&& other)
        {
            *this = std::move(other);
        }

        Queue& operator=(Queue&& other)
        {
            std::swap(m_eventList, other.m_eventList);
            return *this;
        }

        template<typename Type>
        void Push(const Type& event)
        {
            // Add event to the list (will be stored in std::any).
            m_eventList.push(event);
        }

        std::any Pop()
        {
            // Return an invalid object if list is empty.
            if(m_eventList.empty())
                return std::any();

            // Pop an event from the list and return it.
            std::any event = m_eventList.front();
            m_eventList.pop();
            return event;
        }

        bool IsEmpty() const
        {
            return m_eventList.empty();
        }

    private:
        // List of events.
        EventList m_eventList;
    };
}
