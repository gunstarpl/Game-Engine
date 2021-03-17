/*
    Copyright (c) 2018-2021 Piotr Doan. All rights reserved.
    Software distributed under the permissive MIT License.
*/

#pragma once

#include "Reflection/ReflectionDetail.hpp"

/*
    Reflection Static

    Compile-time reflection interface.
*/

namespace Reflection
{
    struct NullType final
    {
    };

    struct TypeAttribute
    {
    };

    struct FieldAttribute
    {
    };

    struct MethodAttribute
    {
    };

    template<typename ReflectedType, typename AttributeType, std::size_t AttributeIndex>
    struct AttributeDescription
    {
        using Type = std::decay_t<AttributeType>;
        static constexpr auto TypeInfo = Detail::TypeInfo<AttributeType>{};
        static constexpr auto Index = AttributeIndex;
        static constexpr auto Name = TypeInfo.Name;

        template<typename OtherType>
        static constexpr bool IsType()
        {
            return std::is_same<Type, OtherType>::value;
        }
    };

    template<typename ReflectedType, typename AttributeType, std::size_t AttributeIndex>
    struct AttributeDescriptionWithInstance :
        public AttributeDescription<ReflectedType, AttributeType, AttributeIndex>
    {
        using TypeWithoutInstance = AttributeDescription<
            ReflectedType, AttributeType, AttributeIndex>;

        constexpr AttributeDescriptionWithInstance(AttributeType&& Instance) :
            Instance(Instance)
        {
        }

        const AttributeType Instance;
    };

    template<typename ReflectedType, typename MemberType, std::size_t MemberIndex>
    struct MemberDescription
    {
        using Type = MemberType;
        static constexpr auto TypeInfo =
            Detail::TypeInfo<ReflectedType>::Members.template Get<MemberIndex>();
        static constexpr auto Index = MemberIndex;
        static constexpr auto Name = TypeInfo.Name;
        static constexpr auto Pointer = TypeInfo.Pointer;
        static constexpr auto Attributes =
            Detail::MakeAttributeDescriptionWithInstanceList<ReflectedType>(
                TypeInfo.Attributes, std::make_index_sequence<TypeInfo.Attributes.Count>());
        static constexpr auto AttributeTypes =
            Detail::MakeAttributeDescriptionWithoutInstanceList<ReflectedType>(
                TypeInfo.Attributes, std::make_index_sequence<TypeInfo.Attributes.Count>());

        template<typename OtherType>
        static constexpr bool IsType()
        {
            return std::is_same<Type, OtherType>::value;
        }

        static constexpr bool HasAttributes()
        {
            return Attributes.Count > 0;
        }

        template<std::size_t AttributeIndex>
        static constexpr auto Attribute() -> decltype(Attributes.template Get<AttributeIndex>())
        {
            return Attributes.template Get<AttributeIndex>();
        }

        template<auto& AttributeName>
        static constexpr auto FindAttribute()
        {
            constexpr auto Index = FindFirstIndex(AttributeTypes,
                [](auto Attribute) constexpr -> bool
            {
                using AttributeType = decltype(Attribute);
                return AttributeType::Name == AttributeName;
            });

            return Attributes.template Get<Index>();
        }
    };

    template<typename ReflectedType>
    struct StaticTypeInfo
    {
        using Type = ReflectedType;
        static constexpr auto TypeInfo = Detail::TypeInfo<Type>{};

        using BaseType = typename decltype(TypeInfo)::BaseType;
        static constexpr auto BaseTypeInfo = Detail::TypeInfo<BaseType>{};

        static constexpr bool Reflected = TypeInfo.Reflected;
        static constexpr std::string_view Name = TypeInfo.Name;
        static constexpr IdentifierType Identifier = Common::StringHash(TypeInfo.Name);
        static constexpr IdentifierType BaseTypeIdentifier = Common::StringHash(BaseTypeInfo.Name);
        static constexpr auto Attributes =
            Detail::MakeAttributeDescriptionWithInstanceList<Type>(
                TypeInfo.Attributes, std::make_index_sequence<TypeInfo.Attributes.Count>());
        static constexpr auto AttributeTypes =
            Detail::MakeAttributeDescriptionWithoutInstanceList<Type>(
                TypeInfo.Attributes, std::make_index_sequence<TypeInfo.Attributes.Count>());
        static constexpr auto Members =
            Detail::MakeMemberDescriptionList<Type>(
                std::make_index_sequence<TypeInfo.Members.Count>());

        static constexpr bool IsNullType()
        {
            return std::is_same<Type, NullType>::value;
        }

        template<typename OtherType>
        static constexpr bool IsType()
        {
            return std::is_same<Type, OtherType>::value;
        }

        static constexpr bool HasBaseType()
        {
            return !std::is_same<BaseType, NullType>::value;
        }

        static constexpr StaticTypeInfo<BaseType> GetBaseType()
        {
            return {};
        }

        template<typename OtherType>
        static constexpr bool IsDerivedFrom()
        {
            return std::is_same<BaseType, OtherType>::value;
        }

        template<typename OtherType>
        static constexpr bool IsBaseOf()
        {
            return std::is_same<Type, typename StaticTypeInfo<OtherType>::BaseType>::value;
        }

        static constexpr bool HasAttributes()
        {
            return Attributes.Count > 0;
        }

        template<std::size_t AttributeIndex>
        static constexpr auto Attribute()
        {
            static_assert(Attributes.Count > AttributeIndex, "Out of bounds attribute index!");
            return Attributes.template Get<AttributeIndex>();
        }

        template<auto& AttributeName>
        static constexpr auto FindAttribute()
        {
            constexpr auto Index = FindFirstIndex(AttributeTypes,
                [](auto Attribute) constexpr -> bool
            {
                using AttributeType = decltype(Attribute);
                return AttributeType::Name == AttributeName;
            });

            return Attributes.template Get<Index>();
        }

        static constexpr bool HasMembers()
        {
            return Members.Count > 0;
        }

        template<std::size_t MemberIndex>
        static constexpr auto Member()
        {
            static_assert(Members.Count > MemberIndex, "Out of bounds member index!");
            return Members.template Get<MemberIndex>();
        }

        template<auto& MemberName>
        static constexpr auto FindMember()
        {
            return FindOne(Members, [](auto Member) constexpr -> bool
            {
                using MemberType = decltype(Member);
                return MemberType::Name == MemberName;
            });
        }
    };

    template<typename ReflectedType>
    constexpr StaticTypeInfo<ReflectedType> StaticType()
    {
        return {};
    }

    template<typename ReflectedType>
    constexpr StaticTypeInfo<ReflectedType> StaticType(const ReflectedType& instance)
    {
        return {};
    }

    template<typename ReflectedType>
    constexpr bool IsReflected()
    {
        return StaticTypeInfo<ReflectedType>::Reflected;
    }

    template<typename ReflectedType>
    constexpr bool IsReflected(const ReflectedType& type)
    {
        return StaticTypeInfo<ReflectedType>::Reflected;
    }
}

#define REFLECTION_IDENTIFIER(Type) \
    Reflection::StaticType<Type>().Identifier
