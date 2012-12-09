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

#ifndef SUBSCRIPTIONSPARQLBASEBINDINGS_H
#define SUBSCRIPTIONSPARQLBASEBINDINGS_H
#include <qobject.h>
#include <qlist.h>
//#include "triple.h"
#include "subscriptionbase.h"

//class Triple;
class QWhiteBoardNode;

class SubscriptionSparqlBaseBindings : public SubscriptionBase
{
 Q_OBJECT

   public:

  SubscriptionSparqlBaseBindings(QueryType type, QWhiteBoardNode *node);
  virtual ~SubscriptionSparqlBaseBindings();

 public:
  
  int parseResultsAdded( QString &results );
  int parseResultsObsolete( QString &results );

  QList<QString> &selectedVariables();
  QList<QList<TripleElement *>*> &results();
  QList<QList<TripleElement *>*> &resultsAdded();
  QList<QList<TripleElement *>*> &resultsObsolete();

protected:
  void resetResults();
  void resetResultsObsolete();
  void resetResultsAdded();

  void mergeResultsObsolete();
  void mergeResultsAdded();
private:

  QList<TripleElement> m_tripleElements;
  QList<QList<TripleElement *> > m_resultRows;

  QList<QString> m_selectedVariables;
  QList<QList<TripleElement *>*> m_results;
  QList<QList<TripleElement *>*> m_resultsAdded;
  QList<QList<TripleElement *>*> m_resultsObsolete;
};

#endif
