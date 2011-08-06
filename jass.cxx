// Copyright (C) 2005-2010 Code Synthesis Tools CC
//
// This program was generated by CodeSynthesis XSD, an XML Schema to
// C++ data binding compiler.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
//
// In addition, as a special exception, Code Synthesis Tools CC gives
// permission to link this program with the Xerces-C++ library (or with
// modified versions of Xerces-C++ that use the same license as Xerces-C++),
// and distribute linked combinations including the two. You must obey
// the GNU General Public License version 2 in all respects for all of
// the code used other than Xerces-C++. If you modify this copy of the
// program, you may extend this exception to your version of the program,
// but you are not obligated to do so. If you do not wish to do so, delete
// this exception statement from your version.
//
// Furthermore, Code Synthesis Tools CC makes a special exception for
// the Free/Libre and Open Source Software (FLOSS) which is described
// in the accompanying FLOSSE file.
//

// Begin prologue.
//
//
// End prologue.

#include <xsd/cxx/pre.hxx>

#include "jass.hxx"

namespace Jass
{
  // Generator
  // 

  const Generator::Sample_type& Generator::
  Sample () const
  {
    return this->Sample_.get ();
  }

  Generator::Sample_type& Generator::
  Sample ()
  {
    return this->Sample_.get ();
  }

  void Generator::
  Sample (const Sample_type& x)
  {
    this->Sample_.set (x);
  }

  void Generator::
  Sample (::std::auto_ptr< Sample_type > x)
  {
    this->Sample_.set (x);
  }

  const Generator::Channel_type& Generator::
  Channel () const
  {
    return this->Channel_.get ();
  }

  Generator::Channel_type& Generator::
  Channel ()
  {
    return this->Channel_.get ();
  }

  void Generator::
  Channel (const Channel_type& x)
  {
    this->Channel_.set (x);
  }


  // Jass
  // 

  const Jass::Generator_sequence& Jass::
  Generator () const
  {
    return this->Generator_;
  }

  Jass::Generator_sequence& Jass::
  Generator ()
  {
    return this->Generator_;
  }

  void Jass::
  Generator (const Generator_sequence& s)
  {
    this->Generator_ = s;
  }
}

#include <xsd/cxx/xml/dom/parsing-source.hxx>

namespace Jass
{
  // Generator
  //

  Generator::
  Generator (const Sample_type& Sample,
             const Channel_type& Channel)
  : ::xml_schema::type (),
    Sample_ (Sample, ::xml_schema::flags (), this),
    Channel_ (Channel, ::xml_schema::flags (), this)
  {
  }

  Generator::
  Generator (const Generator& x,
             ::xml_schema::flags f,
             ::xml_schema::container* c)
  : ::xml_schema::type (x, f, c),
    Sample_ (x.Sample_, f, this),
    Channel_ (x.Channel_, f, this)
  {
  }

  Generator::
  Generator (const ::xercesc::DOMElement& e,
             ::xml_schema::flags f,
             ::xml_schema::container* c)
  : ::xml_schema::type (e, f | ::xml_schema::flags::base, c),
    Sample_ (f, this),
    Channel_ (f, this)
  {
    if ((f & ::xml_schema::flags::base) == 0)
    {
      ::xsd::cxx::xml::dom::parser< char > p (e, true, false);
      this->parse (p, f);
    }
  }

  void Generator::
  parse (::xsd::cxx::xml::dom::parser< char >& p,
         ::xml_schema::flags f)
  {
    for (; p.more_elements (); p.next_element ())
    {
      const ::xercesc::DOMElement& i (p.cur_element ());
      const ::xsd::cxx::xml::qualified_name< char > n (
        ::xsd::cxx::xml::dom::name< char > (i));

      // Sample
      //
      if (n.name () == "Sample" && n.namespace_ ().empty ())
      {
        ::std::auto_ptr< Sample_type > r (
          Sample_traits::create (i, f, this));

        if (!Sample_.present ())
        {
          this->Sample_.set (r);
          continue;
        }
      }

      // Channel
      //
      if (n.name () == "Channel" && n.namespace_ ().empty ())
      {
        if (!Channel_.present ())
        {
          this->Channel_.set (Channel_traits::create (i, f, this));
          continue;
        }
      }

      break;
    }

    if (!Sample_.present ())
    {
      throw ::xsd::cxx::tree::expected_element< char > (
        "Sample",
        "");
    }

    if (!Channel_.present ())
    {
      throw ::xsd::cxx::tree::expected_element< char > (
        "Channel",
        "");
    }
  }

  Generator* Generator::
  _clone (::xml_schema::flags f,
          ::xml_schema::container* c) const
  {
    return new class Generator (*this, f, c);
  }

  Generator::
  ~Generator ()
  {
  }

  // Jass
  //

  Jass::
  Jass ()
  : ::xml_schema::type (),
    Generator_ (::xml_schema::flags (), this)
  {
  }

  Jass::
  Jass (const Jass& x,
        ::xml_schema::flags f,
        ::xml_schema::container* c)
  : ::xml_schema::type (x, f, c),
    Generator_ (x.Generator_, f, this)
  {
  }

  Jass::
  Jass (const ::xercesc::DOMElement& e,
        ::xml_schema::flags f,
        ::xml_schema::container* c)
  : ::xml_schema::type (e, f | ::xml_schema::flags::base, c),
    Generator_ (f, this)
  {
    if ((f & ::xml_schema::flags::base) == 0)
    {
      ::xsd::cxx::xml::dom::parser< char > p (e, true, false);
      this->parse (p, f);
    }
  }

  void Jass::
  parse (::xsd::cxx::xml::dom::parser< char >& p,
         ::xml_schema::flags f)
  {
    for (; p.more_elements (); p.next_element ())
    {
      const ::xercesc::DOMElement& i (p.cur_element ());
      const ::xsd::cxx::xml::qualified_name< char > n (
        ::xsd::cxx::xml::dom::name< char > (i));

      // Generator
      //
      if (n.name () == "Generator" && n.namespace_ ().empty ())
      {
        ::std::auto_ptr< Generator_type > r (
          Generator_traits::create (i, f, this));

        this->Generator_.push_back (r);
        continue;
      }

      break;
    }
  }

  Jass* Jass::
  _clone (::xml_schema::flags f,
          ::xml_schema::container* c) const
  {
    return new class Jass (*this, f, c);
  }

  Jass::
  ~Jass ()
  {
  }
}

#include <istream>
#include <xsd/cxx/xml/sax/std-input-source.hxx>
#include <xsd/cxx/tree/error-handler.hxx>

namespace Jass
{
  ::std::auto_ptr< ::Jass::Jass >
  Jass_ (const ::std::string& u,
         ::xml_schema::flags f,
         const ::xml_schema::properties& p)
  {
    ::xsd::cxx::xml::auto_initializer i (
      (f & ::xml_schema::flags::dont_initialize) == 0,
      (f & ::xml_schema::flags::keep_dom) == 0);

    ::xsd::cxx::tree::error_handler< char > h;

    ::xml_schema::dom::auto_ptr< ::xercesc::DOMDocument > d (
      ::xsd::cxx::xml::dom::parse< char > (
        u, h, p, f));

    h.throw_if_failed< ::xsd::cxx::tree::parsing< char > > ();

    ::std::auto_ptr< ::Jass::Jass > r (
      ::Jass::Jass_ (
        d, f | ::xml_schema::flags::own_dom, p));

    return r;
  }

  ::std::auto_ptr< ::Jass::Jass >
  Jass_ (const ::std::string& u,
         ::xml_schema::error_handler& h,
         ::xml_schema::flags f,
         const ::xml_schema::properties& p)
  {
    ::xsd::cxx::xml::auto_initializer i (
      (f & ::xml_schema::flags::dont_initialize) == 0,
      (f & ::xml_schema::flags::keep_dom) == 0);

    ::xml_schema::dom::auto_ptr< ::xercesc::DOMDocument > d (
      ::xsd::cxx::xml::dom::parse< char > (
        u, h, p, f));

    if (!d.get ())
      throw ::xsd::cxx::tree::parsing< char > ();

    ::std::auto_ptr< ::Jass::Jass > r (
      ::Jass::Jass_ (
        d, f | ::xml_schema::flags::own_dom, p));

    return r;
  }

  ::std::auto_ptr< ::Jass::Jass >
  Jass_ (const ::std::string& u,
         ::xercesc::DOMErrorHandler& h,
         ::xml_schema::flags f,
         const ::xml_schema::properties& p)
  {
    ::xml_schema::dom::auto_ptr< ::xercesc::DOMDocument > d (
      ::xsd::cxx::xml::dom::parse< char > (
        u, h, p, f));

    if (!d.get ())
      throw ::xsd::cxx::tree::parsing< char > ();

    ::std::auto_ptr< ::Jass::Jass > r (
      ::Jass::Jass_ (
        d, f | ::xml_schema::flags::own_dom, p));

    return r;
  }

  ::std::auto_ptr< ::Jass::Jass >
  Jass_ (::std::istream& is,
         ::xml_schema::flags f,
         const ::xml_schema::properties& p)
  {
    ::xsd::cxx::xml::auto_initializer i (
      (f & ::xml_schema::flags::dont_initialize) == 0,
      (f & ::xml_schema::flags::keep_dom) == 0);

    ::xsd::cxx::xml::sax::std_input_source isrc (is);
    return ::Jass::Jass_ (isrc, f, p);
  }

  ::std::auto_ptr< ::Jass::Jass >
  Jass_ (::std::istream& is,
         ::xml_schema::error_handler& h,
         ::xml_schema::flags f,
         const ::xml_schema::properties& p)
  {
    ::xsd::cxx::xml::auto_initializer i (
      (f & ::xml_schema::flags::dont_initialize) == 0,
      (f & ::xml_schema::flags::keep_dom) == 0);

    ::xsd::cxx::xml::sax::std_input_source isrc (is);
    return ::Jass::Jass_ (isrc, h, f, p);
  }

  ::std::auto_ptr< ::Jass::Jass >
  Jass_ (::std::istream& is,
         ::xercesc::DOMErrorHandler& h,
         ::xml_schema::flags f,
         const ::xml_schema::properties& p)
  {
    ::xsd::cxx::xml::sax::std_input_source isrc (is);
    return ::Jass::Jass_ (isrc, h, f, p);
  }

  ::std::auto_ptr< ::Jass::Jass >
  Jass_ (::std::istream& is,
         const ::std::string& sid,
         ::xml_schema::flags f,
         const ::xml_schema::properties& p)
  {
    ::xsd::cxx::xml::auto_initializer i (
      (f & ::xml_schema::flags::dont_initialize) == 0,
      (f & ::xml_schema::flags::keep_dom) == 0);

    ::xsd::cxx::xml::sax::std_input_source isrc (is, sid);
    return ::Jass::Jass_ (isrc, f, p);
  }

  ::std::auto_ptr< ::Jass::Jass >
  Jass_ (::std::istream& is,
         const ::std::string& sid,
         ::xml_schema::error_handler& h,
         ::xml_schema::flags f,
         const ::xml_schema::properties& p)
  {
    ::xsd::cxx::xml::auto_initializer i (
      (f & ::xml_schema::flags::dont_initialize) == 0,
      (f & ::xml_schema::flags::keep_dom) == 0);

    ::xsd::cxx::xml::sax::std_input_source isrc (is, sid);
    return ::Jass::Jass_ (isrc, h, f, p);
  }

  ::std::auto_ptr< ::Jass::Jass >
  Jass_ (::std::istream& is,
         const ::std::string& sid,
         ::xercesc::DOMErrorHandler& h,
         ::xml_schema::flags f,
         const ::xml_schema::properties& p)
  {
    ::xsd::cxx::xml::sax::std_input_source isrc (is, sid);
    return ::Jass::Jass_ (isrc, h, f, p);
  }

  ::std::auto_ptr< ::Jass::Jass >
  Jass_ (::xercesc::InputSource& i,
         ::xml_schema::flags f,
         const ::xml_schema::properties& p)
  {
    ::xsd::cxx::tree::error_handler< char > h;

    ::xml_schema::dom::auto_ptr< ::xercesc::DOMDocument > d (
      ::xsd::cxx::xml::dom::parse< char > (
        i, h, p, f));

    h.throw_if_failed< ::xsd::cxx::tree::parsing< char > > ();

    ::std::auto_ptr< ::Jass::Jass > r (
      ::Jass::Jass_ (
        d, f | ::xml_schema::flags::own_dom, p));

    return r;
  }

  ::std::auto_ptr< ::Jass::Jass >
  Jass_ (::xercesc::InputSource& i,
         ::xml_schema::error_handler& h,
         ::xml_schema::flags f,
         const ::xml_schema::properties& p)
  {
    ::xml_schema::dom::auto_ptr< ::xercesc::DOMDocument > d (
      ::xsd::cxx::xml::dom::parse< char > (
        i, h, p, f));

    if (!d.get ())
      throw ::xsd::cxx::tree::parsing< char > ();

    ::std::auto_ptr< ::Jass::Jass > r (
      ::Jass::Jass_ (
        d, f | ::xml_schema::flags::own_dom, p));

    return r;
  }

  ::std::auto_ptr< ::Jass::Jass >
  Jass_ (::xercesc::InputSource& i,
         ::xercesc::DOMErrorHandler& h,
         ::xml_schema::flags f,
         const ::xml_schema::properties& p)
  {
    ::xml_schema::dom::auto_ptr< ::xercesc::DOMDocument > d (
      ::xsd::cxx::xml::dom::parse< char > (
        i, h, p, f));

    if (!d.get ())
      throw ::xsd::cxx::tree::parsing< char > ();

    ::std::auto_ptr< ::Jass::Jass > r (
      ::Jass::Jass_ (
        d, f | ::xml_schema::flags::own_dom, p));

    return r;
  }

  ::std::auto_ptr< ::Jass::Jass >
  Jass_ (const ::xercesc::DOMDocument& d,
         ::xml_schema::flags f,
         const ::xml_schema::properties& p)
  {
    if (f & ::xml_schema::flags::keep_dom)
    {
      ::xml_schema::dom::auto_ptr< ::xercesc::DOMDocument > c (
        static_cast< ::xercesc::DOMDocument* > (d.cloneNode (true)));

      ::std::auto_ptr< ::Jass::Jass > r (
        ::Jass::Jass_ (
          c, f | ::xml_schema::flags::own_dom, p));

      return r;
    }

    const ::xercesc::DOMElement& e (*d.getDocumentElement ());
    const ::xsd::cxx::xml::qualified_name< char > n (
      ::xsd::cxx::xml::dom::name< char > (e));

    if (n.name () == "Jass" &&
        n.namespace_ () == "http://shirkhan.dyndns.org/Jass")
    {
      ::std::auto_ptr< ::Jass::Jass > r (
        ::xsd::cxx::tree::traits< ::Jass::Jass, char >::create (
          e, f, 0));
      return r;
    }

    throw ::xsd::cxx::tree::unexpected_element < char > (
      n.name (),
      n.namespace_ (),
      "Jass",
      "http://shirkhan.dyndns.org/Jass");
  }

  ::std::auto_ptr< ::Jass::Jass >
  Jass_ (::xml_schema::dom::auto_ptr< ::xercesc::DOMDocument >& d,
         ::xml_schema::flags f,
         const ::xml_schema::properties&)
  {
    ::xml_schema::dom::auto_ptr< ::xercesc::DOMDocument > c (
      ((f & ::xml_schema::flags::keep_dom) &&
       !(f & ::xml_schema::flags::own_dom))
      ? static_cast< ::xercesc::DOMDocument* > (d->cloneNode (true))
      : 0);

    ::xercesc::DOMDocument& doc (c.get () ? *c : *d);
    const ::xercesc::DOMElement& e (*doc.getDocumentElement ());

    const ::xsd::cxx::xml::qualified_name< char > n (
      ::xsd::cxx::xml::dom::name< char > (e));

    if (f & ::xml_schema::flags::keep_dom)
      doc.setUserData (::xml_schema::dom::tree_node_key,
                       (c.get () ? &c : &d),
                       0);

    if (n.name () == "Jass" &&
        n.namespace_ () == "http://shirkhan.dyndns.org/Jass")
    {
      ::std::auto_ptr< ::Jass::Jass > r (
        ::xsd::cxx::tree::traits< ::Jass::Jass, char >::create (
          e, f, 0));
      return r;
    }

    throw ::xsd::cxx::tree::unexpected_element < char > (
      n.name (),
      n.namespace_ (),
      "Jass",
      "http://shirkhan.dyndns.org/Jass");
  }
}

#include <xsd/cxx/post.hxx>

// Begin epilogue.
//
//
// End epilogue.
