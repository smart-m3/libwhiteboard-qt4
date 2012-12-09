/*
@section LICENSE

  Copyright (c) 2009, Nokia Corporation
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:
  
    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.  
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in
    the documentation and/or other materials provided with the
    distribution.  
    * Neither the name of Nokia nor the names of its contributors 
    may be used to endorse or promote products derived from this 
    software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
  COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
  OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 */

#include <QDebug>
#include <QMutexLocker>

extern "C" {
#include <whiteboard_dbus_ifaces.h>
#include <whiteboard_util.h>
}

#include "q_sib_discovery.h"
#include "q_sib_info.h"

static DBusHandlerResult discovery_dispatch_message(DBusConnection *conn,
					  DBusMessage *msg,
					  gpointer data);
static void discovery_unregister_handler(DBusConnection *connection,
						     gpointer user_data);
QSibDiscovery::QSibDiscovery(QObject *parent)
  :QObject(parent),
  m_connection(0),
   sibs()
{
  m_uuid = QUuid::createUuid();
   if(g_registerTry()<0)
     {
       qDebug() << "Could not register SibDiscovery";
     }
}

QSibDiscovery::~QSibDiscovery()
{
    if(m_connection)
        dbus_connection_unref(m_connection);

  QList<QString> keys = sibs.keys();
  while( !sibs.empty() )
    {
      delete sibs.take(keys[0]);
      keys.pop_front();
    }
}

void QSibDiscovery::addSib(QString uri, QString name)
{
  if( !sibs.contains(uri) )
    {
      QSibInfo *si = new QSibInfo(uri, name);
      sibs.insert(uri,si);
    }
}

void QSibDiscovery::removeSib(QString uri)
{
  if(sibs.contains(uri))
    {
      QSibInfo *si = sibs.take(uri);
      delete si;
    }
}

void QSibDiscovery::handleSibInserted(QString uri, QString name)
{
  addSib( uri, name);
  const QSibInfo *newsib = getSib(uri);
  if(newsib)
    emit sibInserted( newsib );
}

void QSibDiscovery::handleSibRemoved(QString uri)
{
  removeSib( uri );
  qDebug() << "emitting sibRemoved(" << uri << ")";
  emit sibRemoved( uri );
}

const QSibInfo *QSibDiscovery::getSib(QString uri) const
{
  if( sibs.contains(uri) )
    {
      return (const QSibInfo *)sibs[uri];
    }
  else
    {
      return NULL;
    }
}

void QSibDiscovery::allForNow()
{
  //x  qDebug() << "QSibDiscovery::allForNow()";

  emit sibList(sibs.values());
}

void QSibDiscovery::getSibs()
{


  if(m_connection && dbus_connection_get_is_connected(m_connection) )
    {
      whiteboard_util_send_method(WHITEBOARD_DBUS_SERVICE,
				  WHITEBOARD_DBUS_OBJECT,
				  WHITEBOARD_DBUS_DISCOVERY_INTERFACE,
				  WHITEBOARD_DBUS_DISCOVERY_METHOD_GET_SIBS,
				  m_connection,
				  WHITEBOARD_UTIL_LIST_END);
    }
  
  
}

int QSibDiscovery::g_registerTry()
{
  QMutexLocker locker(&m_lock);
  DBusMessage *reply = NULL;
  GError *gerror = NULL;

  DBusGConnection *gconnection = NULL;
  gchar *address = NULL;
  int retval = -1;
  QString uuid_str = m_uuid.toString();
  DBusObjectPathVTable vtable = { &discovery_unregister_handler,
				  &discovery_dispatch_message,
				  NULL, NULL, NULL, NULL };
  dbus_int32_t register_success;


  /* Discovering private address */
  if (whiteboard_util_discover(&address) == FALSE)
    {
      qDebug() << "Discovery failed, using default address\n";
      address = g_strdup("unix:path=/tmp/dbus-test");
    }
  if(address)
  {
  /* Open a connection to the address */
  gconnection = dbus_g_connection_open(address, &gerror);
  if (gerror != NULL)
    {
      qDebug() << "Unable to open connection: %s\n"
	       << gerror->message;
      g_error_free(gerror);

      m_connection = NULL;

      retval = -1;
    }
  else
    {
      /* Get the actual connection pointer */
      m_connection = (DBusConnection*)
	dbus_g_connection_get_connection(gconnection);

      /* Registering this UI */
      //qDebug() << "Sending node registration request\n";
      gchar *uuidtmp = g_strdup( uuid_str.toAscii().constData() );

      /* TODO: Switch back to whiteboard_util_send_method_with_reply,
	 when WhiteBoard actually sends something */
      whiteboard_util_send_method_with_reply(WHITEBOARD_DBUS_SERVICE,
					     WHITEBOARD_DBUS_OBJECT,
					     WHITEBOARD_DBUS_REGISTER_INTERFACE,
					     WHITEBOARD_DBUS_REGISTER_METHOD_DISCOVERY,
					     m_connection,
					     &reply,
					     DBUS_TYPE_STRING,
					     &uuidtmp,
					     WHITEBOARD_UTIL_LIST_END);
      
      //      dbus_connection_add_filter(m_connection, 
      //				 &dispatch_message, this, NULL);
      if(reply==NULL)
	{
	  qDebug() << "Could not register node (no reply)";
	}
      else
	{
	  
	  if( !whiteboard_util_parse_message(reply, 
					     DBUS_TYPE_INT32, &register_success,
					     WHITEBOARD_UTIL_LIST_END) )
	    {
	      qDebug() << "Could not register node (reply message parse error)";
	    }
	  else 
	    {
	      if(register_success < 0)
		{
		  qDebug() << "Could not register node (error in reply message)";
		}
	      else
		{
		  //qDebug() << "Node registered";
		  retval = 0;
		}
	      dbus_message_unref(reply);
	    }
	  
	  if(retval==0)
	    {
	      dbus_connection_register_fallback(m_connection, 
						WHITEBOARD_DBUS_OBJECT, &vtable,
						this);
	    }

			   
	   dbus_connection_setup_with_g_main(m_connection,
					     g_main_context_default());
	  retval = 0;
	}
      g_free(uuidtmp);
    }

  g_free(address);
}
  else
  {
      retval = -1;
      qDebug() << "Discovery failed\n";
  }
  return retval;
  
}

static DBusHandlerResult discovery_dispatch_message(DBusConnection */*conn*/,
					  DBusMessage *msg,
					  gpointer data)
{
  const gchar* interface = NULL;
  const gchar *member = NULL;
  QSibDiscovery *sd = static_cast<QSibDiscovery *>(data);
  DBusHandlerResult ret = DBUS_HANDLER_RESULT_HANDLED;
  interface = dbus_message_get_interface(msg);
  member = dbus_message_get_member(msg);
  if( !strcmp( member, WHITEBOARD_DBUS_DISCOVERY_SIGNAL_SIB) )
    {
      //qDebug() << "SIB signal";
      gchar *uri = NULL;
      gchar *name = NULL;
      if(whiteboard_util_parse_message(msg,
				    DBUS_TYPE_STRING, &uri,
				    DBUS_TYPE_STRING, &name,
				       WHITEBOARD_UTIL_LIST_END))
	{
	  if(uri && name )
	    {
	      QString quri(uri);
	      QString qname(name);
	      sd->addSib( quri, qname);
	    }
	}
      else
	{
	  qDebug() << "Error while parsing Dbus Message:" << member;
	}
    }
  else if( !strcmp( member, WHITEBOARD_DBUS_DISCOVERY_SIGNAL_ALL_FOR_NOW) )
    {
      //qDebug() << "All for now signal";
      sd->allForNow();
    }
  else if( !strcmp( member, WHITEBOARD_DBUS_DISCOVERY_SIGNAL_SIB_INSERTED) )
    {
      //qDebug() << "SIB inserted signal";
      gchar *uri = NULL;
      gchar *name = NULL;
      if(whiteboard_util_parse_message(msg,
				    DBUS_TYPE_STRING, &uri,
				    DBUS_TYPE_STRING, &name,
				       WHITEBOARD_UTIL_LIST_END))
	{
	  QString quri(uri);
	  QString qname(name);
	  sd->handleSibInserted( quri, qname);
	}
      else
	{
	  qDebug() << "Error while parsing Dbus Message:" << member;
	}
    }
  else if( !strcmp( member, WHITEBOARD_DBUS_DISCOVERY_SIGNAL_SIB_REMOVED) )
    {
      //qDebug() << "SIB removed signal";
      gchar *uri = NULL;
      gchar *name = NULL;
      if(whiteboard_util_parse_message(msg,
				    DBUS_TYPE_STRING, &uri,
				    DBUS_TYPE_STRING, &name,
				       WHITEBOARD_UTIL_LIST_END))
	{
	  QString quri(uri);
	  sd->handleSibRemoved( quri);
	}
      else
	{
	  qDebug() << "Error while parsing Dbus Message:" << member;
	}
    }
  else
    {
      qDebug() << "Got message: " << QString(member) << " on interface: " << QString(interface);
      ret = DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    }
  return ret;
}

static void discovery_unregister_handler(DBusConnection */*connection*/,
                               gpointer /*user_data*/)
{
  qDebug() << "Got unregister";
}
