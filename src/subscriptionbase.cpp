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

#include <qdebug.h>
#include "q_whiteboard_node.h"
#include "subscriptionbase.h"

SubscriptionBase::SubscriptionBase( QueryType type, QWhiteBoardNode *node)
  : update_sequence(0),
   m_queryType(type),
   m_node(node)
{
    //connect(m_node, SIGNAL(subscriptionFinished(int, int)), this, SLOT(subscriptionFinished(int)));
    //connect(m_node, SIGNAL(subscriptionIndication(int)), this, SLOT(subscriptionIndication()));
}

SubscriptionBase::~SubscriptionBase()
{
}

int SubscriptionBase::unsubscribe()
{
  int ret=-1;
  //QWhiteBoardNode *node = qobject_cast<QWhiteBoardNode *>(parent());
  ret = m_node->unsubscribe(this);
  return ret;
}

void SubscriptionBase::subscriptionFinished( int success )
{
  qDebug() << "SubscriptionBase::subscriptionFinished, id: " << m_id << " success: " << success;
  emit finished(success);
}

QWhiteBoardNode *SubscriptionBase::wbNode()
{
 //QWhiteBoardNode *node = qobject_cast<QWhiteBoardNode *>(parent());
 //Q_ASSERT_X(node != NULL, "SubscriptionBase:wbNode()", "parent() not QWhiteBoardNode");
// return node;
    return m_node;
}

#if 0
void SubscriptionBase::subscriptionFinished( int id, int success )
{
  qDebug() << "SubscriptionBase::subscriptionFinished, id: " << id;
  if( m_id == id)
    {
      emit finished(success);
    }
}


void SubscriptionBase::subscriptionFinished()
{
  emit finished(0);
}
#endif
void SubscriptionBase::subscriptionIndication( )
{
  qDebug() << "SubscriptionBase::subscriptionIndication, id: " << m_id;
  //  if( m_id == id)
  // {
      emit indication();
      //  }
}

QueryType SubscriptionBase::type() const
{
  return m_queryType;
}

int SubscriptionBase::id() const
{
  return m_id;
}

const QString &SubscriptionBase::subscriptionId() const
{
  return m_subscription_id;
}

void SubscriptionBase::setSubscriptionId(QString id)
{
  m_subscription_id = id;
}

void SubscriptionBase::subscribe( QList<Triple *> &/*templates*/, QHash<QString, QString> * /*nameSpace */)
{
  qDebug()<< "QueryBase::subscribe(QList<Triple *> &templates, QHash<QString, QString> *nameSpace) must be implemented in derived class!";
  emit finished(-1);
}

void SubscriptionBase::subscribe(  TripleElement /*startNode*/, TripleElement /*endNode*/, QString /*path*/ )
{
  qDebug()<< "QueryBase::subscribe( TripleElement startNode, TripleElement endNode, QString path) must be implemented in derived class!";
  emit finished(-1);
}


