﻿#include "ReflectionGenerator.h"
#include <print>

ReflectionGenerator::ReflectionGenerator(const std::filesystem::path& logPath)
{
    std::println("Reflection Log Path: {}", logPath.string());

    create_directories(logPath.parent_path());
    _log = std::ofstream(logPath, std::ios::out);
    if (!_log.is_open())
    {
        std::println("ReflectionTool: Failed to open log file.");
    }
}

bool ReflectionGenerator::ProcessDirectoryRecursive(const std::filesystem::path& directory, bool force /*= false*/)
{
    std::println(_log, "Generating Reflection info for directory {}", directory.string());
    for (const auto& dirEntry : std::filesystem::recursive_directory_iterator(directory))
    {
        if (dirEntry.path().string().ends_with(".h"))
        {
            if (!dirEntry.path().string().ends_with(".reflection.h") &&
                !dirEntry.path().string().ends_with(".generated.h"))
            {
                std::filesystem::path reflectionHeaderPath = dirEntry.path().parent_path() / dirEntry.path().stem();
                reflectionHeaderPath += ".reflection.h";

                if (!exists(reflectionHeaderPath) ||
                    last_write_time(dirEntry) > last_write_time(reflectionHeaderPath) || force)
                {
                    if (GenerateReflectionHeader(dirEntry) == ReflectionResult::Failure)
                    {
                        std::println("ReflectionTool: Failed to generate reflection info for {}.", dirEntry.path().string());
                        return false;
                    }
                }
            }
        }
    }

    return true;
}

ReflectionGenerator::ReflectionResult ReflectionGenerator::GenerateReflectionHeader(const std::filesystem::path& header)
{
    std::println(_log, "Generating Reflection info for {}", header.string());

    if (!_parser.Parse(header))
    {
        std::println(_log, "Failed to parse {}", header.string());
        return ReflectionResult::Failure;
    }

    if (_parser.GetTypeInfos().empty())
    {
        return ReflectionResult::Skip;
    }

    std::filesystem::path reflectionHeaderPath = header.parent_path() / header.stem();
    reflectionHeaderPath += ".reflection.h";

    std::ofstream reflectionHeader(reflectionHeaderPath, std::ios::out);
    if (!reflectionHeader.is_open())
    {
        std::println(_log, "Failed to open {} for writing.", reflectionHeaderPath.string());
        return ReflectionResult::Failure;
    }

    const std::string headerName = header.stem().string();
    std::print(reflectionHeader,
               R"(#pragma once

#undef FILENAME
#define FILENAME {}
)",
               headerName);

    for (const TypeInfo& typeInfo : _parser.GetTypeInfos())
    {
        bool usesCustomSerialization = std::ranges::find_if(typeInfo.Attributes, [] (const Attribute& attribute)
        {
            return attribute.Name == "CustomSerialization";
        }) != typeInfo.Attributes.end();
        
        std::string createTypeFunction;
        std::stringstream parentTypeNames;
        if (typeInfo.ParentTypeNames.empty())
        {
            createTypeFunction = std::format("CreateRootType<{}>()", typeInfo.Name);
        }
        else
        {
            for (size_t i = 0; i < typeInfo.ParentTypeNames.size(); ++i)
            {
                parentTypeNames << typeInfo.ParentTypeNames[i];

                if (i < typeInfo.ParentTypeNames.size() - 1)
                {
                    parentTypeNames << ", ";
                }
            }
            createTypeFunction = std::format("CreateType<{}, {}>()", typeInfo.Name, parentTypeNames.view());
        }
        
        std::stringstream propertyMapDefinition;
        std::stringstream serializeDefinition;
        std::stringstream deserializeDefinition;
        if (!typeInfo.Properties.empty())
        {
            propertyMapDefinition << " \\\n";

            for (size_t i = 0; i < typeInfo.Properties.size(); ++i)
            {
                const PropertyInfo& property = typeInfo.Properties[i];

                std::stringstream propertyAttributes;

                propertyAttributes << "{";
                for (const Attribute& attribute : property.Attributes)
                {
                    propertyAttributes << std::format(R"({{"{}", "{}"}})", attribute.Name, attribute.Value);
                    if (&attribute != &property.Attributes.back())
                    {
                        propertyAttributes << ", ";
                    }
                }
                propertyAttributes << "}";

                std::print(propertyMapDefinition,
                           R"(                  .WithProperty("{}", TypeOf<UnderlyingType<{}>>(), &{}::{}, {}))",
                           property.Name,
                           property.Type,
                           typeInfo.Name,
                           property.Name,
                           propertyAttributes.view());
                if (i < typeInfo.Properties.size() - 1)
                {
                    propertyMapDefinition << " \\\n";
                }
            }
            propertyMapDefinition << " \\\n\t\t";

            if (!usesCustomSerialization)
            {
                for (const auto& property : typeInfo.Properties)
                {
                    if (std::ranges::find_if(property.Attributes,
                                             [](const Attribute& attribute)
                                             {
                                                 return attribute.Name == "Serialize";
                                             }) != property.Attributes.end())
                    {
                        std::println(serializeDefinition, R"(writer << {}; \)", property.Name);
                        std::println(deserializeDefinition, R"(reader >> {}; \)", property.Name);
                    }
                }

                std::print(serializeDefinition, "        \\");
                std::print(deserializeDefinition, "        \\");
            }
        }

        std::string dataOffsetDefinition;
        auto dataStartIt = std::ranges::find_if(typeInfo.Attributes,
                                                [](const Attribute& attribute)
                                                {
                                                    return attribute.Name == "DataStart";
                                                });
        if (dataStartIt != typeInfo.Attributes.end())
        {
            const Attribute& dataStart = *dataStartIt;

            if (!dataStart.Value.empty())
            {
                dataOffsetDefinition = std::format("->WithDataOffset(offsetof({}, {}))", typeInfo.Name, dataStart.Value);
            }
            else
            {
                std::println(_log, "Warning: DataStart attribute is missing value for class {} in file {}", typeInfo.Name, header.string());
            }
        }

        std::stringstream serialization;
        std::print(serialization, "\\");
        if (!usesCustomSerialization && !serializeDefinition.view().empty())
        {
            std::print(serialization,
R"(
    virtual bool Serialize(MemoryWriter& writer) const override \
    {{ \
        if (!Super<ISerializeable>::Serialize(writer)) \
        {{ \
            return false; \
        }} \
        {}
        return true; \
    }} \
    \
    virtual bool Deserialize(MemoryReader& reader) override \
    {{ \
        if (!Super<ISerializeable>::Deserialize(reader)) \
        {{ \
            return false; \
        }} \
        {}
        return true; \
    }} \
    \)", serializeDefinition.view(), deserializeDefinition.view());
        }

        // todo methods
        std::print(reflectionHeader,
                   R"(
#define GENERATED_{}_{}() \
    template <typename Interface> \
    using Super = typename FindSuperOfType<Interface, {}>::type; \
public: \
    static Type* StaticType() \
    {{ \
        static Type* staticType = TypeRegistry::Get().{}{} \
                ->WithPropertyMap(std::move(PropertyMap(){})); \
        return staticType; \
    }} \
    \
    virtual Type* GetType() const override \
    {{ \
        return StaticType(); \
    }} \
    \
    virtual std::shared_ptr<Object> Duplicate() const override \
    {{ \
        return std::shared_ptr<{}>(ClassBucketArray.Add(*this), ObjectDeleter());\
    }} \
    \
    virtual Object* DuplicateAt(void* ptr) const override \
    {{ \
        return new(ptr) {}(*this); \
    }} \
    {}
    std::shared_ptr<{}> SharedFromThis() \
    {{ \
        return std::static_pointer_cast<{}>(shared_from_this()); \
    }} \
    \
    std::shared_ptr<const {}> SharedFromThis() const \
    {{ \
        return std::static_pointer_cast<const {}>(shared_from_this()); \
    }} \
    \
private: \
    inline static BucketArray<{}> ClassBucketArray; \
private:
)",
                   headerName,
                   typeInfo.GeneratedMacroLine,
                   parentTypeNames.view(),
                   createTypeFunction,
                   dataOffsetDefinition,
                   propertyMapDefinition.view(),
                   typeInfo.Name,
                   typeInfo.Name,
                   serialization.view(),
                   typeInfo.Name,
                   typeInfo.Name,
                   typeInfo.Name,
                   typeInfo.Name,
                   typeInfo.Name);


        _reflectionInitializer.RegisterType(typeInfo.Name);
    }

    _reflectionInitializer.AddInclude(header);
    return ReflectionResult::Success;
}

void ReflectionGenerator::GenerateReflectionInitializer(const std::filesystem::path& outputDirectory, bool force /*= false*/)
{
    _reflectionInitializer.Generate(outputDirectory, _parser.GetEnumInfos(), force);
}
