/*  This file is part of the KDE project
    Copyright (C) 1999 Simon Hausmann <hausmann@kde.org>
    Copyright (C) 1999 David Faure <faure@kde.org>
    Copyright (C) 1999 Torben Weis <weis@kde.org>
 
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
 
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
 
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 
*/ 
#include <kactivator.h>

#include "konq_factory.h"
#include "konq_mainview.h"
#include "konq_iconview.h"
#include "konq_treeview.h"
#include "konq_txtview.h"
#include "konq_htmlview.h"

#include <kded_instance.h>
#include <ktrader.h>

Browser::View_ptr KonqFactory::createView( const QString &serviceType, 
			                   QStringList &serviceTypes,
			                   KonqMainView *mainView,
                                           Konqueror::DirectoryDisplayMode dirMode )
{
  serviceTypes.clear();

  kdebug(0,1202,"trying to create view for %s", serviceType.ascii());
  
  //check for builtin views first
  if ( serviceType == "inode/directory" )
  {
    serviceTypes.append( serviceType );
    if ( dirMode != Konqueror::TreeView )
    {
      KonqKfmIconView *iconView = new KonqKfmIconView( mainView );
      iconView->setViewMode( dirMode );
      return Browser::View::_duplicate( iconView );
    }
    else
      return Browser::View::_duplicate( new KonqKfmTreeView( mainView ) );
  }
  else if ( serviceType == "text/html" )
  {
    serviceTypes.append( serviceType );
    return Browser::View::_duplicate( new KonqHTMLView( mainView ) );
  }
  else if ( serviceType.left( 5 ) == "text/" &&
            ( serviceType.mid( 5, 2 ) == "x-" ||
	      serviceType.mid( 5 ) == "english" ||
	      serviceType.mid( 5 ) == "plain" ) )
  {
    serviceTypes.append( serviceType );
    return Browser::View::_duplicate( new KonqTxtView( mainView ) );
  }
  
  //now let's query the Trader for view plugins
  KTrader *trader = KdedInstance::self()->ktrader();
  KActivator *activator = KdedInstance::self()->kactivator();
  
  KTrader::OfferList offers = trader->query( serviceType, "'Browser/View' in ServiceTypes" );
  
  if ( offers.count() == 0 ) //no results?
    return Browser::View::_nil();
    
  //activate the view plugin
  KService::Ptr service = offers.first();
  
  if ( service->repoIds().count() == 0 )  //uh...is it a CORBA service at all??
    return Browser::View::_nil();
 
  QString repoId = service->repoIds().first();
  QString tag = service->name(); //use service name as default tag
  int tagPos = repoId.find( "#" );
  if ( tagPos != -1 )
  {
    tag = repoId.mid( tagPos + 1 );
    repoId.truncate( tagPos );
  }  

  CORBA::Object_var obj = activator->activateService( service->name().latin1(), repoId.latin1(), tag.latin1() );
  
  Browser::ViewFactory_var factory = Browser::ViewFactory::_narrow( obj );
  
  if ( CORBA::is_nil( factory ) )
    return Browser::View::_nil();

  serviceTypes = service->serviceTypes();

  return factory->create();
}
