//=============================================================================
//  MusE Reader
//  Music Score Reader
//  $Id$
//
//  Copyright (C) 2010 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#include "omr.h"
#include "omrview.h"
#include "libmscore/xml.h"
#include "omrpage.h"
#include "pdf.h"
#ifdef OCR
#include "ocr.h"
#endif
#include "utils.h"

#ifdef OMR

class ScoreView;

//---------------------------------------------------------
//   Omr
//---------------------------------------------------------

Omr::Omr(Score* s)
      {
      _score = s;
#ifdef OCR
      _ocr = 0;
#endif
      initUtils();
      }

Omr::Omr(const QString& p, Score* s)
      {
      _score = s;
      _path = p;
      _ocr = 0;
      initUtils();
      }

//---------------------------------------------------------
//   newOmrView
//---------------------------------------------------------

OmrView* Omr::newOmrView(ScoreView* sv)
      {
      OmrView* ov = new OmrView(sv);
      ov->setOmr(this);
      return ov;
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Omr::write(Xml& xml) const
      {
      xml.stag("Omr");
      xml.tag("path", _path);
      xml.tag("spatium", _spatium);
      xml.tag("dpmm", _dpmm);
      foreach(OmrPage* page, _pages) {
            page->write(xml);
            }
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Omr::read(QDomElement e)
      {
      _doc = 0;
#ifdef OCR
      if (_ocr == 0)
            _ocr = new Ocr;
      _ocr->init();
#endif
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            QString val(e.text());

            if (tag == "path") {
                  _path = val;
                  }
            else if (tag == "OmrPage") {
                  OmrPage* page = new OmrPage(this);
                  page->read(e);
                  _pages.append(page);
                  }
            else if (tag == "spatium")
                  _spatium = val.toDouble();
            else if (tag == "dpmm")
                  _dpmm = val.toDouble();
            else
                  domError(e);
            }
      }

//---------------------------------------------------------
//   pagesInDocument
//---------------------------------------------------------

int Omr::pagesInDocument() const
      {
      return _doc ? _doc->numPages() : 0;
      }

//---------------------------------------------------------
//   readPdf
//    return true on success
//---------------------------------------------------------

bool Omr::readPdf()
      {
#ifdef OCR
      if (_ocr == 0)
            _ocr = new Ocr;
      _ocr->init();
#endif

printf("Omr::read <%s>\n", qPrintable(_path));

      _doc = new Pdf(_path);

      int n = _doc->numPages();
      for (int i = 0; i < n; ++i) {
            OmrPage* page = new OmrPage(this);
            QImage image = _doc->page(i);
            page->setImage(image);
            _pages.append(page);
            }
      double sp = 0;
      double w  = 0;

      for (int i = 0; i < n; ++i) {
            _pages[i]->read(i);
            sp += _pages[i]->spatium();
            w  += _pages[i]->width();
            }
      _spatium = sp / n;
      w       /= n;
      _dpmm    = w / 210.0;            // PaperSize A4

printf("*** spatium: %f mm  dpmm: %f\n", spatiumMM(), _dpmm);
      return true;
      }

//---------------------------------------------------------
//   spatiumMM
//---------------------------------------------------------

double Omr::spatiumMM() const
      {
      return _spatium / _dpmm;
      }

//---------------------------------------------------------
//   staffDistance
//---------------------------------------------------------

double Omr::staffDistance() const
      {
      return _pages[0]->staffDistance();
      }

//---------------------------------------------------------
//   systemDistance
//---------------------------------------------------------

double Omr::systemDistance() const
      {
      return _pages[0]->systemDistance();
      }

#endif

