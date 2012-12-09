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

#ifndef SUBSCRIPTIONBASE_H
#define SUBSCRIPTIONBASE_H
#include <qobject.h>
#include <qstring.h>
#include <qhash.h>
#include <QPointer>
#include "triple.h"
extern "C" {
#include <sibdefs.h>
}

class QWhiteBoardNode;

class SubscriptionBase : public QObject
{
  Q_OBJECT
public:

  
  SubscriptionBase(QueryType type, QWhiteBoardNode *parent);
  ~SubscriptionBase();

  virtual int parseResultsAdded( QString &results ) = 0;
  virtual int parseResultsObsolete( QString &results ) = 0;

  QueryType type() const;
  int id() const;
  const QString &subscriptionId() const;
  void setSubscriptionId(QString id);
  QWhiteBoardNode *wbNode();
  

  virtual void subscribe( QList<Triple *> &templates, QHash<QString, QString> *nameSpace = NULL );
  virtual void subscribe( TripleElement startNode, TripleElement endNode, QString path);

  int unsubscribe();

  int update_sequence;

 public slots:  
  //void subscriptionFinished( int queryid, int status );
  virtual void subscriptionFinished( int status );
   void subscriptionIndication( );
 signals:
  void indication();
  void finished(int status);

protected:

  QueryType m_queryType;
  int m_id; 
  QString m_subscription_id;
  QPointer<QWhiteBoardNode> m_node;
private:
  // Do not use
  SubscriptionBase();
  

};

#endif
