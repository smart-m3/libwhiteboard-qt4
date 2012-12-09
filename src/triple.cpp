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
#include "triple.h"


//TripleElement class implementation

TripleElement::TripleElement()
  :m_type(ElementTypeURI)
{
    // @Todo: Change ElementType in TripleElement default constructor
    //        from ElementTypeURI to ElementTypeBNode
}

TripleElement::TripleElement(const QString &node, ElementType type)
  :m_type(type)

  ,m_node(node)
{
}

TripleElement::TripleElement( const TripleElement &aClass )
{
  if(this != &aClass)
    {
      this->m_type = aClass.m_type;
      this->m_node = aClass.m_node;
    }
}

TripleElement &
TripleElement::operator=(const TripleElement &aClass)
{
  if( this != &aClass)
    {
      this->m_type = aClass.m_type;
      this->m_node = aClass.m_node;
    }
  return *this;
}

bool
TripleElement::operator==(const TripleElement &el2)
{
  bool ret = true;
  if( this != &el2)
    {
      ret = (( this->m_type == el2.m_type ) && ( this->m_node == el2.m_node));
    }
  return ret;
}

TripleElement::~TripleElement()
{
}


bool TripleElement::setNode(const QString &node, ElementType type)
{
  bool success = false;
  if(isEmpty())
  {
     m_node=node;
     m_type = type;
     success = true;
  }
  return success;
}

bool TripleElement::isEmpty() const
{
  return m_node.isEmpty();
}

const QString TripleElement::node() const
{
  //if( m_node.isEmpty() )
  // qDebug() << "Warning,  TripleElement::node(): m_node is empty";

  return m_node;
}

TripleElement::ElementType TripleElement::type() const
{
  return m_type;
}

void TripleElement::print() const
{
   switch( m_type )
     {
     case ElementTypeURI:
        qDebug() << "\t"<< m_node << "\t(URI)";
       break;
     case ElementTypeLiteral:
        qDebug() << "\t"<< m_node << "\t(literal)";
       break;
     case TripleElement::ElementTypebNode:
        qDebug() << "\t"<< m_node << "\t(bNode)";
       break;
     }
}
// Triple class implementation
Triple::Triple()
{
}

// Triple class implementation
Triple::Triple(  const Triple &aClass )
{
  if(this != &aClass)
    {
      this->m_subject = aClass.m_subject;
      this->m_predicate = aClass.m_predicate;
      this->m_object = aClass.m_object;
    }
}

Triple &
Triple::operator=(const Triple &aClass)
{
  if( this != &aClass)
    {
      this->m_subject = aClass.m_subject;
      this->m_predicate = aClass.m_predicate;
      this->m_object = aClass.m_object;
    }
  return *this;
}



Triple::Triple(const TripleElement &subj, const TripleElement &pred, const TripleElement &obj)
  :m_subject(subj),
   m_predicate(pred),
   m_object(obj)   
{
}

Triple::~Triple()
{
}
  

bool
Triple::operator==( const Triple &t2)
{
  bool ret = true;
  if( this != &t2)
    {
      ret = (( this->m_subject == t2.m_subject ) && 
	     ( this->m_predicate == t2.m_predicate) &&
	     ( this->m_object == t2.m_object) );
    }
  return ret;
}

const TripleElement &Triple::subject() const
{
  return m_subject;
}

const TripleElement &Triple::predicate() const
{  
  return m_predicate;
}

const TripleElement &Triple::object() const
{ 
  return m_object;
}


bool Triple::isComplete() const
{
  return ( !m_subject.isEmpty() && !m_predicate.isEmpty() && 
	    ( (m_object.type() == TripleElement::ElementTypeLiteral) || !m_object.isEmpty() ) ); 
}

bool Triple::setSubject( const TripleElement &subj)
{
  bool ret = false;
  if(m_subject.isEmpty())
    {
      m_subject = subj;
      ret = true;
    }
  return ret;
}

bool Triple::setPredicate(const TripleElement &pred)
{
  bool ret = false;
  if(m_predicate.isEmpty())
    {
      m_predicate = pred;
      ret = true;
    }
  return ret;
}

bool Triple::setObject( const TripleElement &obj)
{
  bool ret = false;
  if(m_object.isEmpty())
    {
      m_object = obj;
      ret = true;
    }
  return ret;
}

bool Triple::bnode2uri( QString tag, QString uri)
{
  qDebug() << "Triple::bnode2urit( " << tag <<", " << uri << ")";
  bool found = false;
  if( !m_subject.isEmpty() && 
      (m_subject.type() == TripleElement::ElementTypebNode) && 
      (m_subject.node().compare(tag)== 0))
    {
      TripleElement el(uri);
      qDebug() << "replacing bnode subj" << m_subject.node();
      m_subject = el;
      qDebug() << "with uri " << m_subject.node();
      found = true;

    }

  if( !m_object.isEmpty() && 
      (m_object.type() == TripleElement::ElementTypebNode) && 
      (m_object.node().compare(tag)== 0))
    {
      TripleElement el(uri);
      qDebug() << "replacing bnode obj" << m_object.node();
      m_object = el;
      qDebug() << "with uri " << m_object.node();
      found = true;
    }
  return found;
}

bool Triple::noBnodes() const
{
  return !( ( !m_object.isEmpty() && (m_object.type() == TripleElement::ElementTypebNode) ) || 
	    ( !m_subject.isEmpty()  && (m_subject.type() == TripleElement::ElementTypebNode) ) );
}

void Triple::print()
{
    qDebug() << "\nSubject:";
    m_subject.print();

    qDebug() << "\nPredicate:";
    m_predicate.print();

    qDebug() << "\nObject:";
    m_object.print();
    qDebug() << "\n";
}

