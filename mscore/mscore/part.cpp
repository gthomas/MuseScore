//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2002-2007 Werner Schweer and others
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

#include "part.h"
#include "staff.h"
#include "xml.h"
#include "score.h"
#include "style.h"
#include "note.h"
#include "drumset.h"
#include "instrtemplate.h"
#include "text.h"
#include "measure.h"
#include "tablature.h"

//---------------------------------------------------------
//   Part
//---------------------------------------------------------

Part::Part(Score* s)
      {
      _instr = new Instrument;
      _longName  = new Text(s);
      _longName->setSubtype(TEXT_INSTRUMENT_LONG);
      _longName->setTextStyle(TEXT_STYLE_INSTRUMENT_LONG);
      _shortName = new Text(s);
      _shortName->setSubtype(TEXT_INSTRUMENT_SHORT);
      _shortName->setTextStyle(TEXT_STYLE_INSTRUMENT_SHORT);
      _score = s;
      _show  = true;
      }

Part::~Part()
      {
      delete _instr;
      delete _longName;
      delete _shortName;
      }

//---------------------------------------------------------
//   initFromInstrTemplate
//---------------------------------------------------------

void Part::initFromInstrTemplate(const InstrumentTemplate* t)
      {
      _instr->setAmateurPitchRange(t->minPitchA, t->maxPitchA);
      _instr->setProfessionalPitchRange(t->minPitchP, t->maxPitchP);

      setShortNameEncoded(t->shortName);
      _instr->setTrackName(t->trackName);
      setLongNameEncoded(t->longName);

      _instr->setTranspose(t->transpose);
      if (t->useDrumset) {
            _instr->setUseDrumset(true);
            _instr->setDrumset(new Drumset(*((t->drumset) ? t->drumset : smDrumset)));
            }
      _instr->setMidiActions(t->midiActions);
      _instr->setArticulation(t->articulation);
      _instr->setChannel(t->channel);
      _instr->setTablature(t->tablature ? new Tablature(*t->tablature) : 0);
      }

//---------------------------------------------------------
//   staff
//---------------------------------------------------------

Staff* Part::staff(int idx) const
      {
      return _staves[idx];
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Part::read(QDomElement e)
      {
      int rstaff = 0;
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            QString val(e.text());
            if (tag == "Staff") {
                  Staff* staff = new Staff(_score, this, rstaff);
                  _score->staves().push_back(staff);
                  _staves.push_back(staff);
                  staff->read(e);
                  ++rstaff;
                  }
            else if (tag == "Instrument")
                  _instr->read(e);
            else if (tag == "name") {
                  if (_score->mscVersion() <= 101)
                        _longName->setHtml(val);
                  else if (_score->mscVersion() <= 110)
                        _longName->setHtml(Xml::htmlToString(e.firstChildElement()));
                  else
                        _longName->read(e);
                  }
            else if (tag == "shortName") {
                  if (_score->mscVersion() <= 101)
                        _shortName->setHtml(val);
                  else if (_score->mscVersion() <= 110)
                        _shortName->setHtml(Xml::htmlToString(e.firstChildElement()));
                  else
                        _shortName->read(e);
                  }
            else if (tag == "trackName") {
                  _instr->setTrackName(val);
                  }
            else if (tag == "show")
                  _show = val.toInt();
            else
                  domError(e);
            }
      }

//---------------------------------------------------------
//   parseInstrName
//---------------------------------------------------------

static void parseInstrName(QTextDocument* doc, const QString& name)
      {
      if (name.isEmpty())
            return;
      QTextCursor cursor(doc);
      QTextCharFormat f = cursor.charFormat();
      QTextCharFormat sf(f);

      QFont font("MScore1-test");
      sf.setFont(font);

      QDomDocument dom;
      int line, column;
      QString err;
      if (!dom.setContent(name, false, &err, &line, &column)) {
            QString col, ln;
            col.setNum(column);
            ln.setNum(line);
            QString error = err + "\n at line " + ln + " column " + col;
            printf("parse instrument name: %s\n", qPrintable(error));
            printf("   data:<%s>\n", qPrintable(name));
            return;
            }

      for (QDomNode e = dom.documentElement(); !e.isNull(); e = e.nextSibling()) {
            for (QDomNode ee = e.firstChild(); !ee.isNull(); ee = ee.nextSibling()) {
                  QDomElement de1 = ee.toElement();
                  QString tag(de1.tagName());
                  if (tag == "symbol") {
                        QString name = de1.attribute(QString("name"));
                        if (name == "flat")
                              cursor.insertText(QString(0xe10d), sf);
                        else if (name == "sharp")
                              cursor.insertText(QString(0xe10c), sf);
                        }
                  QDomText t = ee.toText();
                  if (!t.isNull())
                        cursor.insertText(t.data(), f);
                  }
            }
      }

//---------------------------------------------------------
//   setLongName
//---------------------------------------------------------

void Part::setLongNameEncoded(const QString& name)
      {
      parseInstrName(_longName->doc(), name);
      }

void Part::setLongName(const QString& name)
      {
      _longName->setText(name);
      }

void Part::setLongNameHtml(const QString& s)
      {
      _longName->setHtml(s);
      }

void Part::setLongName(const QTextDocument& s)
      {
//TODOxx      _longName->setDoc(s);
      }

//---------------------------------------------------------
//   setShortName
//---------------------------------------------------------

void Part::setShortNameEncoded(const QString& s)
      {
      parseInstrName(_shortName->doc(), s);
      }

void Part::setShortName(const QString& s)
      {
      _shortName->setText(s);
      }

void Part::setShortNameHtml(const QString& s)
      {
      _shortName->setHtml(s);
      }

void Part::setShortName(const QTextDocument& s)
      {
//TODOxx      _shortName->setDoc(s);
      }

//---------------------------------------------------------
//   shortNameHtml
//---------------------------------------------------------

QString Part::shortNameHtml() const
      {
      return _shortName->getHtml();
      }

//---------------------------------------------------------
//   longNameHtml
//---------------------------------------------------------

QString Part::longNameHtml() const
      {
      return _longName->getHtml();
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Part::write(Xml& xml) const
      {
      xml.stag("Part");
      foreach(const Staff* staff, _staves)
            staff->write(xml);
      if (!_longName->isEmpty())
            _longName->write(xml, "name");
      if (!_shortName->isEmpty())
            _shortName->write(xml, "shortName");
      if (!_show)
            xml.tag("show", _show);
      _instr->write(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   setStaves
//---------------------------------------------------------

void Part::setStaves(int n)
      {
      int ns = _staves.size();
      if (n < ns) {
            printf("Part::setStaves(): remove staves not implemented!\n");
            return;
            }
      int staffIdx = _score->staffIdx(this) + ns;
      for (int i = ns; i < n; ++i) {
            Staff* staff = new Staff(_score, this, i);
            _staves.push_back(staff);
            _score->staves().insert(staffIdx, staff);
            for (MeasureBase* mb = _score->first(); mb; mb = mb->next()) {
                  if (mb->type() != MEASURE)
                        continue;
                  Measure* m = static_cast<Measure*>(mb);
                  m->insertStaff(staff, staffIdx);
                  }
            ++staffIdx;
            }
      }

//---------------------------------------------------------
//   insertStaff
//---------------------------------------------------------

void Part::insertStaff(Staff* staff)
      {
      int idx = staff->rstaff();
      if (idx > _staves.size())
            idx = _staves.size();
      _staves.insert(idx, staff);
      staff->setShow(_show);
      idx = 0;
      foreach(Staff* staff, _staves)
            staff->setRstaff(idx++);
      }

//---------------------------------------------------------
//   removeStaff
//---------------------------------------------------------

void Part::removeStaff(Staff* staff)
      {
      _staves.removeAll(staff);
      int idx = 0;
      foreach(Staff* staff, _staves)
            staff->setRstaff(idx++);
      }

//---------------------------------------------------------
//   setShow
//---------------------------------------------------------

void Part::setShow(bool val)
      {
      _show = val;
      foreach(Staff* staff, _staves)
            staff->setShow(_show);
      }

//---------------------------------------------------------
//   setMidiProgram
//    TODO
//---------------------------------------------------------

void Part::setMidiProgram(int p)
      {
      // LVIFIX: check if this is correct
      // at least it fixes the MIDI program handling in the MusicXML regression test
      Channel c = _instr->channel(0);
      c.program = p;
      c.updateInitList();
      _instr->setChannel(0, c);
      }

int Part::volume() const
      {
      return _instr->channel(0).volume;
      }

int Part::reverb() const
      {
      return _instr->channel(0).reverb;
      }

int Part::chorus() const
      {
      return _instr->channel(0).chorus;
      }

int Part::pan() const
      {
      return _instr->channel(0).pan;
      }

int Part::midiProgram() const
      {
      return _instr->channel(0).program;
      }

//---------------------------------------------------------
//   midiChannel
//---------------------------------------------------------

int Part::midiChannel() const
      {
      return score()->midiChannel(_instr->channel(0).channel);
      }

//---------------------------------------------------------
//   setMidiChannel
//    called from importmusicxml
//---------------------------------------------------------

void Part::setMidiChannel(int) const
      {
      }

//---------------------------------------------------------
//   setInstrument
//---------------------------------------------------------

void Part::setInstrument(const Instrument& i)
      {
      delete _instr;
      _instr = new Instrument(i);

      if (!_score->styleB(ST_concertPitch) && _instr->transpose().chromatic) {
            foreach(Staff* staff, _staves) {
                  _score->cmdTransposeStaff(staff->idx(), _instr->transpose(), false);
                  }
            }
      if (!_score->styleB(ST_concertPitch) && _instr->transpose().chromatic) {
            foreach(Staff* staff, _staves) {
                  Interval iv(_instr->transpose());
                  iv.flip();
                  _score->cmdTransposeStaff(staff->idx(), iv, false);
                  }
            }
      }

