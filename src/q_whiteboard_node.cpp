/*
@section LICENSE

  Copyright (c) 2010, Nokia Corporation
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


#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusReply>
#include <QUuid>
#include <QDebug>
#include <QMutexLocker>
#include <QtXml/QDomDocument>
#include <QtXml/QDomElement>
#include <QtXml/QDomCDATASection>

#define DBUS_API_SUBJECT_TO_CHANGE

#include <dbus/dbus.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-lowlevel.h>
#include "q_whiteboard_node.h"
#include "triple.h"
#include "querybase.h"
#include "subscriptionbase.h"
#include "insertresponsehandler.h"

extern "C"
{
#include <whiteboard_dbus_ifaces.h>
#include <whiteboard_util.h>
#include <sibdefs.h>
}

/**
 * Callback function when a message is received in the D-Bus interface.
 *
 **/
static DBusHandlerResult node_dispatch_message(DBusConnection *conn,
					       DBusMessage *msg,
					       gpointer data);

/**
 * node_unregister_handler
 *
 **/
static void node_unregister_handler(DBusConnection *connection,
				    gpointer data);


/**
 * Constructor for QWhiteBoardNode.
 * @param parent Parent QObject. Default value NULL.
 */

QWhiteBoardNode::QWhiteBoardNode( QObject *parent)
  :QObject(parent),
   m_connection(NULL),
   m_joined(false),
   m_msgnum(0)

{
  m_uuid = QUuid::createUuid();
  if(g_registerTry()<0)
    {
      return;
    }
}

/**
 * Destructor
 *
 **/

QWhiteBoardNode::~QWhiteBoardNode()
{
  g_unregister();
}

/**
 *
 * Register with the whiteboardd. 
 * @return 0 upon success, -1 upon failure.
 *
 **/
int QWhiteBoardNode::g_registerTry()
{
  DBusMessage *reply = NULL;
  GError *gerror = NULL;
  DBusGConnection *gconnection = NULL;
  gchar *address = NULL;
  int retval = -1;
  QString uuid_str = m_uuid.toString();
  DBusObjectPathVTable vtable = { &node_unregister_handler,
				  &node_dispatch_message,
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
      gchar *uuidtmp = g_strdup(uuid_str.toAscii().constData());

      /* TODO: Switch back to whiteboard_util_send_method_with_reply,
	 when WhiteBoard actually sends something */
      whiteboard_util_send_method_with_reply(WHITEBOARD_DBUS_SERVICE,
					     WHITEBOARD_DBUS_OBJECT,
					     WHITEBOARD_DBUS_REGISTER_INTERFACE,
					     WHITEBOARD_DBUS_REGISTER_METHOD_NODE,
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
      qDebug() << "Discovery failed";
  }
  return retval;
  
}

/**
 *
 * Unregister with the whiteboardd.
 * 
 * @return 0 upon success, -1 upon failure.
 **/

int QWhiteBoardNode::g_unregister()
{
  QString uuid_str = m_uuid.toString();
  if( joined() )
    {
      leave();
    }
  gchar *uuidtmp = g_strdup(uuid_str.toAscii().constData());
  whiteboard_util_send_signal(WHITEBOARD_DBUS_OBJECT,
			      WHITEBOARD_DBUS_REGISTER_INTERFACE,
			      WHITEBOARD_DBUS_REGISTER_SIGNAL_UNREGISTER_NODE,
			      m_connection,
			      DBUS_TYPE_STRING, &uuidtmp,
			      WHITEBOARD_UTIL_LIST_END);
  
  dbus_connection_unref(m_connection);
  m_connection = NULL;
  g_free(uuidtmp);
  return 0;
}

#if 0
int QWhiteBoardNode::registerTry()
{
  QDBusInterface discovery_interface(WHITEBOARD_DBUS_SERVICE,
				     WHITEBOARD_DBUS_OBJECT,
				     WHITEBOARD_DBUS_INTERFACE);

  
  QDBusReply<QString> reply = discovery_interface.call( WHITEBOARD_DBUS_METHOD_DISCOVERY,
							uuid.toString() );
  
  if(reply.isValid())
    {
      address = reply.value();
      //qDebug() << "Discovery reply, address: " << address;
    }
  else
    {
      qDebug() << "Discovery failed. Maybe Whiteboard daemon not running. Error: " << reply.error().message();
      return 0;
    }
  
  QDBusInterface register_interface( WHITEBOARD_DBUS_SERVICE,
				     WHITEBOARD_DBUS_OBJECT,
				     WHITEBOARD_DBUS_REGISTER_INTERFACE);

  QDBusReply<int> reply2 = register_interface.call( WHITEBOARD_DBUS_REGISTER_METHOD_NODE,
						    uuid.toString() );
  
  if(reply2.error().isValid())
    {
      qDebug() << "Could not send register message:"
	       << address << ": " << reply2.error().message();
      return 0;
    }

  //qDebug() << "Node registered.";
  return 1;
  
}

int QWhiteBoardNode::unregister()
{
  int retval = 0;
  QDBusConnection conn = QDBusConnection::sessionBus();
  QDBusMessage msg = QDBusMessage::createSignal(WHITEBOARD_DBUS_OBJECT,
						WHITEBOARD_DBUS_INTERFACE,
						WHITEBOARD_DBUS_REGISTER_SIGNAL_UNREGISTER_NODE);
  QList<QVariant> arguments;
  arguments.append(QVariant(uuid.toString()));

  msg.setArguments(arguments);

  if( !conn.send(msg) )
    {
      qDebug() << "QWhiteBoardNode::unregister(): could not send unregister message";
      
    }
  else
    {
      //qDebug() << "QWhiteBoardNode::unregister(): unregister message sent";
      retval = 1;
    }
  return retval;
}
#endif

/**
 *
 * Is the QWhiteBoardNode joined with some SIB.
 * @return true if the node has joined successfully with some SIB, false otherwise.
 *
 **/

bool QWhiteBoardNode::joined()
{
  if(m_joined)
    return true;

  return false;
}

/**
 *
 * Is the QWhiteBoardNode joined with the @param sib.
 * @param sib SIB
 * @return true if the node has joined successfully with the sib @param sib, false otherwise.
 *
 **/

bool QWhiteBoardNode::joined(QString sib)
{
  if(m_joined && (m_sib.compare(sib)==0))
    return true;

  return false;
}

/**
 *
 * Leave SIB currently joined with.
 * @return 0 upon success, -1 upon failure.
 **/
int QWhiteBoardNode::leave()
{
  DBusMessage *reply = NULL;
  dbus_int32_t success = -1;
  
  if( !joined() )
  {
    return 0;
  }
  else if( !m_connection)
   {
      return -1;
    }
  else
    {
      gchar *uuidtmp = g_strdup(m_uuid.toString().toAscii().constData());
      int msgnum = ++m_msgnum;
      
      whiteboard_util_send_method_with_reply(WHITEBOARD_DBUS_SERVICE,
					     WHITEBOARD_DBUS_OBJECT,
					     WHITEBOARD_DBUS_NODE_INTERFACE,
					     WHITEBOARD_DBUS_NODE_METHOD_LEAVE,
					     m_connection,
					     &reply,
					     DBUS_TYPE_STRING, &uuidtmp,
					     DBUS_TYPE_INT32, &msgnum,
					     WHITEBOARD_UTIL_LIST_END);
      
      if(reply)
	{
	  if( !whiteboard_util_parse_message(reply,
					     DBUS_TYPE_INT32, &success,
					     WHITEBOARD_UTIL_LIST_END))
	    {
	      qDebug() << "Error while leaving (could not parse reply)";
	    }
	  dbus_message_unref(reply);
	}
      else
	{
	  qDebug() << "Error while leaving (no reply)";
	}
      g_free(uuidtmp);
      m_joined = false;
    }
  return (int)success;
}

/**
 *
 * Start Join transcation with a SIB. Connect to signal joinComplete(int) to get notification when join procedure is finished.
 *
 * @param sib SIB to join.
 * @return 0 upon success, -1 upon failure. 
 *
 **/
int QWhiteBoardNode::join( QString sib )
{
    QMutexLocker locker( &m_lock);

  dbus_int32_t join_id = -1;
  DBusMessage *reply = NULL;
  
  if(joined())
    {
      if(joined(sib))
	return 0;
      else
	return -1;
    }
  if(!m_connection)
    {
      return -1;
    }

  gchar *uuidtmp = g_strdup(m_uuid.toString().toAscii().constData());
  gchar *sibtmp = g_strdup(sib.toAscii().constData());

  int msgnum = ++m_msgnum;
  whiteboard_util_send_method_with_reply(WHITEBOARD_DBUS_SERVICE,
					 WHITEBOARD_DBUS_OBJECT,
					 WHITEBOARD_DBUS_NODE_INTERFACE,
					 WHITEBOARD_DBUS_NODE_METHOD_JOIN,
					 m_connection,
					 &reply,
					 DBUS_TYPE_STRING, &uuidtmp,
					 DBUS_TYPE_STRING, &sibtmp,
					 DBUS_TYPE_INT32, &msgnum,
					 WHITEBOARD_UTIL_LIST_END);
  
  if(reply)
    {
      if(!whiteboard_util_parse_message(reply,
					DBUS_TYPE_INT32, &join_id,
					WHITEBOARD_UTIL_LIST_END))
	{
	  qDebug() << "Join error (could not parse message)";
	}
      dbus_message_unref(reply);
    }
  else
    {
      qDebug() << "Join error (no reply)";
    }
  
  g_free(uuidtmp);
  g_free(sibtmp);

  if(join_id > 0)
    {
      m_sib = sib; 
    }
  qDebug() << "got join_id" << join_id;
  return (join_id > 0) ? 0 : -1;
}

/**
 * Insert graph defined by encoded string triples to the SIB currently joined. 
 * @param triples Encoded triples to insert.
 * @param encoding Encoding type (NOTE: only EncodingRDFM3 supported currently)
 * @return 0 upon success, -1 upon failure
 *
 **/
int QWhiteBoardNode::insert( QString triples, EncodingType encoding)
{
  return insert(triples, encoding, NULL);
}

/**
 * Insert graph defined by itriples to the SIB currently joined. 
 * @param itriples Serialized string containing insert graph.
 * @param encoding Encoding method of itriples string.
 * @param ilist List of Triple structures constituting insert graph. If triples contain bNode elements, they will be replaced with URIs reported by the SIB.
 * @return 0 upon success, -1 upon failure.
 **/

int QWhiteBoardNode::insert( QString triplesxml, EncodingType encoding, QList<Triple *> *triples )
{
  if(!joined() || !m_connection)
    return -1;

  gint success = -1;
  gint response_success = -1;
  gchar *response  = NULL;

  DBusMessage *reply=NULL;
  gchar *uuidtmp = g_strdup(m_uuid.toString().toAscii().constData());
  gchar *sibtmp = g_strdup(m_sib.toAscii().constData());
  gchar *triplestmp = g_strdup(triplesxml.toAscii().constData());
  int msgnum = ++m_msgnum;
  //qDebug() << "QWhiteBoardNode::insert:\n"
  //	   << "\t" << uuidtmp << " \n"
  //	   << "\t" << sibtmp << " \n"
  //	   << "\t" << msgnum << " \n"
  //	   << "\t" << encoding << " \n"
  //	   << "\t" << triplestmp << " \n";
  printf("\t tiplesttmp: %s\n", triplestmp);
  whiteboard_util_send_method_with_reply(WHITEBOARD_DBUS_SERVICE,
					 WHITEBOARD_DBUS_OBJECT,
					 WHITEBOARD_DBUS_NODE_INTERFACE,
					 WHITEBOARD_DBUS_NODE_METHOD_INSERT,
					 m_connection,
					 &reply,
					 DBUS_TYPE_STRING, &uuidtmp,
					 DBUS_TYPE_STRING, &sibtmp,
					 DBUS_TYPE_INT32, &msgnum,
					 DBUS_TYPE_INT32, &encoding,
					 DBUS_TYPE_STRING, &triplestmp,
					 WHITEBOARD_UTIL_LIST_END);
  if(reply)
    {
      if(whiteboard_util_parse_message(reply,
				       DBUS_TYPE_INT32, &response_success,
				       DBUS_TYPE_STRING, &response,
				       WHITEBOARD_UTIL_LIST_END))
	{
	  if( response_success == 0 )
	    {
	      if(encoding == EncodingM3XML )
		{
		  QXmlSimpleReader parser;
  
		  InsertResponseHandler *handler  = new InsertResponseHandler( triples );
		  QXmlInputSource source;
		  QString qresponse(response);
		  source.setData(qresponse);
		  //qDebug() << "Insert response: " << qresponse;
		  parser.setContentHandler(handler);
		  parser.setErrorHandler(handler);
  
		  if( parser.parse(source) )
		    {
		      success = 0;
		    }
		  else
		    {
		      success = -1;
		      qDebug() << "parsing of Insert response failed:";
		      qDebug() << parser.errorHandler()->errorString();
		    }
		  delete handler;
		}
	    }
	  else
	    {
	      qDebug() << "Insert failed";
	    }
	}
      else
	{
	  qDebug() << "Invalid insert response";
	  success = -1;
	}
      dbus_message_unref(reply);
      //ssBufDesc_free(&desc);
    }
  g_free(triplestmp);
  g_free(uuidtmp);
  g_free(sibtmp);
  return success;
}

/**
 * Insert graph defined by triples to the SIB currently joined. 
 * @param triples QList of pointers to Triple structures.
 * @param name_space QHash table with prefixes as keys and full URIs as values. 
 * @return 0 upon success, -1 upon failure.
 *
 **/
int QWhiteBoardNode::insert( QList<Triple *> &triples, QHash<QString, QString> *name_space )
{
  if(!joined())
    return -1;
  QString insertMsg = generateTripleListString(triples, name_space);
  
  return insert(insertMsg, EncodingM3XML, &triples );
}
/**
 * Remove graph defined by triples from the SIB currently joined. 
 * @param triples QList of pointers to Triple structures.
 * @param name_space QHash table with prefixes as keys and full URIs as values. Give NULL if no namespace is used.
 * @return 0 upon success, -1 upon failure.
 *
 **/
int QWhiteBoardNode::remove( QList<Triple *> &triples, QHash<QString, QString> *name_space )
{
  if(!joined())
    return -1;
  QString removeMsg = generateTripleListString(triples, name_space);
  
  return remove(removeMsg, EncodingM3XML);
}
/**
 * Remove graph defined by triples.
 * @param triples Encoded triples.
 * @param encoding Encoding type. NOTE: currently only EncodingRDFM3 supported.
 * @return 0 upon success, -1 upon failure.
 *
 **/
int QWhiteBoardNode::remove( QString triples, EncodingType encoding )
{
  if(!joined() || !m_connection)
    return -1;

  gint success = -1;
  gint response_success = -1;
  gchar *response  = NULL;

  DBusMessage *reply=NULL;
  gchar *uuidtmp = g_strdup(m_uuid.toString().toAscii().constData());
  gchar *sibtmp = g_strdup(m_sib.toAscii().constData());
  gchar *triplestmp = g_strdup(triples.toAscii().constData());
  int msgnum = ++m_msgnum;

  whiteboard_util_send_method_with_reply(WHITEBOARD_DBUS_SERVICE,
					 WHITEBOARD_DBUS_OBJECT,
					 WHITEBOARD_DBUS_NODE_INTERFACE,
					 WHITEBOARD_DBUS_NODE_METHOD_REMOVE,
					 m_connection,
					 &reply,
					 DBUS_TYPE_STRING, &uuidtmp,
					 DBUS_TYPE_STRING, &sibtmp,
					 DBUS_TYPE_INT32, &msgnum,
					 DBUS_TYPE_INT32, &encoding,
					 DBUS_TYPE_STRING, &triplestmp,
					 WHITEBOARD_UTIL_LIST_END);
  if(reply)
    {
      whiteboard_util_parse_message(reply,
				    DBUS_TYPE_INT32, &response_success,
				    DBUS_TYPE_STRING, &response,
				    WHITEBOARD_UTIL_LIST_END);
      if(NULL != response)
	{
	  
	  if(response_success != 0)
	    {
	      qDebug() << "Remove failed";
	      success = -1;
	    }
	  else
	    {
	      // TODO: real parsing of the response message
	      //qDebug() << "Remove response:" << response;
	      success = 0;
	    }
	}
      else
	{
	  qDebug() << "Invalid insert response";
	  success = -1;
	}
      dbus_message_unref(reply);
    } 
  else
    {
      qDebug() << "No remove response";
      success = -1;
    }
  //ssBufDesc_free(&desc);
  
  g_free(triplestmp);
  g_free(uuidtmp);
  g_free(sibtmp);

  return success;
}
/**
 * Insert graph defined by itriples and remove graph defined by rtriples to and from the SIB currently joined. 
 * @param itriples QList of pointers to Triple structures to insert.
 * @param rtriples QList of pointers to Triple structures to remove.
 * @param iname_space QHash table with prefixes as keys and full URIs as values used in insert graph. Give NULL if no namespace is used in insert graph.
 * @param rname_space QHash table with prefixes as keys and full URIs as values used in remove graph.  Give NULL if no namespace is used in remove graph.
 * @return 0 upon success, -1 upon failure.
 *
 **/
int QWhiteBoardNode::update(QList<Triple *> itriples, 
			    QList<Triple *> rtriples,
			    QHash<QString, QString> *iname_space,
			    QHash<QString, QString> *rname_space)
{
  if(!joined())
    return -1;

  QString insertMsg = generateTripleListString(itriples, iname_space);
  QString removeMsg = generateTripleListString(rtriples, rname_space);

  return update(insertMsg,removeMsg, EncodingM3XML, &itriples );
  
}
/**
 * Insert graph defined by itriples and remove graph defined by rtriples to and from the SIB currently joined. 
 * @param itriples Serialized string containing insert graph.
 * @param rtriples Serialized string containing remove graph.
 * @param encoding Encoding method of itriples and rtriples.
 * @param ilist List of Triple structures constituting insert graph. If triples contain bNode elements, they will be replaced with URIs reported by the SIB.
 * @return 0 upon success, -1 upon failure.
 *
 **/
int QWhiteBoardNode::update(QString itriples, 
			    QString rtriples,
			    EncodingType encoding,
			    QList<Triple *> *ilist)
{
  gint success = -1;
  gint response_success = -1;
  gchar *response  = NULL;

  DBusMessage *reply=NULL;

  if(!joined() || !m_connection)
    {
      return -1;
    }


  gchar *uuidtmp = g_strdup(m_uuid.toString().toAscii().constData());
  gchar *sibtmp = g_strdup(m_sib.toAscii().constData());
  gchar *itriplestmp = g_strdup(itriples.toAscii().constData());
  gchar *rtriplestmp = g_strdup(rtriples.toAscii().constData());
  int msgnum = ++m_msgnum;


  whiteboard_util_send_method_with_reply(WHITEBOARD_DBUS_SERVICE,
					 WHITEBOARD_DBUS_OBJECT,
					 WHITEBOARD_DBUS_NODE_INTERFACE,
					 WHITEBOARD_DBUS_NODE_METHOD_UPDATE,
					 m_connection,
					 &reply,
					 DBUS_TYPE_STRING, &uuidtmp,
					 DBUS_TYPE_STRING, &sibtmp,
					 DBUS_TYPE_INT32, &msgnum,
					 DBUS_TYPE_INT32, &encoding,
					 DBUS_TYPE_STRING, &itriplestmp,
					 DBUS_TYPE_STRING, &rtriplestmp,
					 WHITEBOARD_UTIL_LIST_END);
  if(reply)
    {
      whiteboard_util_parse_message(reply,
				    DBUS_TYPE_INT32, &response_success,
				    DBUS_TYPE_STRING, &response,
				    WHITEBOARD_UTIL_LIST_END);
      if(NULL != response)
	{
	  
	  if(response_success != 0)
	    {
	      qDebug() << "Update failed";
	      success = -1;
	    }
	  else
	    {
	      if(encoding == EncodingM3XML )
		{
		  QXmlSimpleReader parser;
  
		  InsertResponseHandler *handler  = new InsertResponseHandler( ilist );
		  QXmlInputSource source;
		  QString qresponse(response);
		  source.setData(qresponse);
		  //qDebug() << "Update response: " << qresponse;
		  parser.setContentHandler(handler);
		  parser.setErrorHandler(handler);
  
		  if( parser.parse(source) )
		    {
		      success = 0;
		    }
		  else
		    {
		      success = -1;
		      qDebug() << "parsing of Insert response failed";
		    }
		  delete handler;
		}
	    }
	}
      else
	{
	  qDebug() << "Invalid update response";
	  success = -1;
	}
      dbus_message_unref(reply);
    } 
  else
    {
      qDebug() << "No update response";
      success = -1;
    }
  //ssBufDesc_free(&desc);
  
  g_free(itriplestmp);
  g_free(rtriplestmp);
  g_free(uuidtmp);
  g_free(sibtmp);
  
  return success;
}

/**
 * Called when WHITEBOARD_DBUS_NODE_SIGNAL_JOIN_COMPLETE signal message has been received from D-Bus.
 * @param msgstatus ssStatus_t
 **/
void QWhiteBoardNode::joinCompleteRcvd( int  msgstatus)
{
 QMutexLocker locker( &m_lock);
  int status = -1;
  if(msgstatus != ss_StatusOK)
    {
      qDebug() << "Join failed, status was" << msgstatus;
    }
  else
    {
      status = 0;
      //qDebug() << "Join ok, status was" << msgstatus;
      m_joined = true;
    }
  emit joinComplete(status);
}
/**
 * Called when method call return has been received for WHITEBOARD_DBUS_NODE_METHOD_QUERY D-Bus method call.
 * @param access_id Access ID of the query request.
 * @param msgstatus ssStatus_t
 * @param results Query result string. Format according to query request.
 **/
void QWhiteBoardNode::queryResponseRcvd(int access_id, int msgstatus, gchar *results)
{
    qDebug() << "QWhiteBoardNode::queryResponseRcvd()";
  int ret = -1;
  if( access_id > 0) 
    {
       QMutexLocker locker(&m_lock);

#if 1 //1=original / 0=for testing
      if( (msgstatus == ss_StatusOK) && (NULL != results) )
	{
	  if( m_queries.contains(access_id) )
	    {
#else
      if( /************rb TEMP
(msgstatus == ss_StatusOK) && */(NULL != results) )
	{
	  if( m_queries.contains(access_id) )
	    {
//rb <======================================================= check this..
	      results = 
"<sparql_results>"

"  <head>"
"    <variable name=\"contact\"/>"
"    <variable name=\"name\"/>"
"  </head>"

"  <results>"

"    <result>" 
"      <binding name=\"contact\">"
"	<uri>1828791f-4050-4502-99d4-98a5918fg6ac</uri>"
"      </binding>"
"      <binding name=\"name\">"
"	<literal><![CDATA[Charlie Brown]]></literal>"
"      </binding>"
"    </result>"

"    <result> "
"      <binding name=\"contact\">"
"	<uri>1828791f-4050-4502-99d4-98a5918fg6ef</uri>"
"      </binding>"
"    </result>"

"    <result> "
"    </result>"

"    <result> "
"      <binding name=\"name\">"
"	<literal><![CDATA[George]]></literal>"
"      </binding>"
"    </result>"

"  </results>"
"</sparql_results>";
#endif
	      QString qstr = QString::fromUtf8(results);
	      ret = m_queries[access_id]->parseResults( qstr );
	      if( ret < 0)
		{
		  qDebug() << "Error while parsing results for query w/ access_id: " << access_id;
		}
	    }
	  else
	    {
	      qDebug() << "Could not find Query results w/ access_id: " << access_id;
	    }
	}
      else
	{
	  qDebug() << "Error with Query w/ access_id: " << access_id;
	}
      m_queries[access_id]->queryFinished( ret);
      m_queries.remove(access_id);
    }
  else
    qDebug() << "Error with Query, invalid access_id < 0";
	  
}
/**
 * Called when method call return has been received for WHITEBOARD_DBUS_NODE_METHOD_SUBSCRIBE D-Bus method call.
 * @param access_id Access ID of the subscribe request.
 * @param msgstatus ssStatus_t
 * @param subscription_id String defining the subsciption id.
 * @param results Query result string. Format according to query request.
 **/
void QWhiteBoardNode::subscribeResponseRcvd(int access_id, int msgstatus, gchar * subscription_id, gchar *results)
{
  int ret = -1;
  if( access_id > 0) 
    {
       QMutexLocker locker( &m_lock);
      if( (msgstatus == ss_StatusOK) && (NULL != results) )
	{
	  if( m_subscriptions.contains(access_id) )
	    {
	      QString id(subscription_id);
	      QString qstr = QString::fromUtf8(results);
	      m_subscriptions[access_id]->setSubscriptionId( id );
	      ret = m_subscriptions[access_id]->parseResultsAdded( qstr );
	      if( ret < 0)
		{
		  qDebug() << "Error while parsing results for subscriptionw/ access_id: " << access_id;
		}
	    }
	  else
	    {
	      qDebug() << "Could not find subscription results w/ access_id: " << access_id;
	    }
	}
      else
	{
	  qDebug() << "Error with subscription w/ access_id: " << access_id << " status: " << msgstatus;
	}
      if(ret < 0 )
	{
	  m_subscriptions[access_id]->subscriptionFinished( ret );
	  m_subscriptions.remove(access_id);
	}
      else
	{
	  SubscriptionBase *s =  m_subscriptions[access_id];
	  s->subscriptionIndication();
	}
    }
  else
    qDebug() << "Error with SubscriptionRsp, invalid access_id < 0";
	  
}

/**
 * Called when WHITEBOARD_DBUS_NODE_SIGNAL_SUBSCRIPTION_IND D-Bus signal message has been received.
 * @param access_id Access ID of the subscribe request.
 * @param update_sequence always 0 or 1,2..(SSAP_IND_WRAP_NUM-1),1,2..
 * @param subscription_id String defining the subsciption id.
 * @param results_added New items in the query results since the previus incication. Format according to query request.
 * @param results_removed Obsolete items in the query results since the previous indication. Format according to query request.
 **/
void QWhiteBoardNode::subscriptionIndRcvd(int access_id, int update_sequence, gchar * /*subscription_id*/, gchar *results_added, gchar *results_removed )
{
  int ret = -1;
  qDebug() << "subscriptionIndRcvd:";
  qDebug() << "access_id:" << access_id;
  qDebug() << "seqnum:" << update_sequence;
  qDebug() << "added:" << results_added;
  qDebug() << "obsolete:" << results_removed;
 QMutexLocker locker( &m_lock);
  if( access_id > 0 && m_subscriptions.contains(access_id) ) 
    {
      SubscriptionBase *sb = m_subscriptions[access_id];

      //verify sequence number, if used.  always 0 or 1,2..(SSAP_IND_WRAP_NUM-1),1,2..
      if (update_sequence==0
	  && sb->update_sequence==0) //sb->update_sequence initialized with 0
	sb->update_sequence = -1;//lock 0 as only acceptable update_sequence

      if (sb->update_sequence!=-1 && ++sb->update_sequence==SSAP_IND_WRAP_NUM)
	sb->update_sequence = 1;
      
      if ((update_sequence!=sb->update_sequence)
	  && !(update_sequence==0 && sb->update_sequence == -1))
	{
          qDebug() << "SequenseIndication Error, new sequence: " << update_sequence
                  << " old: " << sb->update_sequence;
	  ret = ss_IndicationSequenceError;//passed in callback and used to effect empty lists (no parsing)
	}
      else if((NULL != results_added)&&( NULL != results_removed) )
	{
	  QString qstr_added = QString::fromUtf8(results_added);
	  ret = m_subscriptions[access_id]->parseResultsAdded( qstr_added );
	  if( ret < 0)
	    {
	      qDebug() << "Error while parsing resultsAdded for subscriptionw/ access_id: " << access_id;
	    }
	  else
	    {
	      QString qstr_rem = QString::fromUtf8(results_removed);
	      ret = m_subscriptions[access_id]->parseResultsObsolete( qstr_rem );
	      if( ret < 0)
		{
		  qDebug() << "Error while parsing resultsObsolete for subscriptionw/ access_id: " << access_id;
		}
	    }
	}
      else
	{
	  qDebug() << "SubscriptionInd: Empty results w/ access_id: " << access_id;
	  ret = ss_OperationFailed;
	}

      if(ret != 0 )
	{
	  sb->subscriptionFinished(ret);
	  m_subscriptions.remove(access_id);
	}
      else
	{
	  sb->subscriptionIndication();
	}
    }
  else
  {
      qDebug() << "Error with SubscriptionInd, invalid access_id or subscription not found." << access_id;
   }
}


/**
 * Called when WHITEBOARD_DBUS_NODE_SIGNAL_UNSUBSCRIBE_COMPLETE D-Bus signal message has been received.
 * @param access_id Access ID of the unsubscribe request.
 * @param subscription_id String defining the subsciption id.
 * @param results_added New items in the query results since the previus incication. Format according to query request.
 * @param results_removed Obsolete items in the query results since the previous indication. Format according to query request.
 **/
void QWhiteBoardNode::unsubscribeCompleteRcvd(int access_id, int msgStatus, gchar * /*subscription_id*/)
{
  int ret = -1;
  if( access_id > 0) 
    {
       QMutexLocker locker( &m_lock);
      if( m_subscriptions.contains(access_id) )
	{
	  SubscriptionBase *s = m_subscriptions[access_id];
	  ret = (msgStatus == ss_StatusOK) ? 0 :-1;
	  //qDebug() << "emit subscriptionFinished for id:" << access_id << " msgStatus: " << msgStatus;
	  s->subscriptionFinished(ret);
	}
      else
	{
	  qDebug() << "unsubscribeComplete: no subscription w/ access_id: " << access_id;
	}
    }
  else
    qDebug() << "Error with unsubscribeCompleteRcvd, invalid access_id < 0";
	  
}

static DBusHandlerResult node_dispatch_message(DBusConnection * /*conn*/,
					       DBusMessage *msg,
					       gpointer data)
{
  QWhiteBoardNode *node = static_cast<QWhiteBoardNode *>(data);
  const gchar* interface = NULL;
  const gchar *member = NULL;
  DBusHandlerResult ret = DBUS_HANDLER_RESULT_HANDLED;
  gint type = 0;

  interface = dbus_message_get_interface(msg);
  member = dbus_message_get_member(msg);
  type = dbus_message_get_type(msg);
  //qDebug() << "node_dispatch_message";
  switch (type)
    {
    case DBUS_MESSAGE_TYPE_SIGNAL:
      if( !strcmp( member,  WHITEBOARD_DBUS_NODE_SIGNAL_JOIN_COMPLETE) )
	{
	  //qDebug() << "node_dispatch_message: Join complete";
	  dbus_int32_t access_id=0;
	  int msgstatus=-1;
	  if( whiteboard_util_parse_message(msg,
					    DBUS_TYPE_INT32, &access_id,
					    DBUS_TYPE_INT32, &msgstatus,
					    WHITEBOARD_UTIL_LIST_END))
	    {
	      node->joinCompleteRcvd( msgstatus);
	    }
	  else
	    {
	      qDebug() << "node_dispatch_message: Could not parse D-Bus JOIN_COMPLETE message";
	    }
	}
      else if( !strcmp( member, WHITEBOARD_DBUS_NODE_SIGNAL_SUBSCRIPTION_IND) )
	{
	  //qDebug() << "node_dispatch_message: Subscription ind";
	  dbus_int32_t access_id=0;
	  dbus_int32_t update_sequence=0;
	  gchar *results_added  = NULL;
	  gchar *results_removed  = NULL;
	  gchar *subscription_id = NULL;
	  if( whiteboard_util_parse_message(msg,
					    DBUS_TYPE_INT32, &access_id,
					    DBUS_TYPE_INT32, &update_sequence,
					    DBUS_TYPE_STRING, &subscription_id,
					    DBUS_TYPE_STRING, &results_added,
					    DBUS_TYPE_STRING, &results_removed,
					    WHITEBOARD_UTIL_LIST_END))
	    {
	      node->subscriptionIndRcvd(access_id, update_sequence, subscription_id, results_added, results_removed );
	    }
	  else
	    {
	      qDebug() << "node_dispatch_message: Could not parse D-Bus SUBSCRIPTION_IND message";
	    } 
	  
	}
      else if( !strcmp( member, WHITEBOARD_DBUS_NODE_SIGNAL_UNSUBSCRIBE_COMPLETE) )
	{

	  
	  int access_id = 0;
	  int msgstatus=-1;
	  //int msgstatus = -1;
	  gchar *subscription_id = NULL;
	  if( whiteboard_util_parse_message(msg,
					    DBUS_TYPE_INT32, &access_id,
					    DBUS_TYPE_INT32, &msgstatus,
					    DBUS_TYPE_STRING, &subscription_id,
					    WHITEBOARD_UTIL_LIST_END))
	    {
	      //qDebug() << "node_dispatch_message: Unsubscribe complete, access_id:" << access_id
	      //       << " status:" << msgstatus << "suscription_id: " << subscription_id;
	      node->unsubscribeCompleteRcvd(access_id, msgstatus, subscription_id);
	    }
	  else
	    {
	      qDebug() << "node_dispatch_message: Could not parse D-Bus UNSUBSCRIBE_COMPLETE message";
	    }
	}
      else  if( !strcmp( member,  WHITEBOARD_DBUS_NODE_METHOD_QUERY) )
	{
	  gint access_id = -1;
	  //msgStatus_t msgstatus = MSG_E_NSET;
	  int msgstatus = -1;
	  gchar *results = NULL;
	  //qDebug() << "Got QUERY method return.";
	  if(whiteboard_util_parse_message(msg,
					DBUS_TYPE_INT32, &access_id,
					DBUS_TYPE_INT32, &msgstatus,
					DBUS_TYPE_STRING, &results,
					   WHITEBOARD_UTIL_LIST_END))
	    {
	      node->queryResponseRcvd( access_id, msgstatus, results);
	    }
	  else
	    {
	      qDebug() << "node_dispatch_message: Could not parse D-Bus QUERY_RETURN message";
	    }
	  
	}
      else if( !strcmp( member,  WHITEBOARD_DBUS_NODE_METHOD_SUBSCRIBE) )
	{
      	  gint access_id = -1;
	  //msgStatus_t msgstatus = MSG_E_NSET;
	  int msgstatus = -1;
	  gchar *subscription_id = NULL;
	  gchar *results = NULL;
	  //qDebug() << "Got SUBSCRIBE method return.";
	  if( whiteboard_util_parse_message(msg,
					DBUS_TYPE_INT32, &access_id,
					DBUS_TYPE_INT32, &msgstatus,
					DBUS_TYPE_STRING, &subscription_id,
					DBUS_TYPE_STRING, &results,
					    WHITEBOARD_UTIL_LIST_END))
	    {
	      node->subscribeResponseRcvd( access_id, msgstatus, subscription_id, results);
	    }
	  else
	    {
	      qDebug() << "node_dispatch_message: Could not parse D-Bus SUBSCRIBE_RETURN message";
	    }
	}
      else
	{
	  qDebug() << "node_dispatch_message: Got message: " << QString(member) << " on interface: " << QString(interface);
	  ret = DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
	}
      break;
    case DBUS_MESSAGE_TYPE_METHOD_RETURN:
#if 0
      if( !strcmp( member,  WHITEBOARD_DBUS_NODE_METHOD_QUERY) )
	{
	  gint access_id = -1;
	  //msgStatus_t msgstatus = MSG_E_NSET;
	  int msgstatus = -1;
	  gchar *results = NULL;
	  //qDebug() << "Got QUERY method return.";
	  if(whiteboard_util_parse_message(msg,
					DBUS_TYPE_INT32, &access_id,
					DBUS_TYPE_INT32, &msgstatus,
					DBUS_TYPE_STRING, &results,
					   WHITEBOARD_UTIL_LIST_END))
	    {
	      node->queryResponseRcvd( access_id, msgstatus, results);
	    }
	  else
	    {
	      qDebug() << "node_dispatch_message: Could not parse D-Bus QUERY_RETURN message";
	    }
	  
	}
      else if( !strcmp( member,  WHITEBOARD_DBUS_NODE_METHOD_SUBSCRIBE) )
	{
      	  gint access_id = -1;
	  //msgStatus_t msgstatus = MSG_E_NSET;
	  int msgstatus = -1;
	  gchar *subscription_id = NULL;
	  gchar *results = NULL;
	  //qDebug() << "Got SUBSCRIBE method return.";
	  if( whiteboard_util_parse_message(msg,
					DBUS_TYPE_INT32, &access_id,
					DBUS_TYPE_INT32, &msgstatus,
					DBUS_TYPE_STRING, &subscription_id,
					DBUS_TYPE_STRING, &results,
					    WHITEBOARD_UTIL_LIST_END))
	    {
	      node->subscribeResponseRcvd( access_id, msgstatus, subscription_id, results);
	    }
	  else
	    {
	      qDebug() << "node_dispatch_message: Could not parse D-Bus SUBSCRIBE_RETURN message";
	    }
	}
      else
#endif
	{
	  qDebug() << "node_dispatch_message: Unknown DBUS method return message:" << QString(member);
	}
      break;
    default:
      qDebug() << "node_dispatch_message: Unknown DBUS message type:" << type;
    }
  return ret;
}

int QWhiteBoardNode::query(QList<Triple *> &templates, 
			   QHash<QString, QString> *name_space,
			   QueryBase *q)
{
  
  
  if( !joined() || !m_connection || (q->type() != QueryTypeTemplate) )
    {
      qDebug() << "Node " << m_uuid.toString() << " not joined or not connected to daemon, can not create query.";
      return -1;
    }
  else
    {
      QString triplelist = generateTripleListString(templates, name_space);
      
      return query(triplelist, q);     
    }
}

int QWhiteBoardNode::query( TripleElement startNode, QString pathExpression, QueryBase *q )
{

  if( !joined() || !m_connection| (q->type() != QueryTypeWQLValues))
    {
      qDebug() << "Node " << m_uuid.toString() << " not joined or not connected to daemon, can not create query.";
      return -1;
    }
  else
    {
      QString message = generateWQLValuesQueryString(startNode, pathExpression);
      qDebug() << "Query Message: " << message;
      return query(message,q);
    }
}

int QWhiteBoardNode::query( TripleElement startNode, TripleElement endNode, QString pathExpression, QueryBase *q)
{
  if( !joined() || !m_connection| (q->type() != QueryTypeWQLRelated))
    {
      qDebug() << "Node " << m_uuid.toString() << " not joined or not connected to daemon, can not create query.";
      return -1;
    }
  else
    {
      QString message = generateWQLRelatedQueryString(startNode, endNode, pathExpression);
      return query(message,q);
    }
}

int QWhiteBoardNode::query( TripleElement node, QueryBase *q)
{
  if( !joined() || !m_connection| (q->type() != QueryTypeWQLNodeTypes))
    {
      qDebug() << "Node " << m_uuid.toString() << " not joined or not connected to daemon, can not create query.";
      return -1;
    }
  else
    {
      QString message = generateWQLNodeTypesQueryString(node);
      return query(message,q);
    }
}

int QWhiteBoardNode::queryIsSubType( TripleElement subtype, TripleElement supertype, QueryBase *q)
{
  if( !joined() || !m_connection| (q->type() != QueryTypeWQLIsSubType))
    {
      qDebug() << "Node " << m_uuid.toString() << " not joined or not connected to daemon, can not create query.";
      return -1;
    }
  else
    {
      QString message = generateWQLIsSubTypeQueryString(subtype, supertype);
      return query(message,q);
    }
}

int QWhiteBoardNode::queryIsType( TripleElement node, TripleElement type, QueryBase *q)
{
  if( !joined() || !m_connection| (q->type() != QueryTypeWQLIsType))
    {
      qDebug() << "Node " << m_uuid.toString() << " not joined or not connected to daemon, can not create query.";
      return -1;
    }
  else
    {
      QString message = generateWQLIsTypeQueryString(node, type);
      return query(message,q);
    }
}

int QWhiteBoardNode::query( QList<TripleElement *> &select, QList<Triple *> &where, QList<QList<Triple *>*> *optionals, QHash<QString, QString> *name_space, QueryBase *q)
{
  if( !joined() || !m_connection || (q->type() != QueryTypeSPARQLSelect) )
    {
      qDebug() << "Node " << m_uuid.toString() << " not joined or not connected to daemon, can not create query.";
      return -1;
    }
  else
    {
      QString message = generateSPARQLSelectQueryString(select, where, optionals, name_space);
      qDebug() << "Query Message: " << message;
      return query(message, q);     
    }
}

int QWhiteBoardNode::queryForm(QString &queryForm,
			       QueryBase *q)
{
  //for now, the only user supplied query text string "form" supported is a SELECT query
  if( !joined() || !m_connection || (q->type() != QueryTypeSPARQLSelect) )
    {
      qDebug() << "Node " << m_uuid.toString() << " not joined or not connected to daemon, can not create query.";
      return -1;
    }
  else
    {
      QString message = generateSPARQLFormQueryString(queryForm);
      qDebug() << "Query Message: " << message;
      return query(message, q);     
    }
}

int QWhiteBoardNode::query( QString message, QueryBase *q )
{
  if( !joined() || !m_connection)
    {
      qDebug() << "Node " << m_uuid.toString() << " not joined or not connected to daemon, can not create query.";
     return -1;
    }
  else
    {
      DBusMessage *reply = NULL;
      gint access_id = -1;
      gint type;
      type = q->type();
#if 0
      switch(q->type())
	{
	case EQueryTypeTemplate:
	  type = QueryTypeTemplate;
	  break;
	case EQueryTypeWQLValues:
	  type = QueryTypeWQLValues;
	  break;
	case EQueryTypeWQLNodeTypes:
	  type = QueryTypeWQLNodeTypes;
	  break;
	case EQueryTypeWQLRelated:
	  type = QueryTypeWQLRelated;
	  break;
	case EQueryTypeWQLIsType:
	  type = QueryTypeWQLIsType;
	  break;
	case EQueryTypeWQLIsSubType:
	  type = QueryTypeWQLIsSubType;
	  break;
	case EQueryTypeSPARQL:
	  type = QueryTypeSPARQL;
	  break;
	default:
	  qDebug() << "Invalid QueryType";
	  return -1;
	}
#endif
      QMutexLocker locker(&m_lock);

      gchar *uuidtmp = g_strdup(m_uuid.toString().toAscii().constData());
      gchar *sibtmp = g_strdup(m_sib.toAscii().constData());
      gchar *query_message = g_strdup(message.toAscii().constData());
      gint msgnum = ++m_msgnum;

      whiteboard_util_send_method_with_reply(WHITEBOARD_DBUS_SERVICE,
					     WHITEBOARD_DBUS_OBJECT,
					     WHITEBOARD_DBUS_NODE_INTERFACE,
					     WHITEBOARD_DBUS_NODE_METHOD_QUERY,
					     m_connection,
					     &reply,
					     DBUS_TYPE_STRING, &uuidtmp,
					     DBUS_TYPE_STRING, &sibtmp,
					     DBUS_TYPE_INT32, &msgnum,
					     DBUS_TYPE_INT32, &type,
					     DBUS_TYPE_STRING, &query_message,
					     WHITEBOARD_UTIL_LIST_END);
      if(reply)
	{
	  whiteboard_util_parse_message(reply,
					DBUS_TYPE_INT32, &access_id,
					WHITEBOARD_UTIL_LIST_END);
	  if(access_id < 0)
	    {
	      qDebug() << "Could not create query.";
	    }
	  else
	    {
	      if( !m_queries.contains(access_id) )
		{
		  m_queries[access_id] = q;
		}
	      else
		{
		  qDebug() << "m_queries already contain item w/ key" << access_id;
		}
	    }
	  dbus_message_unref(reply);
	}
      //      ssBufDesc_free(&desc);
      g_free(sibtmp);
      g_free(uuidtmp);
      g_free(query_message);
      return access_id;
    }
}

int QWhiteBoardNode::subscribe(QList<Triple *> &templates, 
			       QHash<QString, QString> *name_space,
			       SubscriptionBase *q )
{
  if( !joined() || !m_connection || (q->type() != QueryTypeTemplate) )
    {
      qDebug() << "Node " << m_uuid.toString() << " not joined or not connected to daemon, can not create subscription.";
      return -1;
    }
  else
    {
      QString triplelist = generateTripleListString(templates, name_space);
      
      return subscribe(triplelist, q);     
    }
}

int QWhiteBoardNode::subscribe( TripleElement startNode,  QString pathExpression, SubscriptionBase *q )
{
  if( !joined() || !m_connection || (q->type() != QueryTypeWQLValues ) )
    {
      qDebug() << "Node " << m_uuid.toString() << " not joined or not connected to daemon, can not create subscription.";
      return -1;
    }
  else
    {
      QString query_string = generateWQLValuesQueryString(startNode, pathExpression);
      
      return subscribe(query_string, q);     
    }
}

int QWhiteBoardNode::subscribe( TripleElement startNode,  TripleElement endNode, QString pathExpression, SubscriptionBase *q )
{
  if( !joined() || !m_connection || (q->type() != QueryTypeWQLRelated ) )
    {
      qDebug() << "Node " << m_uuid.toString() << " not joined or not connected to daemon, can not create subscription.";
      return -1;
    }
  else
    {
      QString query_string = generateWQLRelatedQueryString(startNode, endNode, pathExpression);
      
      return subscribe(query_string, q);     
    }
}

int QWhiteBoardNode::subscribe( QList<TripleElement *> &select, QList<Triple *> &where, QList<QList<Triple *>*> *optionals, QHash<QString, QString> *name_space, SubscriptionBase *q )
{
  if( !joined() || !m_connection || (q->type() != QueryTypeSPARQLSelect ) )
    {
      qDebug() << "Node " << m_uuid.toString() << " not joined or not connected to daemon, can not create subscription.";
      return -1;
    }
  else
    {
      QString message = generateSPARQLSelectQueryString(select, where, optionals, name_space);
      
      return subscribe(message, q);     
    }
}

int QWhiteBoardNode::subscribeForm(QString &queryForm,
				   SubscriptionBase *q)
{
  //for now, the only user supplied query text string "form" supported is a SELECT query
  if( !joined() || !m_connection || (q->type() != QueryTypeSPARQLSelect) )
    {
      qDebug() << "Node " << m_uuid.toString() << " not joined or not connected to daemon, can not create query.";
      return -1;
    }
  else
    {
      QString message = generateSPARQLFormQueryString(queryForm);
      qDebug() << "Query Message: " << message;
      return subscribe(message, q);     
    }
}

int QWhiteBoardNode::subscribe( QString message, SubscriptionBase *q )
{
  gint access_id = -1;
  QueryType type;
  DBusMessage *reply=NULL;

  type = q->type();

  if( !joined() || !m_connection)
    {
      qDebug() << "Node " << m_uuid.toString() << " not joined or not connected to daemon, can not create subscription.";
    }
  else
    {
      //QMutexLocker locker( &m_lock);
      gchar *uuidtmp = g_strdup(m_uuid.toString().toAscii().constData());
      gchar *sibtmp = g_strdup(m_sib.toAscii().constData());
      int msgnum = ++m_msgnum;
      gchar *query_message = g_strdup(message.toAscii().constData());

      whiteboard_util_send_method_with_reply(WHITEBOARD_DBUS_SERVICE,
					     WHITEBOARD_DBUS_OBJECT,
					     WHITEBOARD_DBUS_NODE_INTERFACE,
					     WHITEBOARD_DBUS_NODE_METHOD_SUBSCRIBE,
					     m_connection,
					     &reply,
					     DBUS_TYPE_STRING, &uuidtmp,
					     DBUS_TYPE_STRING, &sibtmp,
					     DBUS_TYPE_INT32, &msgnum,
					     DBUS_TYPE_INT32, &type,
					     DBUS_TYPE_STRING, &query_message,
					     WHITEBOARD_UTIL_LIST_END);
      
      if(reply && whiteboard_util_parse_message(reply,
						DBUS_TYPE_INT32, &access_id,
						WHITEBOARD_UTIL_LIST_END))
	{
	  if(access_id < 0)
	    {
	      qDebug() << "Could not create subscription.";
	    }
	  else
	    {
	      if( !m_subscriptions.contains(access_id) )
		{
		  m_subscriptions[access_id] = q;
		}
	      else
		{
		  qDebug() << "m_subscriptions already contain item w/ key" << access_id;
		}
	    }
	  dbus_message_unref(reply);
	}
      g_free(uuidtmp);
      g_free(sibtmp);
      g_free(query_message);
    }
  return access_id;
}


int QWhiteBoardNode::unsubscribe(SubscriptionBase *q )
{
  QMutexLocker locker( &m_lock);
  gchar *subscriptionid = g_strdup( q->subscriptionId().toAscii().constData() );
  gchar *uuidtmp = g_strdup(m_uuid.toString().toAscii().constData());
  gchar *sibtmp = g_strdup(m_sib.toAscii().constData());
  int access_id = q->id();
  int msgnum = ++m_msgnum;

  if( !joined() || !m_connection)
    {
      qDebug() << "Node " << m_uuid.toString() << " not joined or not connected to daemon, can not unsubscription.";
      return -1;
    }
  else
    {
      //desc = ssBufDesc_new();
      //insert_message =  whiteboard_insert_new_request(triples);
      //g_return_val_if_fail( ss_StatusOK == addRDFXML_query_w_templates (desc,  templates), -1); 
      //subscribe_message = ssBufDesc_GetMessage(desc);
      
      whiteboard_util_send_signal(WHITEBOARD_DBUS_OBJECT,
				  WHITEBOARD_DBUS_NODE_INTERFACE,
				  WHITEBOARD_DBUS_NODE_SIGNAL_UNSUBSCRIBE,
				  m_connection,
				  DBUS_TYPE_INT32, &access_id,
				  DBUS_TYPE_STRING, &uuidtmp,
				  DBUS_TYPE_STRING, &sibtmp,
				  DBUS_TYPE_INT32, &msgnum,
				  DBUS_TYPE_STRING, &subscriptionid,
				  WHITEBOARD_UTIL_LIST_END);
      
    }
  g_free( subscriptionid );
  g_free( uuidtmp );
  g_free( sibtmp );
  return 0;
}

static void node_unregister_handler(DBusConnection */*connection*/,
                                    gpointer /*data*/)
{
  qDebug() << "unregister_handler";
}

QString QWhiteBoardNode::generateTripleListString(QList<Triple *> &triples, const QHash<QString,QString> *name_space)
{
  QList<Triple *>::iterator it;
  QDomDocument doc;
  QDomElement triplelist = doc.createElement("triple_list");
  if(name_space)
    {
      QHash<QString,QString>::const_iterator it;
      for(it = name_space->constBegin(); it != name_space->constEnd(); ++it)
	{
	  QString fullPrefix("xmlns:");
	  fullPrefix.append( it.key());
	  triplelist.setAttribute( fullPrefix, it.value());
	}
    }

  for(it = triples.begin(); it != triples.end(); ++it)
    {
      QDomElement triple = doc.createElement("triple");
  //  triplelist.setTe
      QDomElement subj = doc.createElement("subject");

      switch( (*it)->subject().type() )
	{
	case TripleElement::ElementTypeURI:
	  subj.setAttribute("type", "uri");
	  break;
	case  TripleElement::ElementTypeLiteral:
	  qDebug()<< "ElementType : Literal is illegal for subject";
	break;
	case  TripleElement::ElementTypebNode:
	  subj.setAttribute("type", "bnode");
	  break;

	}
      
      QDomText stxt = doc.createTextNode((*it)->subject().node() );
      //      stxt.appendData((*it)->subject());
      subj.appendChild(stxt);
      QDomElement pred = doc.createElement("predicate");
      QDomText ptxt = doc.createTextNode( (*it)->predicate().node() );
      pred.appendChild(ptxt);

      QDomElement obj = doc.createElement("object");
      switch( (*it)->object().type() )
	{
	case TripleElement::ElementTypeURI:
	  obj.setAttribute("type", "uri");
	  break;
	case  TripleElement::ElementTypeLiteral:
	  obj.setAttribute("type", "literal");
	  break;
	case  TripleElement::ElementTypebNode:
	  obj.setAttribute("type", "bnode");
	  break;
	}
      if( (*it)->object().type() == TripleElement::ElementTypeLiteral)
	{
	  if( !(*it)->object().node().isEmpty() )
	    {
	      QString illegal = "]]>";
	      QString legal = "]]]]><![CDATA[>";
	      QString literalvalue= (*it)->object().node();
	      literalvalue.replace(illegal, legal);
	      QDomCDATASection ocdata = doc.createCDATASection( literalvalue.toUtf8().constData() );
	      obj.appendChild(ocdata);
	    }
	}
      else
	{
	  QDomText octxt = doc.createTextNode(  (*it)->object().node() );
	  obj.appendChild(octxt);
	}
      triple.appendChild(subj);
      triple.appendChild(pred);
      triple.appendChild(obj);
      triplelist.appendChild(triple);
    }
  doc.appendChild(triplelist);

  return doc.toString();
}

QString QWhiteBoardNode::generateWQLValuesQueryString(TripleElement startNode, QString pathExpression)
{
  QDomDocument doc;
  //  QDomElement param = doc.createElement("parameter");
  //param.setAttribute("name","query");

  QDomElement wqlquery = doc.createElement("wql_query");
  QDomElement start = doc.createElement("node");
  start.setAttribute("name", "start");
  if( startNode.type() == TripleElement::ElementTypeLiteral)
    {
	  start.setAttribute("type", "literal");
	  if( !startNode.node().isEmpty() )
	    {
	      QString illegal = "]]>";
	      QString legal = "]]]]><![CDATA[>";
	      QString literalvalue= startNode.node();
	      QDomCDATASection ocdata = doc.createCDATASection( literalvalue.replace(illegal, legal).toUtf8().constData() );
	      start.appendChild(ocdata);
	    }
    }
  else if( startNode.type() == TripleElement::ElementTypeURI)
    {
      start.setAttribute("type", "uri");
      QDomText startnodetxt = doc.createTextNode(startNode.node());
      start.appendChild(startnodetxt);
    }

  QDomElement path = doc.createElement("path_expression");
  QDomText pathtxt = doc.createTextNode(pathExpression);
  path.appendChild(pathtxt);

  wqlquery.appendChild(start);
  wqlquery.appendChild(path);

  //param.appendChild(wqlquery);
  doc.appendChild(wqlquery);

  return doc.toString();
}

QString QWhiteBoardNode::generateWQLRelatedQueryString( TripleElement startNode, TripleElement endNode, QString pathExpression)
{
  QDomDocument doc;
  //QDomElement param = doc.createElement("parameter");
  //param.setAttribute("name","query");

   QDomElement wqlquery = doc.createElement("wql_query");
  QDomElement start = doc.createElement("node");
  start.setAttribute("name", "start");
  if( startNode.type() == TripleElement::ElementTypeLiteral)
    {
      start.setAttribute("type", "literal");
      if( !startNode.node().isEmpty() )
	{
	  QString illegal = "]]>";
	  QString legal = "]]]]><![CDATA[>";
	  QString literalvalue= startNode.node();
	  QDomCDATASection ocdata = doc.createCDATASection( literalvalue.replace(illegal, legal).toUtf8().constData() );
	  start.appendChild(ocdata);
	}
    }
  else if( startNode.type() == TripleElement::ElementTypeURI)
    {
    start.setAttribute("type", "uri");
    QDomText startnodetxt = doc.createTextNode(startNode.node());
    start.appendChild(startnodetxt);
    }

  QDomElement end = doc.createElement("node");
  end.setAttribute("name", "end");
  if( endNode.type() == TripleElement::ElementTypeLiteral)
    {
      end.setAttribute("type", "literal");
      if( !endNode.node().isEmpty() )
	{
	  QString illegal = "]]>";
	  QString legal = "]]]]><![CDATA[>";
	  QString literalvalue= endNode.node();
	  QDomCDATASection ocdata = doc.createCDATASection( literalvalue.replace(illegal, legal).toUtf8().constData() );
	  end.appendChild(ocdata);
	}
    }
  else if( endNode.type() == TripleElement::ElementTypeURI)
    {
      end.setAttribute("type", "uri");
      QDomText endnodetxt = doc.createTextNode(endNode.node());
      end.appendChild(endnodetxt);
    }

  QDomElement path = doc.createElement("path_expression");
  QDomText pathtxt = doc.createTextNode(pathExpression);
  path.appendChild(pathtxt);

  wqlquery.appendChild(start);
  wqlquery.appendChild(end);
  wqlquery.appendChild(path);

  //param.appendChild(wqlquery);
  doc.appendChild(wqlquery);

  return doc.toString();
}

QString QWhiteBoardNode::generateWQLNodeTypesQueryString( TripleElement node)
{
  QDomDocument doc;
  //  QDomElement param = doc.createElement("parameter");
  //param.setAttribute("name","query");

  QDomElement wqlquery = doc.createElement("wql_query");
  QDomElement subnode = doc.createElement("node");
  if( node.type() == TripleElement::ElementTypeLiteral)
    {
      subnode.setAttribute("type", "literal");
      if(!node.node().isEmpty() )
	{
	  QString illegal = "]]>";
	  QString legal = "]]]]><![CDATA[>";
	  QString literalvalue= node.node();
	  QDomCDATASection ocdata = doc.createCDATASection( literalvalue.replace(illegal, legal).toUtf8().constData() );
	  subnode.appendChild(ocdata);
	}
    }
  else if( node.type() == TripleElement::ElementTypeURI)
    {
      subnode.setAttribute("type", "uri");
      QDomText subtypetxt = doc.createTextNode(node.node());
      subnode.appendChild(subtypetxt);
    }
  wqlquery.appendChild(subnode);
  doc.appendChild(wqlquery);

  return doc.toString();
}

QString QWhiteBoardNode::generateWQLIsSubTypeQueryString( TripleElement subtype, TripleElement supertype)
{
  QDomDocument doc;
  //  QDomElement param = doc.createElement("parameter");
  //param.setAttribute("name","query");

  QDomElement wqlquery = doc.createElement("wql_query");
  QDomElement subnode = doc.createElement("node");
  subnode.setAttribute("name", "subtype");
  if( subtype.type() == TripleElement::ElementTypeLiteral)
    {
      subnode.setAttribute("type", "literal");
      if( !subtype.node().isEmpty() )
	{
	  QString illegal = "]]>";
	  QString legal = "]]]]><![CDATA[>";
	  QString literalvalue= subtype.node();
	  QDomCDATASection ocdata = doc.createCDATASection( literalvalue.replace(illegal, legal).toUtf8().constData() );
	  subnode.appendChild(ocdata);
	}
    }
  else if( subtype.type() == TripleElement::ElementTypeURI)
    {
    subnode.setAttribute("type", "uri");  
    QDomText subtypetxt = doc.createTextNode(subtype.node());
    subnode.appendChild(subtypetxt);
    }

  QDomElement supernode = doc.createElement("node");
  supernode.setAttribute("name", "supertype");

  if( supertype.type() == TripleElement::ElementTypeLiteral)
    {
      supernode.setAttribute("type", "literal");
      if( !supertype.node().isEmpty() )
	{
	  QString illegal = "]]>";
	  QString legal = "]]]]><![CDATA[>";
	  QString literalvalue= supertype.node();
	  QDomCDATASection ocdata = doc.createCDATASection( literalvalue.replace(illegal, legal).toUtf8().constData() );
	  supernode.appendChild(ocdata);
	}
    }
  else if( supertype.type() == TripleElement::ElementTypeURI)
    {
      supernode.setAttribute("type", "uri");
      QDomText supernodetxt = doc.createTextNode(supertype.node());
      supernode.appendChild(supernodetxt);
      
    }

  wqlquery.appendChild(subnode);
  wqlquery.appendChild(supernode);

  //param.appendChild(wqlquery);
  doc.appendChild(wqlquery);

  return doc.toString();
}

QString QWhiteBoardNode::generateWQLIsTypeQueryString( TripleElement node, TripleElement type)
{
  QDomDocument doc;
  //  QDomElement param = doc.createElement("parameter");
  //param.setAttribute("name","query");

  QDomElement wqlquery = doc.createElement("wql_query");
  QDomElement start = doc.createElement("node");
  // start.setAttribute("name", "start"); // Commented out to comply with SIB parser --jh 4.9.2009

  if( node.type() == TripleElement::ElementTypeLiteral)
    {
      start.setAttribute("type", "literal");
      if( !node.node().isEmpty() )
	{
	  QString illegal = "]]>";
	  QString legal = "]]]]><![CDATA[>";
	  QString literalvalue= node.node();
	  QDomCDATASection ocdata = doc.createCDATASection( literalvalue.replace(illegal, legal).toUtf8().constData() );
	  start.appendChild(ocdata);
	}
    }
  else if( node.type() == TripleElement::ElementTypeURI)
    {
      start.setAttribute("type", "uri");
      QDomText startnodetxt = doc.createTextNode(node.node());
      start.appendChild(startnodetxt);
    }
  QDomElement typeel = doc.createElement("node");
  typeel.setAttribute("name", "type");
  if( type.type() == TripleElement::ElementTypeLiteral)
    {
      typeel.setAttribute("type", "literal"); 
      if( !type.node().isEmpty() )
	{
	  QString illegal = "]]>";
	  QString legal = "]]]]><![CDATA[>";
	  QString literalvalue= type.node();
	  QDomCDATASection ocdata = doc.createCDATASection( literalvalue.replace(illegal, legal).toUtf8().constData() );
	  typeel.appendChild(ocdata);
	}
    }
  else if( type.type() == TripleElement::ElementTypeURI)
    {
      typeel.setAttribute("type", "uri");
      QDomText typenodetxt = doc.createTextNode(type.node());
      typeel.appendChild(typenodetxt);
    }

  wqlquery.appendChild(start);
  wqlquery.appendChild(typeel);

  //param.appendChild(wqlquery);
  doc.appendChild(wqlquery);

  return doc.toString();
}

QString QWhiteBoardNode::generateSPARQLFormQueryString(QString text)
{
  QDomDocument doc;
  QDomElement sparqlquery = doc.createElement("sparql_query");
  QDomCDATASection queryForm( doc.createCDATASection( text ) );

  sparqlquery.appendChild(queryForm);
  
  doc.appendChild(sparqlquery);

  return doc.toString();
}

QString QWhiteBoardNode::generateSPARQLSelectQueryString(QList<TripleElement *>&select, QList<Triple*>&where, QList<QList<Triple*>*>*optionals, QHash<QString, QString>*name_space)
{
  QString queryForm("\n");

  if(name_space)
    {
      QHash<QString,QString>::const_iterator it;
      for(it = name_space->constBegin(); it != name_space->constEnd(); ++it)
	{
	  queryForm.append("PREFIX " + it.key() + " <" + it.value() + ">\n" );
	}
    }

  queryForm.append("SELECT");
  QList<TripleElement *>::const_iterator selectIt;
  for(selectIt = select.constBegin(); selectIt != select.constEnd(); ++selectIt)
    if ((*selectIt)->type()==TripleElement::ElementTypebNode)
      queryForm.append(" ?"+(*selectIt)->node());

  queryForm.append("\n");

  queryForm.append("WHERE { ");
  QList<Triple *>::const_iterator whereIt;
  for(whereIt = where.constBegin(); whereIt != where.constEnd(); ++whereIt)
    for(int i = 0; i < 3; ++i)
      { 
	const TripleElement *te;
	switch(i) {
	case 0: te = &(*whereIt)->subject();
	  break;
	case 1: te = &(*whereIt)->predicate();
	  break;
	case 2: te = &(*whereIt)->object();
	}

	if (te->type()==TripleElement::ElementTypebNode)
	  queryForm.append(" ?"+te->node());
	else if (i==2 && te->type()==TripleElement::ElementTypeLiteral)
	  queryForm.append(" \""+te->node()+"\"");
	else if (te->type()==TripleElement::ElementTypeURI)
	  {
	    QStringList seg = te->node().split(":");
	    if ((*name_space)[seg.at(0)+":"].isEmpty())
	      queryForm.append(" <"+te->node()+">");
	    else
	      queryForm.append(" "+te->node());
	  }
	else
	  return QString("*** Error: subject or predicate set literal");

	if(i==2)
	  queryForm.append(" .\n");
      }

  if (optionals) for (int n=0; n < optionals->size(); ++n)
    {
      QList<Triple *>*optional = optionals->at(n);
      queryForm.append("  OPTIONAL {");
      QList<Triple *>::const_iterator optionalIt;
      for(optionalIt = optional->constBegin(); optionalIt != optional->constEnd(); ++optionalIt)
	for(int i = 0; i < 3; ++i)
	  { 
	    const TripleElement *te;
	    switch(i) {
	    case 0: te = &(*optionalIt)->subject();
	      queryForm.append(" ");
	      break;
	    case 1: te = &(*optionalIt)->predicate();
	      break;
	    case 2: te = &(*optionalIt)->object();
	    }

	    if (te->type()==TripleElement::ElementTypebNode)
	      queryForm.append(" ?"+te->node());
	    else if (i==2 && te->type()==TripleElement::ElementTypeLiteral)
	      queryForm.append(" \""+te->node()+"\"");
	    else if (te->type()==TripleElement::ElementTypeURI)
	      {
		QStringList seg = te->node().split(":");
		if ((*name_space)[seg.at(0)+":"].isEmpty())
		  queryForm.append(" <"+te->node()+">");
		else
		  queryForm.append(" "+te->node());
	      }
	    else
	      return QString("*** Error: subject or predicate set literal");

	    if(i==2)
	      queryForm.append(" .\n");
	  }
    
      queryForm.append("  }\n");
    }

  queryForm.append("}\n");

  return generateSPARQLFormQueryString( queryForm );
}
