/* This file is part of the KDE project
   Copyright (C) 1998, 1999 David Faure <faure@kde.org>
   Copyright (C) 2001 Holger Freyther <freyther@yahoo.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef __konqpopupmenu_h
#define __konqpopupmenu_h

#include <sys/types.h>

#include <qpopupmenu.h>
#include <qmap.h>
#include <kaction.h>

#include <qstringlist.h>

#include <kfileitem.h>
#include <kmimetype.h> // for KDEDesktopMimeType

#include "konq_xmlguiclient.h"

class KNewMenu;
class KService;
class KonqPopupMenuPlugin;
class KBookmarkManager;
/**
 * This class implements the popup menu for URLs in konqueror and kdesktop
 * It's usage is very simple : on right click, create the KonqPopupMenu instance
 * with the correct arguments, then exec() to make it appear, then destroy it.
 *
 */
class KonqPopupMenu : public QPopupMenu, public KonqXMLGUIClient
{
  Q_OBJECT
public:

  /**
   * Constructor
   * @param items the list of file items the popupmenu should be shown for
   * @param viewURL the URL shown in the view, to test for RMB click on view background
   * @param actions list of actions the caller wants to see in the menu
   * @param newMenu "New" menu, shared with the File menu, in konqueror
   * @param allowEmbeddingServices whether to allow services to be embedded
   * (true in konqueror, false in kdesktop and others)
   *
   * The actions to pass in include :
   * showmenubar, back, forward, up, cut, copy, paste, pasteto, trash, rename, del, shred
   * The others items are automatically inserted.
   *
   */
  KonqPopupMenu( KBookmarkManager* manager,
                 const KFileItemList &items,
                 KURL viewURL,
                 KActionCollection & actions,
                 KNewMenu * newMenu, bool showPropertiesAndFileType = true );
  /**
   * Don't forget to destroy the object
   */
  ~KonqPopupMenu();

  /**
   * Set the title of the URL, when the popupmenu is opened for a single URL.
   * This is used if the user chooses to add a bookmark for this URL.
   */
  void setURLTitle( const QString& urlTitle );

  class ProtocolInfo {
   public:
    ProtocolInfo();
    bool supportsReading()  const;
    bool supportsWriting()  const;
    bool supportsDeleting() const;
    bool supportsMoving()   const;
    bool trashIncluded()    const;
   private:
    friend class KonqPopupMenu;
    bool m_Reading:1;
    bool m_Writing:1;
    bool m_Deleting:1;
    bool m_Moving:1;
    bool m_TrashIncluded:1;
  };
  /**
   * Reimplemented for internal purpose
   */
  virtual KAction *action( const QDomElement &element ) const;


  virtual KActionCollection *actionCollection() const;
  QString mimeType( ) const;
  KURL url( ) const;
  KFileItemList fileItemList() const;
  KURL::List popupURLList( ) const;
  ProtocolInfo protocolInfo() const;

public slots:
  void slotPopupNewDir();
  void slotPopupNewView();
  void slotPopupEmptyTrashBin();
  void slotPopupOpenWith();
  void slotPopupAddToBookmark();
  void slotRunService();
  void slotPopupMimeType();
  void slotPopupProperties();
  void slotOpenShareFileDialog();
protected:
  KActionCollection &m_actions;
  KActionCollection m_ownActions;

private:
  void addPlugins( );
  class KonqPopupMenuPrivate;
  KonqPopupMenuPrivate *d;
  KNewMenu *m_pMenuNew;
  KURL m_sViewURL;
  QString m_sMimeType;
  KFileItemList m_lstItems;
  KURL::List m_lstPopupURLs;
  QMap<int,KService::Ptr> m_mapPopup;
  QMap<int,KDEDesktopMimeType::Service> m_mapPopupServices;
  bool m_bHandleEditOperations;
  KXMLGUIFactory *m_factory;
  KXMLGUIBuilder *m_builder;
  QString attrName;
  ProtocolInfo m_info;
  QPtrList<KonqPopupMenuPlugin> m_pluginList;
  KBookmarkManager* m_pManager;
};

class KonqPopupMenuPlugin : public QObject, public KonqXMLGUIClient {
	Q_OBJECT
public:
  /**
  * Constructor
  * If you want to insert a dynamic item or menu to konqpopupmenu
  * this class is the right choice.
  * Create a KAction and use _popup->addAction(new KAction );
  * If you want to create a submenu use _popup->addGroup( );
  */
  KonqPopupMenuPlugin( KonqPopupMenu *_popup, const char *name ); // this should also be the parent
  virtual ~KonqPopupMenuPlugin ( );
};

#endif

