/*
  Copyright (c) 2015-2020 Laurent Montel <montel@kde.org>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.

*/

#include "markallmessagesasreadinfolderandsubfolderjob.h"
#include "kmail_debug.h"

#include <AkonadiCore/CollectionFetchJob>
#include <AkonadiCore/CollectionFetchScope>

MarkAllMessagesAsReadInFolderAndSubFolderJob::MarkAllMessagesAsReadInFolderAndSubFolderJob(QObject *parent)
    : QObject(parent)
{
}

MarkAllMessagesAsReadInFolderAndSubFolderJob::~MarkAllMessagesAsReadInFolderAndSubFolderJob()
{
}

void MarkAllMessagesAsReadInFolderAndSubFolderJob::setTopLevelCollection(const Akonadi::Collection &topLevelCollection)
{
    mTopLevelCollection = topLevelCollection;
}

void MarkAllMessagesAsReadInFolderAndSubFolderJob::start()
{
    if (mTopLevelCollection.isValid()) {
        auto fetchJob = new Akonadi::CollectionFetchJob(mTopLevelCollection, Akonadi::CollectionFetchJob::Recursive, this);
        fetchJob->fetchScope().setAncestorRetrieval(Akonadi::CollectionFetchScope::All);
        connect(fetchJob, &Akonadi::CollectionFetchJob::finished,
                this, [this](KJob *job) {
            if (job->error()) {
                qCWarning(KMAIL_LOG) << job->errorString();
                slotFetchCollectionFailed();
            } else {
                auto fetch = static_cast<Akonadi::CollectionFetchJob *>(job);
                slotFetchCollectionDone(fetch->collections());
            }
        });
    } else {
        qCDebug(KMAIL_LOG()) << "Invalid toplevel collection";
        deleteLater();
    }
}

void MarkAllMessagesAsReadInFolderAndSubFolderJob::slotFetchCollectionFailed()
{
    qCDebug(KMAIL_LOG()) << "Fetch toplevel collection failed";
    deleteLater();
}

void MarkAllMessagesAsReadInFolderAndSubFolderJob::slotFetchCollectionDone(const Akonadi::Collection::List &list)
{
    Akonadi::MessageStatus messageStatus;
    messageStatus.setRead(true);
    Akonadi::MarkAsCommand *markAsReadAllJob = new Akonadi::MarkAsCommand(messageStatus, list);
    connect(markAsReadAllJob, &Akonadi::MarkAsCommand::result, this, &MarkAllMessagesAsReadInFolderAndSubFolderJob::slotMarkAsResult);
    markAsReadAllJob->execute();
}

void MarkAllMessagesAsReadInFolderAndSubFolderJob::slotMarkAsResult(Akonadi::MarkAsCommand::Result result)
{
    switch (result) {
    case Akonadi::MarkAsCommand::Undefined:
        qCDebug(KMAIL_LOG()) << "MarkAllMessagesAsReadInFolderAndSubFolderJob undefined result";
        break;
    case Akonadi::MarkAsCommand::OK:
        qCDebug(KMAIL_LOG()) << "MarkAllMessagesAsReadInFolderAndSubFoldeJob Done";
        break;
    case Akonadi::MarkAsCommand::Canceled:
        qCDebug(KMAIL_LOG()) << "MarkAllMessagesAsReadInFolderAndSubFoldeJob was canceled";
        break;
    case Akonadi::MarkAsCommand::Failed:
        qCDebug(KMAIL_LOG()) << "MarkAllMessagesAsReadInFolderAndSubFoldeJob was failed";
        break;
    }
    deleteLater();
}
