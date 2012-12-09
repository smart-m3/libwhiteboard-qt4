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

#include "qdebug.h"
#include "q_whiteboard_node.h"
#include "templatesubscription.h"
#include "templatequeryhandler.h"

TemplateSubscription::TemplateSubscription( QWhiteBoardNode *node)
  :SubscriptionBase(QueryTypeTemplate, node)
{
  //connect(node, SIGNAL(subscriptionFinished(int, int)), this, SLOT(subscriptionFinished(int, int)));
}

TemplateSubscription::~TemplateSubscription()
{
  resetResults();
} 

void TemplateSubscription::subscribe(QList<Triple *> &templates, QHash<QString, QString> *name_space)
{
  int ret;
  //QWhiteBoardNode *node = static_cast<QWhiteBoardNode *>(parent());

  ret = m_node->subscribe( templates, name_space, this);
  if (ret > 0)
    {
      //connect(node, SIGNAL(subscriptionFinished(int)), this, SLOT(subscriptionFinished(int)));
      //connect(node, SIGNAL(subscriptionIndication(int)), this, SLOT(subscriptionIndication(int)));
      m_id = ret;
      //qDebug() << "m_id: " << m_id;
    }
  else 
    {
      emit finished(ret);
    }
}

int TemplateSubscription::parseResultsAdded( QString &results )
{
  int success = -1;
  QXmlSimpleReader parser;
  resetResultsAdded();
  
  TemplateQueryHandler *handler  = new TemplateQueryHandler( &m_resultsAdded );
  QXmlInputSource source;
  source.setData(results);
  //qDebug() << "TemplateSubscription::parseResultsAdded: " << results;
  parser.setContentHandler(handler);
  parser.setErrorHandler(handler);
  
  success = parser.parse(source);
  if(!success)
    {
      qDebug() << "Parse error: " << parser.errorHandler()->errorString();
      
    }
  else
    {
      mergeResultsAdded();
    }
  delete handler;
  return (success ? 0 : -1);
}

int TemplateSubscription::parseResultsObsolete( QString &results )
{
  int success = -1;
  QXmlSimpleReader parser;
  resetResultsObsolete();
  
  TemplateQueryHandler *handler  = new TemplateQueryHandler( &m_resultsObsolete );
  QXmlInputSource source;
  source.setData(results);
  //qDebug() << "TemplateSubscription::parseResultsObsolete: " << results;
  parser.setContentHandler(handler);
  parser.setErrorHandler(handler);
  
  success = parser.parse(source);
  if(!success)
    {
      qDebug() << "Parse error: " << parser.errorHandler()->errorString();
      
    }
  else
    {
      mergeResultsObsolete();
    }
  delete handler;
  return (success ? 0 : -1);
}

QList<Triple *> &TemplateSubscription::results()
{
  return m_results;
}

QList<Triple *> &TemplateSubscription::resultsAdded()
{
  return m_resultsAdded;
}

QList<Triple *> &TemplateSubscription::resultsObsolete()
{
  return m_resultsObsolete;
}

void TemplateSubscription::resetResults()
{
  while( !m_results.isEmpty())
    {
      Triple *triple = m_results.takeFirst();
      delete triple;
    }
}

void TemplateSubscription::resetResultsAdded()
{
  while( !m_resultsAdded.isEmpty())
    {
      Triple *triple = m_resultsAdded.takeFirst();
      delete triple;
    }
}

void TemplateSubscription::resetResultsObsolete()
{
  while( !m_resultsObsolete.isEmpty())
    {
      Triple *triple = m_resultsObsolete.takeFirst();
      delete triple;
    }
}

void TemplateSubscription::mergeResultsAdded()
{
  QList<Triple *>::iterator it = m_resultsAdded.begin();
  while( it != m_resultsAdded.end() )
    {
      Triple *addT = *it;
      Triple *t = new Triple( *addT );
      m_results.append(t);
      ++it;
    }
}

void TemplateSubscription::mergeResultsObsolete()
{
  QList<Triple *>::iterator it_obs = m_resultsObsolete.begin();
  while( it_obs != m_resultsObsolete.end() )
    {
      QList<Triple *>::iterator it = m_results.begin();
      Triple *remT = *it_obs;

      while( (it != m_results.end()) )
	{
	  Triple *t = *it;
	  if( *t == *remT)
	    {
	      delete t;
	      m_results.erase(it);
	      break;
	    }
	  ++it;
	}
      ++it_obs;
    }
}
