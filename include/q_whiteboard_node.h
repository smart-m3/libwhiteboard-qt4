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

/* -*- C++ -*-
 *
 */

#ifndef Q_WHITEBOARD_NODE_H
#define Q_WHITEBOARD_NODE_H

#include <QObject>
#include <QString>
#include <QUuid>
#include <QHash>
#include <QMutex>
#include <dbus/dbus.h>

extern "C" {
#include <sibmsg.h>
#include <sibdefs.h>
}
//#include "querybase.h"
#include "triple.h"


class QueryBase;
class SubscriptionBase;


/**
 *
 * @brief QWhiteBoardNode is the information access API for a selected SIB. 
 * 
 * QWhiteBoardNode 
 *
 **/

class QWhiteBoardNode : public QObject
{
Q_OBJECT

public:
  
  QWhiteBoardNode( QObject *parent = 0);
  ~QWhiteBoardNode();

public:
  
  int join( QString sib);
  int leave();
  int insert( QString triples, EncodingType encoding);
  int insert( QList<Triple *> &triples, QHash<QString, QString> *name_space=NULL);

  int remove( QString triples, EncodingType encoding );
  int remove( QList<Triple *> &triples, QHash<QString, QString> *name_space=NULL);

  int update(QString itriples, QString rtriples, EncodingType encoding);

  int update(QList<Triple *> itriples, 
	     QList<Triple *> rtriples, 
	     QHash<QString, QString> *iname_space=NULL,
	     QHash<QString, QString> *rname_space=NULL);

  /* template query */
  int query( QList<Triple *> &templates, 
	     QHash<QString, QString> *name_space, QueryBase *q);

  /* WQL-Values query */
  int query( TripleElement startNode, QString pathExpression, QueryBase *q);

  /* WQL-Related query */
  int query( TripleElement startNode, TripleElement endNode, QString pathExpression, QueryBase *q);

  /* WQL-NodeTypes query */
  int query( TripleElement startNode, QueryBase *q);

  /* WQL-IsType query */
  int queryIsType( TripleElement node, TripleElement type, QueryBase *q);

  /* WQL-IsSubType query */
  int queryIsSubType( TripleElement subtype, TripleElement supertype, QueryBase *q);

  /* SPARQL-Select query & subscribe */
  int query( QList<TripleElement *> &select, QList<Triple *> &where, QList<QList<Triple *>*> *optional, QHash<QString, QString> *name_space, QueryBase *q );
  int subscribe( QList<TripleElement *> &select, QList<Triple *> &where, QList<QList<Triple *>*> *optional, QHash<QString, QString> *name_space, SubscriptionBase *s );

  /* SPARQL-Select query & subscribe using txt for query input*/
  int queryForm( QString &queryExpression, QueryBase *q );
  int subscribeForm( QString &queryExpression, SubscriptionBase *s );

  int subscribe( QList<Triple *> &templates, QHash<QString, QString> *name_space, SubscriptionBase *s );
  int subscribe( TripleElement startNode, QString pathExpression, SubscriptionBase *s );
  int subscribe( TripleElement startNode, TripleElement endNode, QString pathExpression, SubscriptionBase *s );

  int unsubscribe(SubscriptionBase *s );

  void joinCompleteRcvd( int status);  
  void queryResponseRcvd(int access_id, int msgstatus, gchar *results);
  void subscribeResponseRcvd(int access_id, int msgstatus, gchar *subscription_id, gchar *results);
  void subscriptionIndRcvd(int access_id, int update_sequence, gchar *subscription_id, gchar *results_added, gchar *results_removed);
  void unsubscribeCompleteRcvd(int access_id, int msgStatus, gchar * subscription_id);

  static QString generateTripleListString(QList<Triple *> &list, const QHash<QString, QString> *name_space=NULL);
  //  static QString generateQueryTripleList(QList<Triple *> &triples);
public slots:
  static QString generateWQLValuesQueryString( TripleElement startNode, QString pathExpression);
  static QString generateWQLRelatedQueryString( TripleElement startNode, TripleElement endNode, QString pathExpression);
  static QString generateWQLIsTypeQueryString(TripleElement node, TripleElement type);
  static QString generateWQLNodeTypesQueryString(TripleElement node);
  static QString generateWQLIsSubTypeQueryString( TripleElement subtype, TripleElement supertype);
  static QString generateSPARQLSelectQueryString( QList<TripleElement *> &select, QList<Triple *> &where, QList<QList<Triple *>*> *optional=NULL, QHash<QString, QString> *name_space=NULL );
  static QString generateSPARQLFormQueryString ( QString queryExpression );

signals:
  
  void joinComplete(int status);  
  void queryFinished(int query_id, int success);

  //void subscriptionFinished(int query_id, int success);
  //void subscriptionFinished(int success);
  //void subscriptionIndication(int query_id );

protected:
  int insert( QString triplesxml, EncodingType encoding, QList<Triple *> *triples);
  int update( QString itriplesxml, QString rtriplesxml, EncodingType encoding, QList<Triple *> *ilist);

  int g_registerTry();
  int g_unregister();

  int query( QString message, QueryBase *q);
  int subscribe( QString message, SubscriptionBase *q );

  bool joined();
  bool joined(QString sib);
private:

  
  // members
  QString m_name;
  QString m_sib;
  QUuid m_uuid;
  DBusConnection *m_connection;
  bool m_joined;
  int m_msgnum;

  QHash<int, QueryBase *> m_queries;
  QHash<int, SubscriptionBase *> m_subscriptions;

  QMutex m_lock;
};

#endif
