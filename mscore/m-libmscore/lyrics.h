//=============================================================================
//  MuseScore
//  Music Score Editor/Player
//  $Id:$
//
//  Copyright (C) 2002-2011 Werner Schweer
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

#ifndef __LYRICS_H__
#define __LYRICS_H__

#include "text.h"
#include "chord.h"

class Segment;
class Chord;
class Painter;

//---------------------------------------------------------
//   Lyrics
//---------------------------------------------------------

class Lyrics : public Text {
   public:
      enum Syllabic { SINGLE, BEGIN, END, MIDDLE };

   private:
      int _no;                ///< row index
      int _ticks;             ///< if > 0 then draw an underline to tick() + _ticks
      Syllabic _syllabic;
      QList<Line*> _separator;
      Text* _verseNumber;

   public:
      Lyrics(Score*);
      Lyrics(const Lyrics&);
      ~Lyrics();
      virtual Lyrics* clone() const    { return new Lyrics(*this); }
      virtual ElementType type() const { return LYRICS; }
      virtual QPointF canvasPos() const;
      virtual void scanElements(void* data, void (*func)(void*, Element*));

      Segment* segment() const { return (Segment*)parent()->parent(); }
      Measure* measure() const { return (Measure*)parent()->parent()->parent(); }
      ChordRest* chordRest() const { return (ChordRest*)parent(); }

      virtual void layout();

      virtual void read(XmlReader*);
      void setNo(int n)             { _no = n; }
      int no() const                { return _no; }
      void setSyllabic(Syllabic s)  { _syllabic = s; }
      Syllabic syllabic() const     { return _syllabic; }
      virtual void add(Element*);
      virtual void remove(Element*);
      virtual void draw(Painter*) const;

      int ticks() const             { return _ticks;    }
      void setTicks(int tick)       { _ticks = tick;    }
      int endTick() const;

      void clearSeparator()         { _separator.clear(); } // TODO: memory leak
      QList<Line*>* separatorList() { return &_separator; }
      Text* verseNumber() const     { return _verseNumber; }
      void setVerseNumber(Text* t)  { _verseNumber = t;    }
      };

#endif

