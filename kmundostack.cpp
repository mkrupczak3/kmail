/*
    This file is part of KMail

    Copyright (C) 1999 Waldo Bastian (bastian@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    version 2 as published by the Free Software Foundation.

    This software is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this library; see the file COPYING. If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include "kmundostack.h"

#include "kmmainwin.h"
#include "kmkernel.h"
#include "kmfolder.h"
#include "kmmessage.h"
#include "kmmsgdict.h"

#include <kmessagebox.h>
#include <klocale.h>
#include <kdebug.h>


KMUndoStack::KMUndoStack(int size)
  : QObject(0, "undostack"), mSize(size), mLastId(0),
    mCachedInfo(0)
{
   mStack.setAutoDelete(true);
}

void KMUndoStack::clear()
{
   mStack.clear();
}

int KMUndoStack::newUndoAction( KMFolder *srcFolder, KMFolder *destFolder )
{
  KMUndoInfo *info = new KMUndoInfo;
  info->id         = ++mLastId;
  info->srcFolder  = srcFolder;
  info->destFolder = destFolder;
  if ((int) mStack.count() == mSize)
    mStack.removeLast();
  mStack.prepend( info );
  emit undoStackChanged();
  return info->id;
}

void KMUndoStack::addMsgToAction( int undoId, ulong serNum )
{
  if ( !mCachedInfo || mCachedInfo->id != undoId ) {
    QPtrListIterator<KMUndoInfo> itr( mStack );
    while ( itr.current() ) {
      if ( itr.current()->id == undoId ) {
        mCachedInfo = itr.current();
        break;
      }
      ++itr;
    }
  }

  Q_ASSERT( mCachedInfo );
  mCachedInfo->serNums.append( serNum );
}

void KMUndoStack::undo()
{
  KMMessage *msg;
  ulong serNum;
  int idx = -1;
  KMFolder *curFolder;
  if ( mStack.count() > 0 )
  {
    KMUndoInfo *info = mStack.take(0);
    emit undoStackChanged();
    QValueList<ulong>::iterator itr;
    info->destFolder->open();
    for( itr = info->serNums.begin(); itr != info->serNums.end(); ++itr ) {
      serNum = *itr;
      kernel->msgDict()->getLocation(serNum, &curFolder, &idx);
      if ( idx == -1 || curFolder != info->destFolder ) {
        kdDebug(5006)<<"Serious undo error!"<<endl;
        delete info;
        return;
      }
      msg = curFolder->getMsg( idx );
      info->srcFolder->moveMsg( msg );
      if ( info->srcFolder->count() > 1 )
        info->srcFolder->unGetMsg( info->srcFolder->count() - 1 );
    }
    info->destFolder->close();
    delete info;
  }
  else
  {
    // Sorry.. stack is empty..
    KMessageBox::sorry( kernel->mainWin(), i18n("There is nothing to undo!"));
  }
}

void
KMUndoStack::pushSingleAction(ulong serNum, KMFolder *folder, KMFolder *destFolder)
{
  int id = newUndoAction( folder, destFolder );
  addMsgToAction( id, serNum );
}

void
KMUndoStack::msgDestroyed( KMMsgBase* /*msg*/)
{
  /*
   for(KMUndoInfo *info = mStack.first(); info; )
   {
      if (info->msgIdMD5 == msg->msgIdMD5())
      {
         mStack.removeRef( info );
         info = mStack.current();
      }
      else
         info = mStack.next();
   }
  */
}

void
KMUndoStack::folderDestroyed( KMFolder *folder)
{
   for(KMUndoInfo *info = mStack.first(); info; )
   {
      if ((info->srcFolder == folder) ||
	  (info->destFolder == folder))
      {
         mStack.removeRef( info );
         info = mStack.current();
      }
      else
         info = mStack.next();
   }
   emit undoStackChanged();
}


#include "kmundostack.moc"
