// kmreadermainwin
// (c) 2002 Don Sanders <sanders@kde.org>
// License: GPL
//
// A toplevel KMainWindow derived class for displaying
// single messages or single message parts.
//
// Could be extended to include support for normal main window
// widgets like a toolbar.

#include <qaccel.h>
#include <kapplication.h>
#include <klocale.h>
#include <kstdaccel.h>
#include <kwin.h>
#include <kaction.h>

#include "kmcommands.h"
#include "kmenubar.h"
#include "kpopupmenu.h"
#include "kmreaderwin.h"
#include "kmfolderindex.h"

#include "kmreadermainwin.h"
#include "kmreadermainwin.moc"

KMReaderMainWin::KMReaderMainWin( bool htmlOverride, char *name )
  : KMTopLevelWidget( name ), mMsg( 0 )
{
  KWin::setIcons(winId(), kapp->icon(), kapp->miniIcon());
  mReaderWin = new KMReaderWin( this, this, actionCollection() );
  //mReaderWin->setShowCompleteMessage( true );
  mReaderWin->setAutoDelete( true );
  mReaderWin->setHtmlOverride( htmlOverride );
  setCentralWidget( mReaderWin );
  setupAccel();
}


KMReaderMainWin::KMReaderMainWin( char *name )
  : KMTopLevelWidget( name ), mMsg( 0 )
{
  mReaderWin = new KMReaderWin( this, this, actionCollection() );
  mReaderWin->setAutoDelete( true );
  setCentralWidget( mReaderWin );
  setupAccel();
}


KMReaderMainWin::KMReaderMainWin(KMMessagePart* aMsgPart,
    bool aHTML, const QString& aFileName, const QString& pname,
    const QTextCodec *codec, char *name )
  : KMTopLevelWidget( name ), mMsg( 0 )
{
  resize( 550, 600 );
  mReaderWin = new KMReaderWin( this, this, actionCollection() ); //new reader
  mReaderWin->setOverrideCodec( codec );
  mReaderWin->setMsgPart( aMsgPart, aHTML, aFileName, pname );
  setCentralWidget( mReaderWin );
  setupAccel();
}


KMReaderMainWin::~KMReaderMainWin()
{
  saveMainWindowSettings(KMKernel::config(), "Separate Reader Window");
}


void KMReaderMainWin::showMsg( const QTextCodec *codec, KMMessage *msg )
{
  mReaderWin->setOverrideCodec( codec );
  mReaderWin->setMsg( msg, true );
  setCaption( msg->subject() );
  mMsg = msg;
}


void KMReaderMainWin::setupAccel()
{
  if (kernel->xmlGuiInstance())
    setInstance( kernel->xmlGuiInstance() );
  applyMainWindowSettings(KMKernel::config(), "Separate Reader Window");
  QAccel *accel = new QAccel(mReaderWin, "showMsg()");
  accel->connectItem(accel->insertItem(Key_Up),
                     mReaderWin, SLOT(slotScrollUp()));
  accel->connectItem(accel->insertItem(Key_Down),
                     mReaderWin, SLOT(slotScrollDown()));
  accel->connectItem(accel->insertItem(Key_Prior),
                     mReaderWin, SLOT(slotScrollPrior()));
  accel->connectItem(accel->insertItem(Key_Next),
                     mReaderWin, SLOT(slotScrollNext()));
  accel->connectItem(accel->insertItem(KStdAccel::shortcut(KStdAccel::Copy)),
                     mReaderWin, SLOT(slotCopySelectedText()));
  connect( mReaderWin, SIGNAL(popupMenu(KMMessage&,const KURL&,const QPoint&)),
	  this, SLOT(slotMsgPopup(KMMessage&,const KURL&,const QPoint&)));
  connect(mReaderWin, SIGNAL(urlClicked(const KURL&,int)),
	  mReaderWin, SLOT(slotUrlClicked()));
  createGUI( "kmreadermainwin.rc" );
  menuBar()->hide();
}


void KMReaderMainWin::slotMsgPopup(KMMessage &aMsg, const KURL &aUrl, const QPoint& aPoint)
{
  KPopupMenu * menu = new KPopupMenu;
  mUrl = aUrl;
  mMsg = &aMsg;

  if (!aUrl.isEmpty()) {
    if (aUrl.protocol() == "mailto") {
      // popup on a mailto URL
      mReaderWin->mailToComposeAction()->plug( menu );
      if ( mMsg ) {
	mReaderWin->mailToReplyAction()->plug( menu );
	mReaderWin->mailToForwardAction()->plug( menu );
        menu->insertSeparator();
      }
      mReaderWin->addAddrBookAction()->plug( menu );
      mReaderWin->openAddrBookAction()->plug( menu );
      mReaderWin->copyAction()->plug( menu );
    } else {
      // popup on a not-mailto URL
      mReaderWin->urlOpenAction()->plug( menu );
      mReaderWin->urlSaveAsAction()->plug( menu );
      mReaderWin->copyURLAction()->plug( menu );
      mReaderWin->addBookmarksAction()->plug( menu );
    }
  } else {
    // popup somewhere else (i.e., not a URL) on the message

    if (!mMsg) // no message
    {
      delete menu;
      return;
    }

    mReaderWin->replyAction()->plug( menu );
    mReaderWin->replyAllAction()->plug( menu );
    mReaderWin->forwardMenu()->plug(menu);
    mReaderWin->bounceAction()->plug( menu );
    menu->insertSeparator();
    mReaderWin->updateListFilterAction();
    mReaderWin->filterMenu()->plug( menu );
    QPopupMenu* copyMenu = new QPopupMenu(menu);
    KMMenuCommand::folderToPopupMenu( false, this, &mMenuToFolder, copyMenu );
    menu->insertItem( i18n("&Copy To" ), copyMenu );
    menu->insertSeparator();
    mReaderWin->toggleFixFontAction()->plug( menu );
    mReaderWin->viewSourceAction()->plug( menu );
    mReaderWin->printAction()->plug( menu );
    menu->insertItem( i18n("Save &As..."), mReaderWin, SLOT(slotSaveMsg()) );
    menu->insertItem( i18n("Save Attachments..."), mReaderWin, SLOT(slotSaveAttachments()) );
  }
  menu->exec(aPoint, 0);
  delete menu;
}

void KMReaderMainWin::copySelectedToFolder( int menuId )
{
  if (!mMenuToFolder[menuId])
    return;

  KMCommand *command = new KMCopyCommand( mMenuToFolder[menuId], mMsg );
  command->start();
}
