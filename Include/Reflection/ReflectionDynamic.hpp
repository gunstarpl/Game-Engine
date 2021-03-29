/*
    Copyright (c) 2018-2021 Piotr Doan. All rights reserved.
    Software distributed under the permissive MIT License.
*/

#pragma once

#include "Reflection/ReflectionDetail.hpp"
#include "Reflection/ReflectionStatic.hpp"

/*
    Reflection Dynamic

    Run-time reflection interface.
*/

namespace Reflection
{
    template<typename ReflectedType>
    constexpr DecayedStaticTypeInfo<ReflectedType> StaticType();

    template<typename ReflectedType>
    TypeIdentifier GetIdentifier();

    class DynamicTypeInfo final
    {
    public:
        friend class Registry;
        static const DynamicTypeInfo Invalid;

        using DynamicTypeList = std::vector<std::reference_wrapper<const DynamicTypeInfo>>;
        using InstantiateFunction = void* (*)();
        
    public:
        DynamicTypeInfo() = default;
        ~DynamicTypeInfo() = default;

        void* Instantiate() const;

        bool IsRegistered() const
        {
            return m_registered;
        }

        bool IsNullType() const 
        {
            return m_registered && m_identifier == Reflection::GetIdentifier<NullType>();
        }

        bool HasBaseType() const
        {
            return m_registered && !m_baseType->IsNullType();
        }

        const std::string_view& GetName() const
        {
            return m_name;
        }

        TypeIdentifier GetIdentifier() const
        {
            return m_identifier;
        }

        const DynamicTypeInfo& GetBaseType() const
        {
            return *m_baseType;
        }

        const DynamicTypeList& GetDerivedTypes() const
        {
            return m_derivedTypes;
        }

        bool IsType(TypeIdentifier identifier) const;
        bool IsBaseOf(TypeIdentifier identifier) const;
        bool IsDerivedFrom(TypeIdentifier identifier) const;

        template<typename OtherType>
        bool IsType() const
        {
            return IsType(StaticType<OtherType>().Identifier);
        }

        template<typename OtherType>
        bool IsType(const OtherType& instance) const
        {
            return IsType(instance.GetTypeInfo().GetIdentifier());
        }

        template<typename OtherType>
        bool IsBaseOf() const
        {
            return IsBaseOf(StaticType<OtherType>().Identifier);
        }

        template<typename OtherType>
        bool IsBaseOf(const OtherType& instance) const
        {
            return IsBaseOf(instance.GetTypeInfo().GetIdentifier());
        }

        template<typename OtherType>
        bool IsDerivedFrom() const
        {
            return IsDerivedFrom(StaticType<OtherType>().Identifier);
        }

        template<typename OtherType>
        bool IsDerivedFrom(const OtherType& instance) const
        {
            return IsDerivedFrom(instance.GetTypeInfo().GetIdentifier());
        }

    private:
        void Register(std::string_view name, TypeIdentifier identifier,
            InstantiateFunction createFunction, DynamicTypeInfo* baseType);
        void AddDerivedType(const DynamicTypeInfo& typeInfo);

        bool m_registered = false;
        std::string_view m_name = "<UnregisteredType>";
        TypeIdentifier m_identifier = InvalidIdentifier;
        InstantiateFunction m_instantiateFunction = nullptr;
        const DynamicTypeInfo* m_baseType = &Invalid;
        DynamicTypeList m_derivedTypes;
    };

    class DynamicTypeStorage
    {
    public:
        const DynamicTypeInfo& GetTypeInfo() const
        {
            return DynamicType;
        }

    private:
        friend class Registry;
        DynamicTypeInfo DynamicType;
    };
}