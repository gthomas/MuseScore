//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: chord.h,v 1.3 2006/03/02 17:08:33 wschweer Exp $
//
//  Copyright (C) 2002-2006 Werner Schweer (ws@seh.de)
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

#ifndef __CHORD_H__
#define __CHORD_H__

/**
 \file
 Definition of classes Chord, HelpLine, NoteList and Stem.
*/

#include "globals.h"
#include "chordrest.h"

class Note;
class Hook;
class Arpeggio;
class Tremolo;

//---------------------------------------------------------
//   Stem
//    Notenhals
//---------------------------------------------------------

/**
 Graphic representation of a note stem.
*/

class Stem : public Element {
      Spatium _len;

   public:
      Stem(Score*);
      Stem &operator=(const Stem&);

      virtual Stem* clone() const { return new Stem(*this); }
      virtual ElementType type() const { return STEM; }
      virtual void draw(QPainter& p);
      void setLen(const Spatium&);
      virtual QRectF bbox() const;
      };

//---------------------------------------------------------
//   NoteList
//---------------------------------------------------------

/**
 List of notes.

 Used by Chord to store its notes.
*/

class NoteList : public std::multimap <const int, Note*> {
   public:
      NoteList::iterator add(Note* n);
      Note* front() const { return (begin() != end()) ? begin()->second : 0;  }
      Note* back()  const { return (begin() != end()) ? rbegin()->second : 0; }
      Note* find(int pitch) const;
      };

typedef NoteList::iterator iNote;
typedef NoteList::reverse_iterator riNote;
typedef NoteList::const_iterator ciNote;
typedef NoteList::const_reverse_iterator criNote;

//---------------------------------------------------------
//   LedgerLine
//---------------------------------------------------------

/**
 Graphic representation of a ledger line.
*/

class LedgerLine : public Line {
   public:
      LedgerLine(Score*);
      LedgerLine &operator=(const LedgerLine&);
      virtual LedgerLine* clone() const { return new LedgerLine(*this); }
      virtual ElementType type() const { return LEDGER_LINE; }
      };

typedef QList<LedgerLine*>::iterator iLedgerLine;
typedef QList<LedgerLine*>::const_iterator ciLedgerLine;

//---------------------------------------------------------
//   Chord
//---------------------------------------------------------

/**
 Graphic representation of a chord.

 Single notes are handled as degenerated chords.
*/

class Chord : public ChordRest {
      NoteList notes;
      QList<LedgerLine*> _ledgerLines;
      Stem* _stem;
      Hook* _hook;
      Direction _stemDirection;
      bool _grace;
      Arpeggio* _arpeggio;
      Tremolo* _tremolo;

      void computeUp();
      void readSlur(QDomElement, int staff);
      virtual qreal upPos()   const;
      virtual qreal downPos() const;
      virtual qreal centerX() const;
      void addLedgerLine(double x, double y, int i);

   public:
      Chord(Score*);
      ~Chord();
      Chord &operator=(const Chord&);

      virtual Chord* clone() const     { return new Chord(*this); }
      virtual ElementType type() const { return CHORD; }

      virtual void write(Xml& xml) const;
      virtual void read(QDomElement, int staff);
      virtual void setSelected(bool f);
      virtual void dump() const;

      virtual QRectF bbox() const;
      void setStemDirection(Direction d)     { _stemDirection = d; }
      Direction stemDirection() const        { return _stemDirection; }
      bool grace() const                     { return _grace; }
      void setGrace(bool g)                  { _grace = g; }

      QList<LedgerLine*>* ledgerLines()      { return &_ledgerLines; }

      virtual void layoutStem(ScoreLayout*);
      NoteList* noteList()                   { return &notes; }
      const NoteList* noteList() const       { return &notes; }
      const Note* upNote() const             { return notes.back(); }
      const Note* downNote() const           { return notes.front(); }
      Note* upNote()                         { return notes.back(); }
      Note* downNote()                       { return notes.front(); }
      virtual int move() const;

      Stem* stem() const                     { return _stem; }
      void setStem(Stem* s);
      Arpeggio* arpeggio() const             { return _arpeggio; }
      Tremolo* tremolo() const               { return _tremolo;  }

      virtual QPointF stemPos(bool, bool) const;

      Hook* hook() const                     { return _hook; }
      void setHook(Hook* f);

      virtual void add(Element*);
      virtual void remove(Element*);

      Note* selectedNote() const;
      virtual void layout(ScoreLayout*);

      virtual int upLine() const;
      virtual int downLine() const;
      virtual void space(double& min, double& extra) const;
      void readNote(QDomElement node, int staffIdx);
      };

#endif

