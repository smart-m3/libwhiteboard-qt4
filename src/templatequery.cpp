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
#include "templatequery.h"
#include "templatequeryhandler.h"

TemplateQuery::TemplateQuery( QWhiteBoardNode *node)
  :QueryBase(QueryTypeTemplate, node)
{
  //connect(m_node, SIGNAL(queryFinished(int, int)), this, SLOT(queryFinished(int, int)));
}

TemplateQuery::~TemplateQuery()
{
  resetResults();
} 

int TemplateQuery::parseResults( QString &results )
{
  int success = -1;
  QXmlSimpleReader parser;
  resetResults();
  TemplateQueryHandler *handler  = new TemplateQueryHandler( &m_results );
  QXmlInputSource source;
  source.setData(results);
  qDebug() << "TemplateQuery::parseResults: " << results;
  parser.setContentHandler(handler);
  parser.setErrorHandler(handler);
  
  success = parser.parse(source);
  if(!success)
    {
      qDebug() << "Parse error: " << parser.errorHandler()->errorString();
      
    }
  delete handler;
  return (success ? 0 : -1);
}

QList<Triple *> &TemplateQuery::results()
{
  return m_results;
}

QList<Triple *> TemplateQuery::takeResults()
{
    QList<Triple *> copyOfResults;
    copyOfResults.append(m_results);
    m_results.clear();
    return copyOfResults;
}

void TemplateQuery::resetResults()
{
  while( !m_results.isEmpty())
    {
      Triple *triple = m_results.takeFirst();
      delete triple;
    }
}

void TemplateQuery::query(QList<Triple *> &templates, QHash<QString, QString> *name_space )
{
  int ret;
  // TODO: check qobject_cast
  //QWhiteBoardNode *node = static_cast<QWhiteBoardNode *>(parent());
  
  ret = m_node->query( templates, name_space, this);
  if (ret <= 0)
    {
      emit finished(ret);
    }
  else
    {
      m_id = ret;
    }

}
