/*
    Copyright (c) 2018-2021 Piotr Doan. All rights reserved.
    Software distributed under the permissive MIT License.
*/

#include "System/Precompiled.hpp"
#include "System/FileSystem/FileSystem.hpp"
#include "System/FileSystem/NativeFileDepot.hpp"
#include <Build/Build.hpp>
using namespace System;

namespace
{
    const char* CreateError = "Failed to create file system instance! {}";
}

FileSystem::FileSystem() = default;
FileSystem::~FileSystem() = default;

bool FileSystem::OnAttach(const Core::EngineSystemStorage& engineSystems)
{
    // Mount native working directory.
    if(auto workingDirectoryDepot = NativeFileDepot::Create("./"))
    {
        if(!MountDepot("./", workingDirectoryDepot.Unwrap()))
        {
            LOG_ERROR(CreateError, "Could not mount default working directory.");
            return false;
        }
    }
    else
    {
        LOG_ERROR(CreateError, "Could not create default working directory.");
        return false;
    }

    // Mount native engine directory.
    if(!Build::GetEngineDir().empty())
    {
        if(auto engineDirectoryDepot = NativeFileDepot::Create(Build::GetEngineDir()))
        {
            if(!MountDepot("./", engineDirectoryDepot.Unwrap()))
            {
                LOG_ERROR(CreateError, "Could not mount default engine directory.");
                return false;
            }
        }
        else
        {
            LOG_ERROR(CreateError, "Could not create default engine directory depot.");
            return false;
        }
    }

    // Mount native game directory.
    if(!Build::GetGameDir().empty())
    {
        if(auto gameDirectoryDepot = NativeFileDepot::Create(Build::GetGameDir()))
        {
            if(!MountDepot("./", gameDirectoryDepot.Unwrap()))
            {
                LOG_ERROR(CreateError, "Could not mount default game directory.");
                return false;
            }
        }
        else
        {
            LOG_ERROR(CreateError, "Could not create default game directory depot.");
            return false;
        }
    }

    // Success!
    return true;
}

FileSystem::MountDepotResult FileSystem::MountDepot(fs::path mountPath, FileDepotPtr&& fileDepot)
{
    CHECK_ARGUMENT_OR_RETURN(!mountPath.empty(),
        Common::Failure(MountDepotErrors::EmptyMountPathArgument));
    CHECK_ARGUMENT_OR_RETURN(fileDepot != nullptr,
        Common::Failure(MountDepotErrors::InvalidFileDepotArgument));

    if(mountPath.has_filename())
    {
        LOG_ERROR("Cannot mount path \"{}\" that contains filename!", mountPath.generic_string());
        return Common::Failure(MountDepotErrors::InvalidMountPathArgument);
    }

    m_mountedDepots.push_back({ mountPath.lexically_normal(), std::move(fileDepot) });

    return Common::Success();
}

FileDepot::OpenFileResult FileSystem::OpenFile(
    fs::path filePath, FileHandle::OpenFlags::Type openFlags)
{
    CHECK_ARGUMENT_OR_RETURN(!filePath.empty(),
        Common::Failure(FileDepot::OpenFileErrors::EmptyFilePathArgument));
    CHECK_ARGUMENT_OR_RETURN(openFlags != FileHandle::OpenFlags::None,
        Common::Failure(FileDepot::OpenFileErrors::InvalidOpenFlagsArgument));

    filePath = filePath.lexically_normal();

    if(!filePath.has_filename())
    {
        LOG_ERROR("Cannot open file from path \"{}\" that does not contain filename!",
            filePath.generic_string());
        return Common::Failure(FileDepot::OpenFileErrors::InvalidFilePathArgument);
    }

    for(auto entry = m_mountedDepots.crbegin(); entry != m_mountedDepots.crend(); ++entry)
    {
        const fs::path& mountPath = entry->mountPath;

        auto mountPathIt = mountPath.begin();
        auto filePathIt = filePath.begin();

        if(*mountPathIt == ".")
        {
            mountPathIt++;
        }

        while(mountPathIt != mountPath.end())
        {
            if(filePathIt == filePath.end())
                break;

            if(*mountPathIt != *filePathIt)
                break;

            ++mountPathIt;
        }

        if(mountPathIt == mountPath.end())
        {
            fs::path depotFilePath = std::accumulate(
                filePathIt, filePath.end(), fs::path(), std::divides());

            if(auto openFileResult = entry->fileDepot->OpenFile(depotFilePath, filePath, openFlags))
            {
                LOG_SUCCESS("Opened \"{}\" file.", filePath.generic_string());
                return openFileResult;
            }
            else
            {
                FileDepot::OpenFileErrors openFileError = openFileResult.UnwrapFailure();
                if(openFileError != FileDepot::OpenFileErrors::FileNotFound)
                {
                    return Common::Failure(openFileError);
                }
            }
        }
    }

    LOG_ERROR("Could not open \"{}\" file!", filePath.generic_string());
    return Common::Failure(FileDepot::OpenFileErrors::FileNotFound);
}
