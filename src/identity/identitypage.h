/*
 *   kmail: KDE mail client
 *   Copyright (C) 2000 Espen Sand, espen@kde.org
 *   Copyright (C) 2001-2003 Marc Mutz, mutz@kde.org
 *   Contains code segments and ideas from earlier kmail dialog code.
 *   Copyright (C) 2010 Volker Krause <vkrause@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License along
 *   with this program; if not, write to the Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef IDENTITYPAGE_H
#define IDENTITYPAGE_H

#include "kmail_export.h"

#include "configuredialog/configuredialog_p.h"
#include "ui_identitypage.h"
namespace KIdentityManagement {
class IdentityManager;
}

namespace KMail {
class IdentityDialog;
class IdentityListView;
class IdentityListViewItem;

class KMAIL_EXPORT IdentityPage : public ConfigModuleTab
{
    Q_OBJECT
public:
    explicit IdentityPage(QWidget *parent = nullptr);
    ~IdentityPage() override;

    Q_REQUIRED_RESULT QString helpAnchor() const;

    void load();
    void save() override;

private:
    void slotNewIdentity();
    void slotModifyIdentity();
    void slotRemoveIdentity();
    /** Connected to @p mRenameButton's clicked() signal. Just does a
      QTreeWidget::editItem on the selected item */
    void slotRenameIdentity();
    /** connected to @p mIdentityList's renamed() signal. Validates the
      new name and sets it in the KIdentityManagement::IdentityManager */
    void slotRenameIdentityFromItem(KMail::IdentityListViewItem *, const QString &);
    void slotContextMenu(KMail::IdentityListViewItem *, const QPoint &);
    void slotSetAsDefault();
    void slotIdentitySelectionChanged();

    void refreshList();
    void updateButtons();

private: // data members
    Ui_IdentityPage mIPage;
    KMail::IdentityDialog *mIdentityDialog = nullptr;
    int mOldNumberOfIdentities = 0;
    KIdentityManagement::IdentityManager *mIdentityManager = nullptr;
};
}

#endif
