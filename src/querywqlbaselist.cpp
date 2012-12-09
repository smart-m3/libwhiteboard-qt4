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
#include "querywqlbaselist.h"
#include "querywqlbaselisthandler.h"

QueryWqlBaseList::QueryWqlBaseList(QueryType type, QWhiteBoardNode *node)
  :QueryBase(type, node)
{
 // connect(node, SIGNAL(queryFinished(int, int)), this, SLOT(queryFinished(int, int)));
}

QueryWqlBaseList::~QueryWqlBaseList()
{
  resetResults();
} 


int QueryWqlBaseList::parseResults( QString &results )
{

  int success = -1;
  QXmlSimpleReader parser;
  resetResults();

  QueryWqlBaseListHandler *handler  = new QueryWqlBaseListHandler( &m_results );
  QXmlInputSource source;
  source.setData(results);
  qDebug() << "QueryWqlBaseList::parseResults: " << results;
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

QList<TripleElement> &QueryWqlBaseList::results()
{
  return m_results;
}

void QueryWqlBaseList::resetResults()
{
  m_results.clear();
}
