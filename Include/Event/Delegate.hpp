/*
    Copyright (c) 2018-2020 Piotr Doan. All rights reserved.
*/

#pragma once

#include <Common/Debug.hpp>

/*
    Delegate

    Binds function, class method or functor/lambda which can be invoked
    at later time. Be careful not to invoke a delegate to method of an 
    instance that no longer exists. Check Receiver and Dispatcher class
    templates for subscription based solution that wraps delegates.
    
    Implementation partially based on:
    - http://molecularmusings.wordpress.com/2011/09/19/generic-type-safe-delegates-and-events-in-c/
*/

namespace Event
{
    template<typename Type>
    class Delegate;

    template<typename ReturnType, typename... Arguments>
    class Delegate<ReturnType(Arguments...)>
    {
    private:
        using InstancePtr = void*;
        using FunctionPtr = ReturnType(*)(InstancePtr, Arguments&&...);

        template<ReturnType(*Function)(Arguments...)>
        static ReturnType FunctionStub(InstancePtr instance, Arguments&&... arguments)
        {
            return (Function)(std::forward<Arguments>(arguments)...);
        }

        template<class InstanceType, ReturnType(InstanceType::*Function)(Arguments...)>
        static ReturnType MethodStub(InstancePtr instance, Arguments&&... arguments)
        {
            return (static_cast<InstanceType*>(instance)->*Function)(std::forward<Arguments>(arguments)...);
        }

        template<class InstanceType>
        static ReturnType FunctorStub(InstancePtr instance, Arguments&&... arguments)
        {
            return (*static_cast<InstanceType*>(instance))(std::forward<Arguments>(arguments)...);
        }

    public:
        Delegate() :
            m_instance(nullptr),
            m_function(nullptr)
        {
        }

        Delegate(std::nullptr_t) :
            Delegate()
        {
        }

        virtual ~Delegate()
        {
        }

        Delegate(Delegate&& other) :
            Delegate()
        {
            *this = std::move(other);
        }

        Delegate(const Delegate& other)
        {
            *this = other;
        }

        Delegate& operator=(Delegate&& other)
        {
            // Performing move of a delegate is very dangerous
            // as they hold references to instances they are invoking.
            // Most often it is preferred to omit moving a delegate.

            std::swap(m_instance, other.m_instance);
            std::swap(m_function, other.m_function);
            return *this;
        }

        Delegate& operator=(const Delegate& other)
        {
            m_instance = other.m_instance;
            m_function = other.m_function;
            return *this;
        }

        Delegate& operator=(std::nullptr_t)
        {
            Bind(nullptr);
            return *this;
        }

        void Bind(std::nullptr_t)
        {
            m_instance = nullptr;
            m_function = nullptr;
        }

        template<ReturnType(*Function)(Arguments...)>
        void Bind()
        {
            m_instance = nullptr;
            m_function = &FunctionStub<Function>;
        }

        template<class InstanceType>
        void Bind(InstanceType* instance)
        {
            ASSERT(instance != nullptr, "Received nullptr as functor instance!");

            m_instance = instance;
            m_function = &FunctorStub<InstanceType>;
        }

        template<class InstanceType, ReturnType(InstanceType::*Function)(Arguments...)>
        void Bind(InstanceType* instance)
        {
            ASSERT(instance != nullptr, "Received nullptr as method instance!");

            m_instance = instance;
            m_function = &MethodStub<InstanceType, Function>;
        }

        template<typename Lambda>
        Delegate(const Lambda& lambda)
        {
            Bind(lambda);
        }

        template<typename Lambda>
        Delegate& operator=(const Lambda& lambda)
        {
            Bind(lambda);
            return *this;
        }

        template<typename Lambda>
        void Bind(const Lambda& lambda)
        {
            // Every lambda has different type. We can abuse this to create static instance
            // for every permutation of this methods called with different lambda type.

            static Lambda staticLambda = lambda;
            m_instance = static_cast<void*>(&staticLambda);
            m_function = &FunctorStub<Lambda>;
        }

        ReturnType Invoke(Arguments... arguments)
        {
            VERIFY(m_function != nullptr, "Attempting to invoke a delegate without a bound function!");
            return m_function(m_instance, std::forward<Arguments>(arguments)...);
        }

        ReturnType operator()(Arguments... arguments)
        {
            return Invoke(std::forward<Arguments>(arguments)...);
        }

        bool IsBound()
        {
            return m_function != nullptr;
        }

    private:
        InstancePtr m_instance;
        FunctionPtr m_function;
    };
}
