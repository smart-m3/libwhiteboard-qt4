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
/* -*- C++ -*-
 *
 */

#ifndef TRIPLE_H
#define TRIPLE_H

#include <qstring.h>

class TripleElement
{
public:  

  enum ElementType { ElementTypeURI, ElementTypeLiteral, ElementTypebNode };

  TripleElement();
  TripleElement( const TripleElement &aClass);
  TripleElement( const QString &node, ElementType type=ElementTypeURI);

  TripleElement &operator=(const TripleElement &aClass);
  bool operator==(const TripleElement &el2);
		    
  ~TripleElement();

  bool isEmpty() const;

  const QString node() const;
  bool setNode(const QString &node, ElementType type);
  ElementType type() const;

  void print() const;

protected:
  ElementType m_type;
  QString m_node;

};

class Triple
{
  
public:

  Triple();
  Triple( const TripleElement &subj, const TripleElement &pred, const TripleElement &obj);
  Triple ( const Triple &aClass);

  Triple &operator=(const Triple &aClass);
  bool operator==(const Triple &t2);

  ~Triple();

public:
  
  bool isComplete() const;
  
  const TripleElement &subject() const;
  const TripleElement &predicate() const;
  const TripleElement &object() const;

  bool setSubject(const TripleElement &subj);
  bool setPredicate(const TripleElement &pred);
  bool setObject(const TripleElement &obj);
  bool bnode2uri( QString tag, QString uri);
  bool noBnodes() const;
  void print();
protected:

private:

  
  // members
  TripleElement m_subject;
  TripleElement m_predicate;
  TripleElement m_object;

};

#endif
