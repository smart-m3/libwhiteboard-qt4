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
#include "subscriptionsparqlbasebindings.h"
#include "querysparqlbasebindingshandler.h"

SubscriptionSparqlBaseBindings::SubscriptionSparqlBaseBindings(QueryType type, QWhiteBoardNode *node)
  :SubscriptionBase(type, node)
{
}

SubscriptionSparqlBaseBindings::~SubscriptionSparqlBaseBindings()
{
  resetResults();
} 


int SubscriptionSparqlBaseBindings::parseResultsAdded( QString &results )
{

  int success = -1;
  QXmlSimpleReader parser;
  resetResultsAdded();

  QuerySparqlBaseBindingsHandler *handler  = new QuerySparqlBaseBindingsHandler( &m_tripleElements,
										 &m_resultRows,
										 &m_selectedVariables,
										 &m_resultsAdded );
  QXmlInputSource source;
  source.setData(results);
  qDebug() << "SubscriptionSparqlBaseBindings::parseResultsAdded: " << results;
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

int SubscriptionSparqlBaseBindings::parseResultsObsolete( QString &results )
{

  int success = -1;
  QXmlSimpleReader parser;
  resetResultsObsolete();

  QuerySparqlBaseBindingsHandler *handler  = new QuerySparqlBaseBindingsHandler( &m_tripleElements,
										 &m_resultRows,
										 &m_selectedVariables,
										 &m_resultsObsolete );
  QXmlInputSource source;
  source.setData(results);
  qDebug() << "SubscriptionSparqlBaseBindings::parseResultsAdded: " << results;
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


QList<QString> &SubscriptionSparqlBaseBindings::selectedVariables()
{
  return m_selectedVariables;
}

QList<QList<TripleElement *>*> &SubscriptionSparqlBaseBindings::results()
{
  return m_results;
}


QList<QList<TripleElement *>*> &SubscriptionSparqlBaseBindings::resultsAdded()
{  
  return m_resultsAdded;
}

QList<QList<TripleElement *>*> &SubscriptionSparqlBaseBindings::resultsObsolete()
{
  return m_resultsObsolete;
}

void SubscriptionSparqlBaseBindings::resetResults()
{
  m_results.clear();
}

void SubscriptionSparqlBaseBindings::resetResultsObsolete()
{
  m_resultsObsolete.clear();
}

void SubscriptionSparqlBaseBindings::resetResultsAdded()
{
  m_resultsAdded.clear();
}

void SubscriptionSparqlBaseBindings::mergeResultsAdded()
{
  QList<QList<TripleElement *>*>::iterator it = m_resultsAdded.begin();
  while( it != m_resultsAdded.end() )
    {
#if 0 //fix this
      TripleElement el = (*it);
      m_results.append( el);
      ++it;
#else
      qDebug() << "************** FIX THIS";
#endif
    }
}

void SubscriptionSparqlBaseBindings::mergeResultsObsolete()
{
  QList<QList<TripleElement *>*>::iterator it_obs = m_resultsObsolete.begin();
  while( it_obs != m_resultsObsolete.end() )
    {
#if 0 //fix this
      QList<QList<TripleElement *>*>::iterator it = m_results.begin();
      TripleElement remT = *it_obs;

      while( (it != m_results.end()) )
	{
	  TripleElement t = *it;
	  if( t == remT)
	    {
	      m_results.erase(it);
	      break;
	    }
	  ++it;
	}
      ++it_obs;
#else
      qDebug() << "************** FIX THIS";
#endif
    }
}
