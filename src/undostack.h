/*
    This file is part of KMail

    Copyright (C) 1999 Waldo Bastian (bastian@kde.org)
    Copyright (c) 2003 Zack Rusin <zack@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    version 2 as published by the Free Software Foundation.

    This software is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this library; see the file COPYING. If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef UNDOSTACK_H
#define UNDOSTACK_H

#include "kmail_private_export.h"
#include <QList>
#include <QObject>
#include <AkonadiCore/collection.h>
#include <AkonadiCore/item.h>

class KJob;

namespace KMail {
/** A class for storing Undo information. */
class UndoInfo
{
public:
    UndoInfo()
    {
    }

    int id = -1;
    Akonadi::Item::List items;
    Akonadi::Collection srcFolder;
    Akonadi::Collection destFolder;
    bool moveToTrash = false;
};

class KMAILTESTS_TESTS_EXPORT UndoStack : public QObject
{
    Q_OBJECT

public:
    explicit UndoStack(int size);
    ~UndoStack();

    void clear();
    Q_REQUIRED_RESULT int  size() const;
    Q_REQUIRED_RESULT int  newUndoAction(const Akonadi::Collection &srcFolder, const Akonadi::Collection &destFolder);
    void addMsgToAction(int undoId, const Akonadi::Item &item);
    Q_REQUIRED_RESULT bool isEmpty() const;
    void undo();

    void pushSingleAction(const Akonadi::Item &item, const Akonadi::Collection &, const Akonadi::Collection &destFolder);
    void folderDestroyed(const Akonadi::Collection &folder);

    Q_REQUIRED_RESULT QString undoInfo() const;

Q_SIGNALS:
    void undoStackChanged();

private:
    Q_DISABLE_COPY(UndoStack)
    void slotMoveResult(KJob *);
    QList<UndoInfo *> mStack;
    int mSize = 0;
    int mLastId = 0;
    UndoInfo *mCachedInfo = nullptr;
};
}

#endif
