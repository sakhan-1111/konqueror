/*

  This is an encapsulation of the  Netscape plugin API.


  Copyright (c) 2000 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
                     Stefan Schimanski <1Stein@gmx.de>

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


#include "NSPluginCallbackIface_stub.h"


#include <stdlib.h>
#include <unistd.h>

#include <qdir.h>
#include <qdict.h>
#include <qtimer.h>

#include "kxt.h"
#include "nsplugin.h"
#include "nsplugin.moc"
#include "resolve.h"

#include <klibloader.h>
#include <kdebug.h>
#include <kurl.h>
#include <kio/netaccess.h>
#include <ktempfile.h>
#include <dcopclient.h>
#include <kprotocolmanager.h>

#include <X11/Intrinsic.h>
#include <X11/Composite.h>
#include <X11/Constraint.h>
#include <X11/Shell.h>
#include <X11/StringDefs.h>
#include <Xm/DrawingA.h>

// server side functions -----------------------------------------------------

// allocate memory
void *g_NPN_MemAlloc(uint32 size)
{
   void *mem = ::malloc(size);

   kdDebug(1431) << "g_NPN_MemAlloc(), size=" << size << " allocated at " << mem << endl;

   return mem;
}


// free memory
void g_NPN_MemFree(void *ptr)
{
   kdDebug(1431) << "g_NPN_MemFree() at " << ptr << endl;

   ::free(ptr);
}

uint32 g_NPN_MemFlush(uint32 /*size*/)
{
   kdDebug(1431) << "g_NPN_MemFlush()" << endl;
   return 0;
}


// redraw
void g_NPN_ForceRedraw(NPP /*instance*/)
{
   kdDebug(1431) << "g_NPN_ForceRedraw() [unimplemented]" << endl;
}


// invalidate rect
void g_NPN_InvalidateRect(NPP /*instance*/, NPRect */*invalidRect*/)
{
   kdDebug(1431) << "g_NPN_InvalidateRect() [unimplemented]" << endl;
}


// invalidate region
void g_NPN_InvalidateRegion(NPP /*instance*/, NPRegion /*invalidRegion*/)
{
   kdDebug(1431) << "g_NPN_InvalidateRegion() [unimplemented]" << endl;
}


// get value
NPError g_NPN_GetValue(NPP /*instance*/, NPNVariable variable, void *value)
{
   kdDebug(1431) << "g_NPN_GetValue(), variable=" << static_cast<int>(variable) << endl;

   switch (variable)
   {
      case NPNVxDisplay:
         *(void**)value = qt_xdisplay();
         return NPERR_NO_ERROR;
      case NPNVxtAppContext:
         *(void**)value = XtDisplayToApplicationContext(qt_xdisplay());
         return NPERR_NO_ERROR;
#ifdef NP4
      case NPNVjavascriptEnabledBool:
         (bool)(*value) = false; // FIXME: Let's have a talk with Harri :-)
         return NPERR_NO_ERROR;
      case NPNVasdEnabledBool:
         (bool)(*value) = false; // FIXME: No clue what this means...
         return NPERR_NO_ERROR;
      case NPNVOfflineBool:
         (bool)(*value) = false;
         return NPERR_NO_ERROR;
#endif
      default:
         return NPERR_INVALID_PARAM;
   }
}


NPError g_NPN_DestroyStream(NPP instance, NPStream* stream,
                          NPReason /*reason*/)
{
   kdDebug(1431) << "g_NPN_DestroyStream()" << endl;

   NSPluginInstance *inst = (NSPluginInstance*) instance->ndata;
   inst->streamFinished( (NSPluginStream *)stream->ndata );

   return NPERR_GENERIC_ERROR;
}


NPError g_NPN_RequestRead(NPStream */*stream*/, NPByteRange */*rangeList*/)
{
   kdDebug(1431) << "g_NPN_RequestRead() [unimplemented]" << endl;

   return NPERR_GENERIC_ERROR;
}

NPError g_NPN_NewStream(NPP /*instance*/, NPMIMEType /*type*/,
                      const char* /*target*/, NPStream** /*stream*/)
{
   kdDebug(1431) << "g_NPN_NewStream() [unimplemented]" << endl;

   return NPERR_GENERIC_ERROR;
}

int32 g_NPN_Write(NPP /*instance*/, NPStream */*stream*/, int32 /*len*/, void */*buf*/)
{
   kdDebug(1431) << "g_NPN_Write() [unimplemented]" << endl;

   return 0;
}


// URL functions
NPError g_NPN_GetURL(NPP instance, const char *url, const char *target)
{
   kdDebug(1431) << "g_NPN_GetURL: url=" << url << " target=" << target << endl;

   NSPluginInstance *inst = (NSPluginInstance*) instance->ndata;
   inst->requestURL( QString::fromLatin1(url), QString::fromLatin1(target), 0 );

   return NPERR_NO_ERROR;
}


NPError g_NPN_GetURLNotify(NPP instance, const char *url, const char *target,
                         void* notifyData)
{
   kdDebug(1431) << "g_NPN_GetURLNotify: url=" << url << " target=" << target << endl;

   NSPluginInstance *inst = (NSPluginInstance*) instance->ndata;
   inst->requestURL( url, target, notifyData );

   return NPERR_NO_ERROR;
}


NPError g_NPN_PostURL(NPP /*instance*/, const char */*url*/, const char */*target*/,
                    uint32 /*len*/, const char */*buf*/, NPBool /*file*/)
{
   kdDebug(1431) << "g_NPN_PostURL() [unimplemented]" << endl;

   return NPERR_GENERIC_ERROR;
}


NPError g_NPN_PostURLNotify(NPP /*instance*/, const char */*url*/, const char */*target*/,
                          uint32 /*len*/, const char */*buf*/, NPBool /*file*/, void */*notifyData*/)
{
   kdDebug(1431) << "g_NPN_PostURL() [unimplemented]" << endl;

   return NPERR_GENERIC_ERROR;
}


// display status message
void g_NPN_Status(NPP instance, const char *message)
{
   kdDebug(1431) << "g_NPN_Status(): " << message << endl;

   if (!instance)
      return;

   // turn into an instance signal
   NSPluginInstance *inst = (NSPluginInstance*) instance->ndata;

   inst->emitStatus(message);
}


// inquire user agent
const char *g_NPN_UserAgent(NPP /*instance*/)
{
   kdDebug(1431) << "g_NPN_UserAgent()" << endl;
   QString agent = DEFAULT_USERAGENT_STRING;
   return agent.latin1();
}


// inquire version information
void g_NPN_Version(int *plugin_major, int *plugin_minor, int *browser_major, int *browser_minor)
{
   kdDebug(1431) << "g_NPN_Version()" << endl;

   // FIXME: Use the sensible values
   *browser_major = NP_VERSION_MAJOR;
   *browser_minor = NP_VERSION_MINOR;

   *plugin_major = NP_VERSION_MAJOR;
   *plugin_minor = NP_VERSION_MINOR;
}


void g_NPN_ReloadPlugins(NPBool /*reloadPages*/)
{
   kdDebug(1431) << "g_NPN_ReloadPlugins() [unimplemented]" << endl;
}


// JAVA functions
JRIEnv *g_NPN_GetJavaEnv()
{
   kdDebug(1431) << "g_NPN_GetJavaEnv() [unimplemented]" << endl;
   return 0;
}


jref g_NPN_GetJavaPeer(NPP /*instance*/)
{
   kdDebug(1431) << "g_NPN_GetJavaPeer() [unimplemented]" << endl;
   return 0;
}


NPError g_NPN_SetValue(NPP /*instance*/, NPPVariable /*variable*/, void */*value*/)
{
   kdDebug(1431) << "g_NPN_SetValue() [unimplemented]" << endl;

   return NPERR_GENERIC_ERROR;
}


/******************************************************************/

NSPluginInstance::NSPluginInstance(NPP privateData, NPPluginFuncs *pluginFuncs,
                                   KLibrary *handle, int width, int height,
                                   QString src, QString /*mime*/,
                                   QObject *parent )
   : QObject( parent ), DCOPObject(), _destroyed(false),
     _callback(0), _handle(handle), _width(width), _height(height)
{
   _npp = privateData;
   _npp->ndata = this;
   _src = src;
   memcpy(&_pluginFuncs, pluginFuncs, sizeof(_pluginFuncs));
   _tempFiles.setAutoDelete( true );
   _streams.setAutoDelete( true );
   _waitingRequests.setAutoDelete( true );
   _timer = new QTimer( this );
   connect( _timer, SIGNAL(timeout()), SLOT(timer()) );

   kdDebug(1431) << "NSPluginInstance::NSPluginInstance" << endl;
   kdDebug(1431) << "pdata = " << _npp->pdata << endl;
   kdDebug(1431) << "ndata = " << _npp->ndata << endl;

   // create drawing area
   Arg args[5];
   Cardinal nargs=0;
   XtSetArg(args[nargs], XtNwidth, width); nargs++;
   XtSetArg(args[nargs], XtNheight, height); nargs++;

   String n, c;
   XtGetApplicationNameAndClass(qt_xdisplay(), &n, &c);

   _toplevel = XtAppCreateShell("shell", c, topLevelShellWidgetClass,
                                qt_xdisplay(), args, nargs);

   // What exactly does widget mapping mean? Without this call the widget isn't
   // embedded correctly. With it the viewer doesn't show anything in standalone mode.
   XtSetMappedWhenManaged(_toplevel, False);
   XtRealizeWidget(_toplevel);

   // Create form window that is searched for by flash plugin
   _form = XtCreateManagedWidget("form", compositeWidgetClass, _toplevel, args, nargs);
   XtRealizeWidget(_form);

   // Create widget that is passed to the plugin
   _area = XmCreateDrawingArea(_form, (char*)("drawingArea"), args, nargs);
   //_area = XtCreateManagedWidget( "drawingArea", constraintWidgetClass, _form, args, nargs );
   XtRealizeWidget(_area);
   XtMapWidget(_area);

   setWindow();

   // create source stream
   if (!src.isEmpty())
   {
      kdDebug(1431) << "Starting src stream" << endl;
      requestURL( src, QString::null, 0 );
   } else
      kdDebug(1431) << "No src stream" << endl;
}

NSPluginInstance::~NSPluginInstance()
{
   kdDebug(1431) << "-> ~NSPluginInstance" << endl;

   kdDebug(1431) << "delete streams" << endl;
   _waitingRequests.clear();
   _streams.clear();

   kdDebug(1431) << "delete callbacks" << endl;
   delete _callback;
   _callback = 0;

   kdDebug(1431) << "destroy plugin" << endl;
   if ( _pluginFuncs.destroy )
       _pluginFuncs.destroy( _npp, 0 );

   XtDestroyWidget(_area);
   XtDestroyWidget(_form);
   XtDestroyWidget(_toplevel);

   ::free(_npp);

   kdDebug(1431) << "<- ~NSPluginInstance" << endl;
}

void NSPluginInstance::destroyPlugin()
{
   kdDebug(1431) << "-> NSPluginInstance::destroyPlugin" << endl;
   delete this;
   kdDebug(1431) << "<- NSPluginInstance::destroyPlugin" << endl;
}


void NSPluginInstance::timer()
{
    if ( _streams.count()==0 )
    {
        // start queued requests
        kdDebug(1431) << "looking for waiting requests" << endl;
        while ( _waitingRequests.head() ) {
            kdDebug(1431) << "request found" << endl;
            Request req( *_waitingRequests.head() );
            _waitingRequests.remove();

            // non empty target = frame target
            if ( !req.target.isEmpty() )
            {
                if (_callback)
                {
                    _callback->requestURL( req.url, req.target );
                    if ( req.notify )
                        NPURLNotify( req.url, NPRES_DONE, req.notify );
                }
            } else
            {
                if (!req.url.isEmpty())
                {
                    kdDebug(1431) << "Starting new stream " << req.url << endl;

                    // hack to get java vm working
                    if ( req.url.lower()=="javascript:document.location" ) {
                        NSPluginBufStream *s = new NSPluginBufStream( this );
                        connect( s, SIGNAL(finished(NSPluginStreamBase*)),
                                 SLOT(streamFinished(NSPluginStreamBase*)) );
                        _streams.append( s );

                        QByteArray buf;
                        buf.setRawData( _src.latin1(), _src.length() );
                        s->get( req.url, "text/html", buf, req.notify );
                    } else {
                        // create stream
                        NSPluginStream *s = new NSPluginStream( this );
                        connect( s, SIGNAL(finished(NSPluginStreamBase*)),
                                 SLOT(streamFinished(NSPluginStreamBase*)) );
                        _streams.append( s );

                        // make absolute url
                        if ( KURL::isRelativeURL(req.url) )
                        {
                            KURL absUrl( _src, req.url );
                            s->get( absUrl.url(), QString::null, req.notify );
                        } else
                            s->get( req.url, QString::null, req.notify );
                    }

                    break;
                }
            }
        }
    }

    // check again in 100ms
    if ( _waitingRequests.count()>0 )
        _timer->start( 100, true );
}


void NSPluginInstance::requestURL( const QString &url, const QString &target, void *notify )
{
    kdDebug(1431) << "NSPluginInstance::requestURL url=" << url << " target=" <<
        target << " notify=" << notify << endl;
    _waitingRequests.enqueue( new Request( url, target, notify ) );
    _timer->start( 100, true );
}


void NSPluginInstance::streamFinished( NSPluginStreamBase *strm )
{
   kdDebug(1431) << "-> NSPluginInstance::streamFinished" << endl;
   _streams.remove( strm );
}


int NSPluginInstance::setWindow(int remove)
{
   if (remove)
   {
      NPSetWindow(0);
      return NPERR_NO_ERROR;
   }

   kdDebug(1431) << "-> NSPluginInstance::setWindow" << endl;

   NPWindow win;
   NPSetWindowCallbackStruct win_info;

   win.x = 0;
   win.y = 0;
   win.height = _height;
   win.width = _width;
   win.type = NPWindowTypeWindow;

   // Well, the docu says sometimes, this is only used on the
   // MAC, but sometimes it says it's always. Who knows...
   NPRect clip;
   clip.top = 0;
   clip.left = 0;
   clip.bottom = _height;
   clip.right = _width;
   win.clipRect = clip;

   win.window = (void*) XtWindow(_area);
   kdDebug(1431) << "Window ID = " << win.window << endl;

   win_info.type = NP_SETWINDOW;
   win_info.display = XtDisplay( _area );
   win_info.visual = DefaultVisualOfScreen( XtScreen(_area) );
   win_info.colormap = DefaultColormapOfScreen( XtScreen(_area) );
   win_info.depth = DefaultDepthOfScreen( XtScreen(_area) );

   win.ws_info = &win_info;

   NPError error = NPSetWindow( &win );

   kdDebug(1431) << "<- NSPluginInstance::setWindow = " << error << endl;
   return error;
}


void NSPluginInstance::resizePlugin(int w, int h)
{
   kdDebug(1431) << "-> NSPluginInstance::resizePlugin( w=" << w << ", h=" << h << " ) " << endl;

   _width = w;
   _height = h;

   Arg args[5];
   Cardinal nargs=0;
   XtSetArg(args[nargs], XtNwidth, _width); nargs++;
   XtSetArg(args[nargs], XtNheight, _height); nargs++;

   XtSetValues(_form, args, nargs);
   XtSetValues(_area, args, nargs);

   setWindow();

   kdDebug(1431) << "<- NSPluginInstance::resizePlugin" << endl;
}


NPError NSPluginInstance::NPGetValue(NPPVariable variable, void *value)
{
   if (!_pluginFuncs.getvalue)
      return NPERR_GENERIC_ERROR;

   NPError error = _pluginFuncs.getvalue(_npp, variable, value);

   CHECK(GetValue,error);
}


NPError NSPluginInstance::NPSetValue(NPNVariable variable, void *value)
{
   if (!_pluginFuncs.setvalue)
      return NPERR_GENERIC_ERROR;

   NPError error = _pluginFuncs.setvalue(_npp, variable, value);

   CHECK(SetValue,error);
}


NPError NSPluginInstance::NPSetWindow(NPWindow *window)
{
   if (!_pluginFuncs.setwindow)
      return NPERR_GENERIC_ERROR;

   NPError error = _pluginFuncs.setwindow(_npp, window);

   CHECK(SetWindow,error);
}


NPError NSPluginInstance::NPDestroyStream(NPStream *stream, NPReason reason)
{
   if (!_pluginFuncs.destroystream)
      return NPERR_GENERIC_ERROR;

   NPError error = _pluginFuncs.destroystream(_npp, stream, reason);

   CHECK(DestroyStream,error);
}


NPError NSPluginInstance::NPNewStream(const NPMIMEType type, NPStream *stream, NPBool seekable, uint16 *stype)
{
   if (!_pluginFuncs.newstream)
      return NPERR_GENERIC_ERROR;

   NPError error = _pluginFuncs.newstream(_npp, type, stream, seekable, stype);

   CHECK(NewStream,error);
}


void NSPluginInstance::NPStreamAsFile(NPStream *stream, const char *fname)
{
   if (!_pluginFuncs.asfile)
      return;

   _pluginFuncs.asfile(_npp, stream, fname);
}


int32 NSPluginInstance::NPWrite(NPStream *stream, int32 offset, int32 len, void *buf)
{
   if (!_pluginFuncs.write)
      return 0;

   return _pluginFuncs.write(_npp, stream, offset, len, buf);
}


int32 NSPluginInstance::NPWriteReady(NPStream *stream)
{
   if (!_pluginFuncs.writeready)
      return 0;

   return _pluginFuncs.writeready(_npp, stream);
}


void NSPluginInstance::NPURLNotify(QString url, NPReason reason, void *notifyData)
{
   if (!_pluginFuncs.urlnotify)
      return;

   _pluginFuncs.urlnotify(_npp, url.ascii(), reason, notifyData);
}


void NSPluginInstance::addTempFile(KTempFile *tmpFile)
{
   _tempFiles.append(tmpFile);
}


void NSPluginInstance::setCallback(QCString app, QCString obj)
{
   delete _callback;
   _callback = new NSPluginCallbackIface_stub(app, obj);
}

/************************************************************************************/

NSPluginViewer::NSPluginViewer( QCString dcopId, QObject *parent )
   : QObject( parent ), DCOPObject(dcopId)
{
}


NSPluginViewer::~NSPluginViewer()
{
   kdDebug(1431) << "NSPluginViewer::~NSPluginViewer" << endl;
   // TODO: shutdown
}


void NSPluginViewer::Shutdown()
{
   kdDebug(1431) << "NSPluginViewer::Shutdown" << endl;
   // tell event loop to quit
   quitXt();
}


DCOPRef NSPluginViewer::NewClass( QString plugin )
{
   kdDebug(1431) << "NSPluginViewer::NewClass( " << plugin << ")" << endl;

   NSPluginClass *cls = _classes[ plugin ];
   if ( !cls )
   {
      cls = new NSPluginClass( plugin, this );
      _classes.insert( plugin, cls );
   }

   kdDebug(1431) << "NSPluginViewer::NewClass = " << (void*)cls << endl;

   return DCOPRef( kapp->dcopClient()->appId(), cls->objId() );
}


/****************************************************************************/


NSPluginClass::NSPluginClass( const QString &library, QObject *parent )
   : QObject( parent ), DCOPObject(), _libname(library), _constructed(false),  _initialized(false)
{
   _handle = KLibLoader::self()->library(library.latin1());

   kdDebug(1431) << "Library handle=" << _handle << endl;

   if (!_handle)
   {
      kdDebug(1431) << "Could not dlopen " << library << endl;
      return;
   }

   _NP_GetMIMEDescription = (NP_GetMIMEDescriptionUPP *)_handle->symbol("NP_GetMIMEDescription");
   _NP_Initialize = (NP_InitializeUPP *)_handle->symbol("NP_Initialize");
   _NP_Shutdown = (NP_ShutdownUPP *)_handle->symbol("NP_Shutdown");

   if (!_NP_GetMIMEDescription)
   {
      kdDebug(1431) << "Could not get symbol NP_GetMIMEDescription" << endl;
      return;
   }

   if (!_NP_Initialize)
   {
      kdDebug(1431) << "Could not get symbol NP_Initialize" << endl;
      return;
   }

   if (!_NP_Shutdown)
   {
      kdDebug(1431) << "Could not get symbol NP_Shutdown" << endl;
      return;
   }

   kdDebug(1431) << "Plugin library " << library << " loaded!" << endl;
   _constructed = true;

   Initialize();
}


NSPluginClass::~NSPluginClass()
{
   Shutdown();
   //KLibLoader::self()->unloadLibrary(_libname);
   delete _handle;
}



int NSPluginClass::Initialize()
{
   kdDebug(1431) << "NSPluginClass::Initialize()" << endl;

   if (!_constructed)
      return NPERR_GENERIC_ERROR;

   if (_initialized)
      kdError() << "FUNC ALREADY INITIALIZED!" << endl;

   memset(&_pluginFuncs, 0, sizeof(_pluginFuncs));
   memset(&_nsFuncs, 0, sizeof(_nsFuncs));

   _pluginFuncs.size = sizeof(_pluginFuncs);
   _nsFuncs.size = sizeof(_nsFuncs);
   _nsFuncs.version = (NP_VERSION_MAJOR << 8) + NP_VERSION_MINOR;
   _nsFuncs.geturl = g_NPN_GetURL;
   _nsFuncs.posturl = g_NPN_PostURL;
   _nsFuncs.requestread = g_NPN_RequestRead;
   _nsFuncs.newstream = g_NPN_NewStream;
   _nsFuncs.write = g_NPN_Write;
   _nsFuncs.destroystream = g_NPN_DestroyStream;
   _nsFuncs.status = g_NPN_Status;
   _nsFuncs.uagent = g_NPN_UserAgent;
   _nsFuncs.memalloc = g_NPN_MemAlloc;
   _nsFuncs.memfree = g_NPN_MemFree;
   _nsFuncs.memflush = g_NPN_MemFlush;
   _nsFuncs.reloadplugins = g_NPN_ReloadPlugins;
   _nsFuncs.getJavaEnv = g_NPN_GetJavaEnv;
   _nsFuncs.getJavaPeer = g_NPN_GetJavaPeer;
   _nsFuncs.geturlnotify = g_NPN_GetURLNotify;
   _nsFuncs.posturlnotify = g_NPN_PostURLNotify;
   _nsFuncs.getvalue = g_NPN_GetValue;
   _nsFuncs.setvalue = g_NPN_SetValue;
   _nsFuncs.invalidaterect = g_NPN_InvalidateRect;
   _nsFuncs.invalidateregion = g_NPN_InvalidateRegion;
   _nsFuncs.forceredraw = g_NPN_ForceRedraw;

   NPError error = _NP_Initialize(&_nsFuncs, &_pluginFuncs);

   if (error==NPERR_NO_ERROR)
      _initialized = true;

   CHECK(Initialize,error);
}


QString NSPluginClass::GetMIMEDescription()
{
   return _NP_GetMIMEDescription();
}


void NSPluginClass::Shutdown()
{
   _NP_Shutdown();
}


DCOPRef NSPluginClass::NewInstance(QString url, QString mimeType, bool embed,
                                   QStringList argn, QStringList argv)
{
   kdDebug(1431) << "-> NSPluginClass::NewInstance" << endl;

   // copy parameters over
   unsigned int argc = argn.count();
   char **_argn = new char*[argc];
   char **_argv = new char*[argc];
   QString src = url;
   int width = 300;
   int height = 300;

   for (unsigned int i=0; i<argc; i++)
   {
      const char *n = (const char*)argn[i].ascii();
      const char *v = (const char*)argv[i].ascii();

      _argn[i] = strdup(n);
      _argv[i] = strdup(v);

      if (!strcasecmp(_argn[i], "WIDTH")) width = atoi(_argv[i]);
      if (!strcasecmp(_argn[i], "HEIGHT")) height = atoi(_argv[i]);
      kdDebug(1431) << "argn=" << _argn[i] << " argv=" << _argv[i] << endl;
   }

   // Create plugin instance
   char mime[256];
   strcpy(mime, mimeType.ascii());
   NPP npp = new NPP_t;
   npp->ndata = NULL;
   NPError error = _pluginFuncs.newp(mime, npp, embed ? NP_EMBED : NP_FULL,
                                     argc, _argn, _argv, 0);
   kdDebug(1431) << "NPP_New = " << (int)error << endl;

   if (error != NPERR_NO_ERROR)
   {
      delete npp;
      kdDebug(1431) << "<- PlluginClass::NewInstance = 0" << endl;
      return DCOPRef();
   }

   // Create DCOP instance for created plugin
   NSPluginInstance *inst = new NSPluginInstance( npp, &_pluginFuncs, _handle,
                                                  width, height, src, mimeType, this );
   delete [] _argn;
   delete [] _argv;
   if (!inst)
   {
      kdDebug(1431) << "<- PlluginClass::NewInstance = 0" << endl;
      return DCOPRef();
   }

   kdDebug(1431) << "<- NSPluginClass::NewInstance = %x" << (void*)inst << endl;
   return DCOPRef(kapp->dcopClient()->appId(), inst->objId());
}


/****************************************************************************/

NSPluginStreamBase::NSPluginStreamBase( NSPluginInstance *instance )
   : QObject( instance ), _instance(instance), _stream(0), _tempFile(0L),
     _pos(0), _queue(0), _queuePos(0), _error(false)
{
}


NSPluginStreamBase::~NSPluginStreamBase()
{
   if (_stream)
   {
      _instance->NPDestroyStream( _stream, NPRES_USER_BREAK );
      delete _stream;
   }

   delete _tempFile;
   delete _queue;
}




bool NSPluginStreamBase::create( QString url, QString mimeType, void *notify )
{
    if ( _stream )
        return false;

    KURL src(url);
    _url = url;
    _notifyData = notify;
    _pos = 0;

    // create new stream
    _stream = new NPStream;
    _stream->ndata = this;
    _stream->url = strdup(url.ascii());
    _stream->end = 0;
    _stream->pdata = 0;
    _stream->lastmodified = 0;
    _stream->notifyData = _notifyData;

    // inform the plugin
    _instance->NPNewStream((char*)mimeType.ascii(), _stream, false, &_streamType);
    kdDebug(1431) << "NewStream stype=" << _streamType << " url=" << url << " mime=" << mimeType << endl;

    // prepare data transfer
    _tempFile = 0L;

    if ( _streamType==NP_ASFILE ) {
        _tempFile = new KTempFile( QString::null, src.fileName() );
        _tempFile->setAutoDelete( TRUE );
    }

    return true;
}


int NSPluginStreamBase::process( const QByteArray &data, int start )
{
   int32 max, sent, to_sent, len;
   char *d = data.data()+start;

   to_sent = data.size()-start;
   while (to_sent>0)
   {
      max = _instance->NPWriteReady( _stream );
      len = QMIN(max, to_sent);

      kdDebug(1431) << "-> Feeding stream to plugin: offset=" << _pos << ", len=" << len << endl;
      sent = _instance->NPWrite( _stream, _pos, len, d );
      kdDebug(1431) << "<- Feeding stream: sent = " << sent << endl;

      if (sent==0) // interrupt the stream for a few ms
          break;

      if ( sent<0 ) {
          // stream data rejected/error
          kdDebug(1431) << "stream data rejected/error" << endl;
          _error = true;
          break;
      }

      if (_tempFile)
          _tempFile->dataStream()->writeRawBytes( d, sent );

      to_sent -= sent;
      _pos += sent;
      d += sent;
   }

   return data.size()-to_sent;
}


bool NSPluginStreamBase::pump()
{
    kdDebug(1431) << "queue pos " << _queuePos << ", size " << _queue.size() << endl;

    if ( _queuePos<_queue.size() )
        _queuePos = process( _queue, _queuePos );

    return _queuePos>=_queue.size();
}


void NSPluginStreamBase::queue( const QByteArray &data )
{
    kdDebug(1431) << "new queue size " << data.size() << endl;
    kdDebug(1431) << "data=" << (void*)data.data() << " size=" << data.size() << endl;

    _queue = data;
    _queue.detach();
    _queuePos = 0;

    kdDebug(1431) << "queue=" << (void*)_queue.data() << " size=" << _queue.size() << endl;
}


void NSPluginStreamBase::finish( bool err )
{
    kdDebug(1431) << "finish error=" << err << endl;

    _queue.resize( 0 );
    _pos = 0;
    _queuePos = 0;

    if ( !err ) {
        if ( _tempFile ) {
            _tempFile->close();
            _instance->addTempFile( _tempFile );
            _instance->NPStreamAsFile( _stream, _tempFile->name().ascii() );
            _tempFile = 0;
        }

        _instance->NPDestroyStream( _stream, NPRES_DONE );
        if ( _notifyData )
            _instance->NPURLNotify( _url, NPRES_DONE, _notifyData );
    } else {
        // close temp file
        if ( _tempFile )
            _tempFile->close();

        // destroy stream
        _instance->NPDestroyStream( _stream, NPRES_NETWORK_ERR );
        if ( _notifyData )
            _instance->NPURLNotify( _url, NPRES_NETWORK_ERR, _notifyData );
    }

    // delete stream
    delete _stream;
    _stream = 0;

    // destroy NSPluginStream object
    emit finished( this );
}


/****************************************************************************/

NSPluginBufStream::NSPluginBufStream( class NSPluginInstance *instance )
    : NSPluginStreamBase( instance )
{
    _timer = new QTimer( this );
    connect( _timer, SIGNAL(timeout()), this, SLOT(timer()) );
}


NSPluginBufStream::~NSPluginBufStream()
{

}


bool NSPluginBufStream::get( QString url, QString mimeType, const QByteArray &buf,
                             void *notifyData)
{
    if ( !create( url, mimeType, notifyData ) )
        return false;

    queue( buf );
    _timer->start( 100, true );
}


void NSPluginBufStream::timer()
{
    if ( !pump() )
        _timer->start( 100, true );
    else
        finish( error() );
}



/****************************************************************************/

NSPluginStream::NSPluginStream( NSPluginInstance *instance )
    : NSPluginStreamBase( instance ), _job(0)
{
   _resumeTimer = new QTimer( this );
   connect(_resumeTimer, SIGNAL(timeout()), this, SLOT(resume()));
}


NSPluginStream::~NSPluginStream()
{
    if ( _job )
        _job->kill( true );
}


bool NSPluginStream::get( QString url, QString mimeType, void *notify )
{
    // create new stream
    if ( !create( url, mimeType, notify ) )
         return false;

    if ( _streamType==NP_ASFILEONLY ) {
        kdDebug(1431) << "stream " << url << " as NP_ASFILEONLY" << endl;
        // do an synchronous download
        QString tmpFile;
        if( KIO::NetAccess::download(url, tmpFile) ) {
            kdDebug(1431) << "downloaded " << url << " as " << tmpFile << endl;
            _instance->NPStreamAsFile( _stream, tmpFile.ascii() );
            // TODO: remove file later???
            finish( false );
            kdDebug(1431) << "finished immediately" << endl;
            return true;
        } else {
            kdDebug(1431) << "error downloaded " << url << endl;
            finish( true );
            return false;
        }
    } else {
        // start the kio job
        _job = KIO::get(url, false, false);
        connect(_job, SIGNAL(data(KIO::Job *, const QByteArray &)),
                SLOT(data(KIO::Job *, const QByteArray &)));
        connect(_job, SIGNAL(result(KIO::Job *)), SLOT(result(KIO::Job *)));
    }
}


void NSPluginStream::data(KIO::Job * job, const QByteArray &data)
{
    kdDebug(1431) << "NSPluginStream::data - job=" << (void*)job << " data size=" << data.size() << endl;
    queue( data );
    if ( !pump() ) {
        _job->suspend();
        _resumeTimer->start( 100, TRUE );
    }
}


void NSPluginStream::resume()
{
   if ( error() ) {
       _job->kill( true );
       finish( true );
       return;
   }

   if ( pump() ) {
      kdDebug(1431) << "resume job" << endl;
      _job->resume();
   } else {
       kdDebug(1431) << "restart timer" << endl;
       _resumeTimer->start( 100, TRUE );
   }
}


void NSPluginStream::result(KIO::Job *job)
{
   int err = job->error();
   _job = 0;
   finish( err!=0 || error() );
}
