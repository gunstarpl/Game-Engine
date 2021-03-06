/*
    Copyright (c) 2018-2021 Piotr Doan. All rights reserved.
    Software distributed under the permissive MIT License.
*/

#pragma once

#include <queue>
#include <vector>
#include <unordered_map>
#include "Game/EntityHandle.hpp"
#include "Game/Component.hpp"

/*
    Component Pool

    Manages a pool for a single type of a component.
    See ComponentSystem for more context.
*/

namespace Game
{
    class ComponentSystem;

    class ComponentPoolInterface
    {
    protected:
        ComponentPoolInterface()
        {
        }

    public:
        virtual ~ComponentPoolInterface()
        {
        }

        virtual bool InitializeComponent(EntityHandle handle) = 0;
        virtual bool DestroyComponent(EntityHandle handle) = 0;
    };

    template<typename ComponentType>
    class ComponentPool final : public ComponentPoolInterface, private Common::NonCopyable
    {
    public:
        static_assert(std::is_base_of<Component, ComponentType>::value, "Not a component type.");

        struct ComponentFlags
        {
            enum
            {
                Unused = 0, // Components is unused and wait in the free list.
                Exists = 1 << 0, // Component exists and can be accessed.
                Initialized = 1 << 1, // Component has been initialized.
            };

            using Type = unsigned int;
        };

        struct ComponentEntry
        {
            typename ComponentFlags::Type flags = ComponentFlags::Unused;
            EntityHandle entity;
            ComponentType component;
        };

        using ComponentIndex = std::size_t;
        using ComponentList = std::vector<ComponentEntry>;
        using ComponentFreeList = std::queue<ComponentIndex>;
        using ComponentLookup = std::unordered_map<EntityHandle, ComponentIndex>;

        class ComponentIterator
        {
        public:
            using BaseIterator = typename ComponentList::iterator;

        public:
            ComponentIterator(const BaseIterator& iterator, const BaseIterator& end);

            ComponentType& operator*();
            bool operator==(const ComponentIterator& other) const;
            bool operator!=(const ComponentIterator& other) const;
            ComponentIterator& operator++();

        private:
            // Iterate to next valid component.
            void ValidateIterator();

        private:
            // Iterator that we are wrapping around.
            BaseIterator m_iterator;

            // End of container that we are iterating over.
            BaseIterator m_end;
        };

    public:
        ComponentPool(ComponentSystem* componentSystem);
        ~ComponentPool();

        // Returns nullptr if component could not be created.
        ComponentType* CreateComponent(EntityHandle entity);

        // Returns nullptr if component could not be found.
        ComponentType* LookupComponent(EntityHandle entity);

        // Returns true if component was successfully initialized.
        bool InitializeComponent(EntityHandle entity) override;

        // Returns true if component was found and destroyed.
        bool DestroyComponent(EntityHandle entity) override;

        ComponentIterator Begin();
        ComponentIterator End();

    private:
        ComponentSystem* m_componentSystem;
        ComponentList m_entries;
        ComponentLookup m_lookup;
        ComponentFreeList m_freeList;
    };

    template<typename ComponentType>
    void ComponentPool<ComponentType>::ComponentIterator::ValidateIterator()
    {
        // Make sure that the current iterator is valid 
        // and if not, find the next iterator that is.
        while(m_iterator != m_end)
        {
            // Check if current iterator points at a valid component.
            if(m_iterator->flags & ComponentFlags::Initialized)
            {
                // Make sure component actually exists.
                ASSERT(m_iterator->flags & ComponentFlags::Exists,
                    "Component is not marked as existing despite being marked as initialized!");

                // Iterator is valid.
                break;
            }

            // Move iterator forward to the next element.
            ++m_iterator;
        }
    }

    template<typename ComponentType>
    ComponentPool<ComponentType>::ComponentIterator::ComponentIterator(const BaseIterator& iterator, const BaseIterator& end) :
        m_iterator(iterator), m_end(end)
    {
        // Make sure iterator is valid.
        this->ValidateIterator();
    }

    template<typename ComponentType>
    ComponentType& ComponentPool<ComponentType>::ComponentIterator::operator*()
    {
        return m_iterator->component;
    }

    template<typename ComponentType>
    bool ComponentPool<ComponentType>::ComponentIterator::operator==(const ComponentIterator& other) const
    {
        return m_iterator == other.m_iterator;
    }

    template<typename ComponentType>
    bool ComponentPool<ComponentType>::ComponentIterator::operator!=(const ComponentIterator& other) const
    {
        return m_iterator != other.m_iterator;
    }

    template<typename ComponentType>
    typename ComponentPool<ComponentType>::ComponentIterator& ComponentPool<ComponentType>::ComponentIterator::operator++()
    {
        ASSERT(m_iterator != m_end, "Trying to increment component iterator past end!");

        // Increment the iterator.
        ++m_iterator;

        // Make sure iterator is valid.
        this->ValidateIterator();

        return *this;
    }

    template<typename ComponentType>
    ComponentPool<ComponentType>::ComponentPool(ComponentSystem* componentSystem) :
        m_componentSystem(componentSystem)
    {
        ASSERT(m_componentSystem != nullptr, "Component system cannot be null!");
    }

    template<typename ComponentType>
    ComponentPool<ComponentType>::~ComponentPool() = default;

    template<typename ComponentType>
    ComponentType* ComponentPool<ComponentType>::CreateComponent(EntityHandle entity)
    {
        // Make sure that there is no component with this entity handle.
        if(m_lookup.find(entity) != m_lookup.end())
            return nullptr;

        // Create a new component entry if the free list is empty.
        if(m_freeList.empty())
        {
            // Create a new component entry.
            m_entries.emplace_back();

            // Add a new entry to the free list queue.
            m_freeList.emplace(m_entries.size() - 1);
        }

        // Retrieve an unused component index.
        ComponentIndex componentIndex = m_freeList.front();
        m_freeList.pop();

        // Add newly created component to the lookup dictionary.
        auto result = m_lookup.emplace(entity, componentIndex);
        ASSERT(result.second, "Failed to add a component to the dictionary!");

        // Retrieve a component entry.
        ComponentEntry& componentEntry = m_entries[componentIndex];

        // Mark component as existing.
        ASSERT(componentEntry.flags == ComponentFlags::Unused);
        componentEntry.flags = ComponentFlags::Exists;

        // Return newly created component
        return &componentEntry.component;
    }

    template<typename ComponentType>
    ComponentType* ComponentPool<ComponentType>::LookupComponent(EntityHandle handle)
    {
        // Find the component index.
        auto it = m_lookup.find(handle);
        if(it == m_lookup.end())
            return nullptr;

        ComponentIndex componentIndex = it->second;

        // Retrieve the component entry.
        ComponentEntry& componentEntry = m_entries[componentIndex];

        // Validate component entry state.
        ASSERT(componentEntry.flags & ComponentFlags::Exists);

        // Return a pointer to the component.
        return &componentEntry.component;
    }

    template<typename ComponentType>
    bool ComponentPool<ComponentType>::InitializeComponent(EntityHandle entity)
    {
        // Find the component index.
        auto it = m_lookup.find(entity);

        // Return true if there is no such component to initialize.
        if(it == m_lookup.end())
            return true;

        // Retrieve the component entry.
        ComponentIndex componentIndex = it->second;
        ComponentEntry& componentEntry = m_entries[componentIndex];

        // Make sure that component's state is valid.
        ASSERT(componentEntry.flags & ComponentFlags::Exists);
        ASSERT(!(componentEntry.flags & ComponentFlags::Initialized));

        // Get base component interface.
        Component& componentInterface = componentEntry.component;

        // Initialize component and return result.
        ASSERT(m_componentSystem != nullptr, "Component system cannot be null!");
        if(!componentInterface.OnInitialize(m_componentSystem, entity))
            return false;

        // Mark component as initialized.
        componentEntry.flags |= ComponentFlags::Initialized;

        return true;
    }

    template<typename ComponentType>
    bool ComponentPool<ComponentType>::DestroyComponent(EntityHandle entity)
    {
        // Find the component index.
        auto it = m_lookup.find(entity);
        if(it == m_lookup.end())
            return false;

        ComponentIndex componentIndex = it->second;

        // Retrieve the component entry.
        ComponentEntry& componentEntry = m_entries[componentIndex];

        // Mark component as unused.
        ASSERT(componentEntry.flags & ComponentFlags::Exists);
        componentEntry.flags = ComponentFlags::Unused;

        // Recreate component instance to trigger a destructor and create a new element.
        ComponentType* component = &componentEntry.component;

        component->~ComponentType();
        new (component) ComponentType();

        // Add an unused component index to the free list.
        m_freeList.emplace(componentIndex);

        // Remove component entry from the dictionary.
        m_lookup.erase(it);

        return true;
    }

    template<typename ComponentType>
    typename ComponentPool<ComponentType>::ComponentIterator ComponentPool<ComponentType>::Begin()
    {
        return ComponentIterator(m_entries.begin(), m_entries.end());
    }

    template<typename ComponentType>
    typename ComponentPool<ComponentType>::ComponentIterator ComponentPool<ComponentType>::End()
    {
        return ComponentIterator(m_entries.end(), m_entries.end());
    }

    template<typename ComponentType>
    typename ComponentPool<ComponentType>::ComponentIterator begin(ComponentPool<ComponentType>& pool)
    {
        return pool.Begin();
    }

    template<typename ComponentType>
    typename ComponentPool<ComponentType>::ComponentIterator end(ComponentPool<ComponentType>& pool)
    {
        return pool.End();
    }
}
