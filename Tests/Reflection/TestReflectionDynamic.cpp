/*
    Copyright (c) 2018-2021 Piotr Doan. All rights reserved.
    Software distributed under the permissive MIT License.
*/

#include <string>
#include <vector>
#include <cinttypes>
#include <doctest/doctest.h>
#include <Reflection/Reflection.hpp>
#include "TestReflectionHeader.hpp"
#include "TestReflection/ReflectionGenerated.hpp"

TEST_CASE("Dynamic Reflection")
{
    SUBCASE("Check registered built-in types")
    {
        Reflection::TypeIdentifier nullTypeIdentifier =
            Reflection::GetIdentifier<Reflection::NullType>();

        CHECK(Reflection::IsRegistered(nullTypeIdentifier));
        CHECK(Reflection::DynamicType(nullTypeIdentifier).IsRegistered());
        CHECK(Reflection::DynamicType(nullTypeIdentifier).IsNullType());
        CHECK_EQ(Reflection::DynamicType(nullTypeIdentifier).GetName(),
            Common::Name("Reflection::NullType"));
        CHECK_FALSE(Reflection::DynamicType(Reflection::GetIdentifier<Undefined>()).IsNullType());
        CHECK_FALSE(Reflection::DynamicType(Reflection::GetIdentifier<Derived>()).IsNullType());
    }

    SUBCASE("Check registered types")
    {
        CHECK_FALSE(Reflection::IsRegistered(Reflection::GetIdentifier<Undefined>()));
        CHECK_FALSE(Reflection::IsRegistered(Reflection::GetIdentifier<CrossUnit>()));
        CHECK_FALSE(Reflection::DynamicType(
            Reflection::GetIdentifier<Undefined>()).IsRegistered());
        CHECK_FALSE(Reflection::DynamicType(
            Reflection::GetIdentifier<CrossUnit>()).IsRegistered());
        CHECK(Reflection::DynamicType<Empty>().IsRegistered());
        CHECK(Reflection::DynamicType<Base>().IsRegistered());
        CHECK(Reflection::DynamicType(Reflection::GetIdentifier<Derived>()).IsRegistered());
        CHECK(Reflection::DynamicType(Reflection::GetIdentifier<Inner>()).IsRegistered());
        CHECK(Reflection::DynamicType(Reflection::GetIdentifier<BranchedOne>()).IsRegistered());
        CHECK(Reflection::DynamicType(Reflection::GetIdentifier<BranchedTwo>()).IsRegistered());
    }

    SUBCASE("Check registered type names")
    {
        CHECK_EQ(Reflection::DynamicType(Reflection::GetIdentifier<Empty>())
            .GetName(), Common::Name("Empty"));
        CHECK_EQ(Reflection::DynamicType(Reflection::GetIdentifier<Base>())
            .GetName(), Common::Name("Base"));

        CHECK_EQ(Reflection::GetName(Derived()), NAME_CONSTEXPR("Derived"));

        Inner inner;
        CHECK_EQ(Reflection::GetName(&inner), NAME_CONSTEXPR("Inner"));

        std::unique_ptr<Derived> branchedOne = std::make_unique<BranchedOne>();
        CHECK_EQ(Reflection::GetName(branchedOne), NAME_CONSTEXPR("BranchedOne"));

        Reflection::TypeIdentifier branchedTwo = Reflection::GetIdentifier<BranchedTwo>();
        CHECK_EQ(Reflection::GetName(branchedTwo), NAME_CONSTEXPR("BranchedTwo"));
    }

    SUBCASE("Check registered type identifier")
    {
        CHECK_EQ(Reflection::DynamicType(Reflection::GetIdentifier<Empty>()).GetIdentifier(),
            Reflection::GetIdentifier<Empty>());
        CHECK_EQ(Reflection::DynamicType(Reflection::GetIdentifier<Base>()).GetIdentifier(),
            Reflection::GetIdentifier<Base>());
        CHECK_EQ(Reflection::DynamicType(Reflection::GetIdentifier<Derived>()).GetIdentifier(),
            Reflection::GetIdentifier<Derived>());

        Inner inner;
        CHECK_EQ(Reflection::DynamicType(inner).GetIdentifier(),
            Reflection::GetIdentifier<Inner>());

        BranchedOne branchedOne;
        CHECK_EQ(Reflection::DynamicType(&branchedOne).GetIdentifier(),
            Reflection::GetIdentifier<BranchedOne>());

        std::unique_ptr<BranchedTwo> branchedTwo = std::make_unique<BranchedTwo>();
        CHECK_EQ(Reflection::DynamicType(branchedTwo).GetIdentifier(),
            Reflection::GetIdentifier<BranchedTwo>());
    }

    SUBCASE("Check registered type by value")
    {
        CHECK(Reflection::IsRegistered(Reflection::NullType()));
        CHECK(Reflection::DynamicType(Empty()).IsType<Empty>());
        CHECK(Reflection::DynamicType(Base()).IsType<Base>());
        CHECK(Reflection::DynamicType(Derived()).IsType<Derived>());
        CHECK(Reflection::DynamicType(Inner()).IsType<Inner>());
        CHECK(Reflection::DynamicType(BranchedOne()).IsType<BranchedOne>());
        CHECK(Reflection::DynamicType(BranchedTwo()).IsType<BranchedTwo>());
    }

    SUBCASE("Check registered type for base type")
    {
        CHECK_FALSE(Reflection::DynamicType(Reflection::GetIdentifier<Undefined>()).HasBaseType());
        CHECK_FALSE(Reflection::DynamicType(Reflection::GetIdentifier<Empty>()).HasBaseType());
        CHECK_FALSE(Reflection::DynamicType(Reflection::GetIdentifier<Base>()).HasBaseType());
        CHECK(Reflection::DynamicType(Reflection::GetIdentifier<Derived>()).HasBaseType());
        CHECK_FALSE(Reflection::DynamicType(Reflection::GetIdentifier<Inner>()).HasBaseType());
        CHECK(Reflection::DynamicType(Reflection::GetIdentifier<BranchedOne>()).HasBaseType());
        CHECK(Reflection::DynamicType(Reflection::GetIdentifier<BranchedTwo>()).HasBaseType());
    }

    SUBCASE("Check registered base type")
    {
        CHECK_EQ(Reflection::DynamicType<Derived>().GetBaseType().GetName(),
            Common::Name("Base"));
        CHECK_EQ(Reflection::DynamicType<BranchedOne>().GetBaseType().GetName(),
            Common::Name("Derived"));
        CHECK_EQ(Reflection::DynamicType<BranchedTwo>().GetBaseType().GetName(),
            Common::Name("Derived"));
        CHECK(Reflection::DynamicType<Derived>().GetBaseType().IsType<Base>());
        CHECK(Reflection::DynamicType<BranchedOne>().GetBaseType().IsType<Derived>());
        CHECK(Reflection::DynamicType<BranchedTwo>().GetBaseType().IsType<Derived>());

        CHECK(Reflection::DynamicType<Derived>().IsDerivedFrom<Base>());
        CHECK(Reflection::DynamicType<BranchedOne>().IsDerivedFrom<Derived>());
        CHECK(Reflection::DynamicType<BranchedTwo>().IsDerivedFrom<Derived>());

        CHECK(Reflection::DynamicType<Base>().IsBaseOf<Derived>());
        CHECK(Reflection::DynamicType<Derived>().IsBaseOf<BranchedOne>());
        CHECK(Reflection::DynamicType<Derived>().IsBaseOf<BranchedOne>());
    }

    SUBCASE("Check registered super declaration")
    {
        CHECK_EQ(Reflection::DynamicType<Derived>().GetBaseType().GetIdentifier(),
            Reflection::DynamicType<Derived::Super>().GetIdentifier());

        CHECK(Reflection::DynamicType<BranchedOne>().GetBaseType().IsType<BranchedOne::Super>());
        CHECK(Reflection::DynamicType<BranchedTwo>().GetBaseType().IsType<BranchedTwo::Super>());
    }

    SUBCASE("Check registered polymorphic type")
    {
        CHECK(Reflection::DynamicType<BranchedOne>().IsType<BranchedOne>());
        CHECK(Reflection::DynamicType<BranchedTwo>().IsType<BranchedTwo>());
        CHECK(Reflection::DynamicType<BranchedOne>().IsType<Derived>());
        CHECK(Reflection::DynamicType<BranchedTwo>().IsType<Derived>());
        CHECK(Reflection::DynamicType<BranchedOne>().IsType<Base>());
        CHECK(Reflection::DynamicType<BranchedTwo>().IsType<Base>());

        CHECK_FALSE(Reflection::DynamicType<BranchedOne>().IsType<Reflection::NullType>());
        CHECK_FALSE(Reflection::DynamicType<BranchedOne>().IsType<BranchedTwo>());
        CHECK_FALSE(Reflection::DynamicType<BranchedTwo>().IsType<BranchedOne>());
        CHECK_FALSE(Reflection::DynamicType<BranchedOne>().IsType<Inner>());
        CHECK_FALSE(Reflection::DynamicType<BranchedTwo>().IsType<Inner>());

        CHECK(Reflection::DynamicType<BranchedOne>().IsDerivedFrom<Derived>());
        CHECK(Reflection::DynamicType<BranchedTwo>().IsDerivedFrom<Derived>());
        CHECK(Reflection::DynamicType<BranchedOne>().IsDerivedFrom<Base>());
        CHECK(Reflection::DynamicType<BranchedTwo>().IsDerivedFrom<Base>());

        CHECK_FALSE(Reflection::DynamicType<Base>().IsDerivedFrom<Reflection::NullType>());
        CHECK_FALSE(Reflection::DynamicType<BranchedOne>().IsDerivedFrom<Reflection::NullType>());
        CHECK_FALSE(Reflection::DynamicType<BranchedOne>().IsDerivedFrom<BranchedOne>());
        CHECK_FALSE(Reflection::DynamicType<Derived>().IsDerivedFrom<BranchedOne>());
        CHECK_FALSE(Reflection::DynamicType<Base>().IsDerivedFrom<BranchedOne>());
        CHECK_FALSE(Reflection::DynamicType<Inner>().IsDerivedFrom<BranchedOne>());

        CHECK(Reflection::DynamicType<Derived>().IsBaseOf<BranchedOne>());
        CHECK(Reflection::DynamicType<Derived>().IsBaseOf<BranchedTwo>());
        CHECK(Reflection::DynamicType<Base>().IsBaseOf<BranchedOne>());
        CHECK(Reflection::DynamicType<Base>().IsBaseOf<BranchedTwo>());

        CHECK_FALSE(Reflection::DynamicType<Derived>().IsBaseOf<Reflection::NullType>());
        CHECK_FALSE(Reflection::DynamicType<Derived>().IsBaseOf<Derived>());
        CHECK_FALSE(Reflection::DynamicType<Inner>().IsBaseOf<Derived>());
        CHECK_FALSE(Reflection::DynamicType<Derived>().IsBaseOf<Inner>());
    }

    SUBCASE("Check registered polymoprhic instance")
    {
        BranchedOne branchedOne;
        Derived& branchedOneDerived = branchedOne;
        Base& branchedOneBase = branchedOne;

        CHECK(branchedOne.GetTypeInfo().IsType<BranchedOne>());
        CHECK(branchedOne.GetTypeInfo().IsType<Derived>());
        CHECK(branchedOne.GetTypeInfo().IsType<Base>());
        CHECK(branchedOneDerived.GetTypeInfo().IsType<BranchedOne>());
        CHECK(branchedOneDerived.GetTypeInfo().IsType<Base>());
        CHECK(branchedOneBase.GetTypeInfo().IsType<BranchedOne>());
        CHECK(branchedOneBase.GetTypeInfo().IsType<Derived>());

        CHECK(branchedOne.GetTypeInfo().IsType(branchedOne));
        CHECK(branchedOne.GetTypeInfo().IsType(branchedOneDerived));
        CHECK(branchedOne.GetTypeInfo().IsType(branchedOneBase));
        CHECK(branchedOneDerived.GetTypeInfo().IsType(branchedOne));
        CHECK(branchedOneDerived.GetTypeInfo().IsType(branchedOneBase));
        CHECK(branchedOneBase.GetTypeInfo().IsType(branchedOne));
        CHECK(branchedOneBase.GetTypeInfo().IsType(branchedOneDerived));

        BranchedTwo branchedTwo;
        Derived& branchedTwoDerived = branchedTwo;
        Base& branchedTwoBase = branchedTwo;

        CHECK_FALSE(branchedTwo.GetTypeInfo().IsType(branchedOne));
        CHECK_FALSE(branchedTwo.GetTypeInfo().IsType(branchedOneDerived));
        CHECK_FALSE(branchedTwo.GetTypeInfo().IsType(branchedOneBase));
        CHECK_FALSE(branchedTwoDerived.GetTypeInfo().IsType(branchedOne));
        CHECK_FALSE(branchedTwoDerived.GetTypeInfo().IsType(branchedOneDerived));
        CHECK_FALSE(branchedTwoDerived.GetTypeInfo().IsType(branchedOneBase));
        CHECK_FALSE(branchedTwoBase.GetTypeInfo().IsType(branchedOne));
        CHECK_FALSE(branchedTwoBase.GetTypeInfo().IsType(branchedOneDerived));
        CHECK_FALSE(branchedTwoBase.GetTypeInfo().IsType(branchedOneBase));
    }

    SUBCASE("Check registered type casting")
    {
        BranchedOne branchedOne;
        branchedOne.inner.value = 42;

        BranchedOne* branchedOnePtr = Reflection::Cast<BranchedOne>(&branchedOne);
        REQUIRE_NE(branchedOnePtr, nullptr);
        CHECK_EQ(branchedOnePtr->inner.value, 42);
        CHECK(branchedOnePtr->GetTypeInfo().IsType<BranchedOne>());
        CHECK(branchedOnePtr->GetTypeInfo().IsType<Derived>());
        CHECK(branchedOnePtr->GetTypeInfo().IsType<Base>());

        Derived* derivedPtr = Reflection::Cast<Derived>(branchedOnePtr);
        REQUIRE_NE(derivedPtr, nullptr);
        CHECK(derivedPtr->GetTypeInfo().IsType<BranchedOne>());
        CHECK(derivedPtr->GetTypeInfo().IsType<Derived>());
        CHECK(derivedPtr->GetTypeInfo().IsType<Base>());

        Base* basePtr = Reflection::Cast<Base>(branchedOnePtr);
        REQUIRE_NE(basePtr, nullptr);
        CHECK(basePtr->GetTypeInfo().IsType<BranchedOne>());
        CHECK(basePtr->GetTypeInfo().IsType<Derived>());
        CHECK(basePtr->GetTypeInfo().IsType<Base>());

        derivedPtr = Reflection::Cast<Derived>(basePtr);
        REQUIRE_NE(derivedPtr, nullptr);
        CHECK(derivedPtr->GetTypeInfo().IsType<BranchedOne>());
        CHECK(derivedPtr->GetTypeInfo().IsType<Derived>());
        CHECK(derivedPtr->GetTypeInfo().IsType<Base>());

        branchedOnePtr = Reflection::Cast<BranchedOne>(derivedPtr);
        REQUIRE_NE(branchedOnePtr, nullptr);
        CHECK_EQ(branchedOnePtr->inner.value, 42);
        CHECK(branchedOnePtr->GetTypeInfo().IsType<BranchedOne>());
        CHECK(branchedOnePtr->GetTypeInfo().IsType<Derived>());
        CHECK(branchedOnePtr->GetTypeInfo().IsType<Base>());

        branchedOnePtr = Reflection::Cast<BranchedOne>(basePtr);
        REQUIRE_NE(branchedOnePtr, nullptr);
        CHECK_EQ(branchedOnePtr->inner.value, 42);
        CHECK(branchedOnePtr->GetTypeInfo().IsType<BranchedOne>());
        CHECK(branchedOnePtr->GetTypeInfo().IsType<Derived>());
        CHECK(branchedOnePtr->GetTypeInfo().IsType<Base>());

        BranchedTwo* branchedTwoPtr = Reflection::Cast<BranchedTwo>(branchedOnePtr);
        CHECK_EQ(branchedTwoPtr, nullptr);

        branchedTwoPtr = Reflection::Cast<BranchedTwo>(derivedPtr);
        CHECK_EQ(branchedTwoPtr, nullptr);

        branchedTwoPtr = Reflection::Cast<BranchedTwo>(basePtr);
        CHECK_EQ(branchedTwoPtr, nullptr);
    }

    SUBCASE("Construct types from identifier")
    {
        CHECK(Reflection::StaticType<Derived>().IsConstructible());
        std::unique_ptr<Base> derived(Reflection::Construct<Derived>());
        CHECK(derived->GetTypeInfo().IsType<Derived>());

        CHECK(Reflection::DynamicType<BranchedOne>().IsConstructible());
        std::unique_ptr<Base> branchedOne(Reflection::Construct<Base>(
            Reflection::GetIdentifier<BranchedOne>()));
        CHECK(branchedOne->GetTypeInfo().IsType<BranchedOne>());

        CHECK(Reflection::DynamicType(Reflection::GetIdentifier<BranchedTwo>()).IsConstructible());
        std::unique_ptr<Base> branchedTwo(Reflection::Construct<Base>(
            Reflection::GetIdentifier<BranchedTwo>()));
        CHECK(branchedTwo->GetTypeInfo().IsType<BranchedTwo>());

        CHECK_FALSE(Reflection::StaticType<Reflection::TypeAttribute>().IsConstructible());
        auto* typeAttribute = Reflection::Construct<Reflection::TypeAttribute>();
        CHECK_EQ(typeAttribute, nullptr);

        CHECK_FALSE(Reflection::StaticType<Reflection::MethodAttribute>().IsConstructible());
        auto* methodAttribute = Reflection::Construct<Reflection::MethodAttribute>();
        CHECK_EQ(methodAttribute, nullptr);

        CHECK_FALSE(Reflection::StaticType<Reflection::FieldAttribute>().IsConstructible());
        auto* fieldAttribute = Reflection::Construct<Reflection::FieldAttribute>();
        CHECK_EQ(fieldAttribute, nullptr);

        SUBCASE("Cast constructed types to base type")
        {
            std::unique_ptr<Base> base;

            base = Reflection::Cast<Derived>(derived);
            CHECK(base->GetTypeInfo().IsType<Derived>());

            base = Reflection::Cast<Base>(branchedOne);
            CHECK(base->GetTypeInfo().IsType<BranchedOne>());

            base = Reflection::Cast<Base>(branchedTwo);
            CHECK(base->GetTypeInfo().IsType<BranchedTwo>());
        }
    }
}
