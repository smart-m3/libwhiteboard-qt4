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

#include <qdebug.h>
#include "q_whiteboard_node.h"
#include "querybase.h"

QueryBase::QueryBase( QueryType type, QWhiteBoardNode *node)
  : QObject(),
   m_queryType(type),
   m_node(node)
{
    //connect(m_node, SIGNAL(queryFinished(int, int)), this, SLOT(queryFinished(int, int)));
}

QueryBase::~QueryBase()
{
}

void QueryBase::queryFinished(int success )
{
    qDebug() << "QueryBase::queryFinished";
    emit finished(success);
}

QWhiteBoardNode *QueryBase::wbNode()
{
// QWhiteBoardNode *node = qobject_cast<QWhiteBoardNode *>(parent());
 //Q_ASSERT_X(node != NULL, "QueryBase:wbNode()", "parent() not QWhiteBoardNode");
 //return node;
    return m_node;
}

QueryType QueryBase::type() const
{
  return m_queryType;
}


void QueryBase::query(QList<Triple *> & /*templates*/, QHash<QString, QString> * /*name_space*/)
{
  // not implemented in base class
  qDebug()<< "QueryBase::query(QList<Triple *> &templates, QString> *name_space) must be implemented in derived class!";
  emit finished(-1);
}

void QueryBase::query(TripleElement /*startNode*/, QString /*pathExpression*/ )
{
  qDebug()<< "QueryBase::query(QString startNode, QString pathExpression) must be implemented in derived class!";
  emit finished(-1);
}

void QueryBase::query( TripleElement /*startNode*/, TripleElement /*endNode*/, QString /*pathExpression*/)
{
  qDebug()<< "QueryBase::query(QString startNode, QString endNode, QString pathExpression) must be implemented in derived class!";
  emit finished(-1);
}

void QueryBase::query(TripleElement /*node*/ )
{
  qDebug()<< "QueryBase::query(QString node) must be implemented in derived class!";
  emit finished(-1);
}

void QueryBase::queryIsType( TripleElement /*node*/, TripleElement /*type*/)
{
  qDebug()<< "QueryBase::queryIsType( TripleElement node, TripleElement type) must be implemented in derived class!";
  emit finished(-1);
}

void QueryBase::queryIsSubType( TripleElement /* subtype */, TripleElement /*supertype*/)
{
  qDebug()<< "QueryBase::queryIsSubType( TripleElement subtype, TripleElement supertype) must be implemented in derived class!";
  emit finished(-1);
}

void QueryBase::query(QString sparqlExpression )
{
  qDebug()<< "QueryBase::query(QString sparqlExpression) must be implemented in derived class!";
  emit finished(-1);
}

