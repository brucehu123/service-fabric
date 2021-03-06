// ------------------------------------------------------------
// Copyright (c) Microsoft Corporation.  All rights reserved.
// Licensed under the MIT License (MIT). See License.txt in the repo root for license information.
// ------------------------------------------------------------

#include "stdafx.h"

using namespace ktl;
using namespace Common;
using namespace TxnReplicator;
using namespace TxnReplicator::TestCommon;
using namespace Data;
using namespace Data::Utilities;

TestBackupCallbackHandler::SPtr TestBackupCallbackHandler::Create(
    __in KString const & externalBackupFolderPath,
    __in KAllocator & allocator)
{
    TestBackupCallbackHandler * result = _new(TEST_BACKUP_CALLBACK_HANDLER_TAG, allocator) TestBackupCallbackHandler(externalBackupFolderPath);

    CODING_ERROR_ASSERT(result != nullptr);
    CODING_ERROR_ASSERT(NT_SUCCESS(result->Status()));

    return SPtr(result);
}

ktl::Awaitable<bool> TestBackupCallbackHandler::UploadBackupAsync(
    __in BackupInfo backupInfo,
    __in CancellationToken const & cancellationToken)
{
    if (backupCallbackACSSPtr_ != nullptr)
    {
        co_await backupCallbackACSSPtr_->GetAwaitable();
    }

    NTSTATUS status = backupInfoArray_.Append(backupInfo);
    THROW_ON_FAILURE(status);

    wstring backupDirectory(backupInfo.BackupId.ToString());
    KStringView backupIdString(backupDirectory.c_str());
    KString::SPtr storeFolderSPtr = Data::Utilities::KPath::Combine(*externalBackupFolderPath_, backupIdString, GetThisAllocator());

    ErrorCode errorCode = Directory::Copy(wstring(*backupInfo.Directory), wstring(*storeFolderSPtr), false);

    co_return true;
}

void TestBackupCallbackHandler::SetBackupCallbackACS(
    __in AwaitableCompletionSource<void> & acs) noexcept
{
    backupCallbackACSSPtr_ = &acs;
}

TestBackupCallbackHandler::TestBackupCallbackHandler(
    __in KString const & externalBackupFolderPath)
    : externalBackupFolderPath_(&externalBackupFolderPath)
    , backupInfoArray_(GetThisAllocator())
{
    SetConstructorStatus(backupInfoArray_.Status());
}

TestBackupCallbackHandler::~TestBackupCallbackHandler()
{
}
