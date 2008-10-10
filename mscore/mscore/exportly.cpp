//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: exportxml.cpp,v 1.71 2006/03/28 14:58:58 wschweer Exp $
//
//  Copyright (C) 2007 Werner Schweer and others
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
/**
 Lilypond export. Olav's in-progress-copy
 For NEWS and TODOS: see end of file
 Still too many problems for exportly to be usable.
 */
#include "config.h"
#include "score.h"
#include "part.h"
#include "measure.h"
#include "staff.h"
#include "layout.h"
#include "chord.h"
#include "note.h"
#include "rest.h"
#include "timesig.h"
#include "clef.h"
#include "keysig.h"
#include "key.h"
#include "page.h"
#include "text.h"
#include "slur.h"
#include "beam.h"
#include "tuplet.h"
#include "articulation.h"
#include "barline.h"
#include "volta.h"
#include <string.h>


//---------------------------------------------------------
//   ExportLy
//---------------------------------------------------------

class ExportLy {
  Score* score;
  QFile f;
  QTextStream os;
  int level;        // indent level
  int curTicks;
  bool slur;
  Direction stemDirection;
  QString  voiceid[32];
  int indx;
  bool graceswitch;
  int prevpitch, chordpitch, oktavdiff;
  int measurenumber, lastind, taktnr;
  bool repeatactive;
  bool firstalt,secondalt;
  enum voltatype {startending, endending, startrepeat, endrepeat, bothrepeat, endbar, none};
  struct  voltareg { voltatype voltart; int barno; };
  struct voltareg  voltarray[255];
  int tupletcount;
  bool pianostaff;


  void indent();
  int getLen(int ticks, int* dots);
  void writeLen(int);
  QString tpc2name(int tpc);
  void checkSlur(int, int);

  void writeScore();
  void writeMeasure(Measure*, int);
  void writeKeySig(int);
  void writeTimeSig(TimeSig*);
  void writeClef(int);
  void writeChord(Chord*);
  void writeRest(int, int, int);
  void findVolta();
  void writeBarline(Measure* m);
  int  voltacheckbar(Measure * meas, int i);
  void writevolta(int measurenumber, int lastind);
  void findtuplets(Note* note);

public:
  ExportLy(Score* s) {
    score  = s;
    level  = 0;
    curTicks = division;
    slur   = false;
    stemDirection = AUTO;
  }
  bool write(const QString& name);
};




//---------------------------------------------------------
// abs num value
//---------------------------------------------------------
int numval(int num)
{  if (num <0) return -num;
  return num;
}

//---------------------------------------------------------
//   indent
//---------------------------------------------------------

void ExportLy::indent()
      {
      for (int i = 0; i < level; ++i)
            os << "    ";
      }




//-------------------------------------
// Find tuplets
//-------------------------------------

void ExportLy::findtuplets(Note* note)

{

  Tuplet* t = note->chord()->tuplet();
  int actNotes = 1;
  int nrmNotes = 1;

  if (t)
    {
      if (tupletcount == 0)
	{
	  actNotes = t->actualNotes();
	  nrmNotes = t->normalNotes();
	  printf("tuplet found. actual: %d, normal: %d\n", actNotes, nrmNotes);
	  tupletcount=actNotes;
	  printf("tupletcount set: %d\n", tupletcount);
	  os << "\\times " <<  nrmNotes << "/" << actNotes << "{" ;
	}
      else if (tupletcount>1)
	{
	  tupletcount--;
	  printf("tupletcount %d\n", tupletcount);
	}
    }
}
//-----------------end of check-tuplets


int ExportLy::voltacheckbar(Measure* meas, int i)
{
  printf("at volta-checkbarline\n");

  int blt = meas->endBarLineType();
  printf("endbarlinetype: %d\n", blt);
  switch(blt)
    {
    case START_REPEAT:
      i++;
      printf("start-repeat-bar taktnr. %d i: %d \n", taktnr, i);
      voltarray[i].voltart=startrepeat;
      voltarray[i].barno=taktnr;
      printf("startrepeat: voltart: %d barno: %d\n", voltarray[i].voltart, voltarray[i].barno);
      break;
    case END_REPEAT:
      i++;
      printf("endrepeat %d\n", taktnr);
      voltarray[i].voltart=endrepeat;
      voltarray[i].barno=taktnr;
      break;
    case END_START_REPEAT:
      i++;
      printf("both repeats %d\n", taktnr);
      voltarray[i].voltart=bothrepeat;
      voltarray[i].barno=taktnr;
      break;
    case END_BAR:
      i++;
      printf("endbar %d\n", taktnr);
      voltarray[i].voltart=endbar;
      voltarray[i].barno=taktnr;
      break;
    default:
      printf("no important barline %d\n", taktnr);
      break;
    }//switch
  return i;
}//end voltacheckbarline

//---------------------------------------------------------
//   findVolta -- find and register volta and repeats in entire piece
//---------------------------------------------------------

void  ExportLy::findVolta()

{
  taktnr=0;
  lastind=0;
  int i=0;

  for (i=0; i<255; i++)
    {
      voltarray[i].voltart=none;
      voltarray[i].barno=0;
    }

  i=0;

  for (MeasureBase * m=score->layout()->first(); m; m=m->next())
    {// for all measures
      if (m->type() !=MEASURE )
	continue;

      //hvis det derimot er en takt:

      ++taktnr;

      foreach (Element* el, *(m->score()->gel()))
	//walk thru all elements of measure
	{
	  if (el->type() == VOLTA)
	    {
	      Volta* v = (Volta*) el;

	      if (v->tick() == m->tick()) //hvis det er først i takten
		{
		  i++;
		  printf("i: %d\n", i);
		  if (v->subtype() == Volta::VOLTA_CLOSED)
		    {//lilypond developers have "never seen" last ending closed.
		      //So they are reluctant to implement it. Final ending is always "open" in lilypond.
		      printf("left volta closed\n");
		    }
		  else if (v->subtype() == Volta::VOLTA_OPEN)
		    {
		      printf("left volta open\n");
		    }
		  voltarray[i].voltart = startending;
		  voltarray[i].barno=taktnr-1; //sist i forrige takt
		  printf("left starvolta, taktnr %d\n", taktnr);
		}

	      if (v->tick2() == m->tick() + m->tickLen()) // hvis det er sist i takten.
		{
		  i++;
		  printf("i: %d\n", i);
		  voltarray[i].voltart = endending;
		  voltarray[i].barno=taktnr;//sist i denne takten
		  if (v->subtype() == Volta::VOLTA_CLOSED)
		    {// se comment above.
		      printf("right, Volta closed\n");
		    }
		  else if (v->subtype() == Volta::VOLTA_OPEN)
		    {
		      printf("right, volta open\n");
		    }

		}
	    }
	}// for all elements

      i=voltacheckbar((Measure *) m, i);

    }//for all measures

  lastind=i;

#define TEST
#ifdef TEST

  for (i=0; i<=lastind; i++)
    {
      printf("utskrift av voltarray fra i=0: %d\n", i);
      switch (voltarray[i].voltart)
	{
	case startending:
	  printf(" startending taktnr: %d\n", voltarray[i].barno);
	  break;
	case endending:
	  printf(" endending taktnr: %d\n", voltarray[i].barno);
	  break;
	case startrepeat:
	  printf("i: %d, startrepeat taktnr: %d\n", i, voltarray[i].barno);
	  break;
	case endrepeat:
	  printf("endrepeat taktnr %d\n", voltarray[i].barno);
	  break;
	case bothrepeat:
	  printf("bothrepeat taktnr %d\n", voltarray[i].barno);
	  break;
	case none:
	  printf("none taktnr %d\n",  voltarray[i].barno);
	  break;
	default:
	  printf("other taktnr %d\n", voltarray[i].barno);
	  break;
	}
    }

#endif

}// end findvolta


//---------------------------------------------------------
//   exportLilypond
//---------------------------------------------------------

bool Score::saveLilypond(const QString& name)
      {
      ExportLy em(this);
      return em.write(name);
      }


//---------------------------------------------------------
//   barline : start-, end-, both-repeats, endbar, doublebar.
//   stolen from exportxml
//---------------------------------------------------------

void ExportLy::writeBarline(Measure* m) //always right barline at end-of-measure.
// nearly obsolete: replace with voltachecking
{

  printf("at writebarline.\n");
  int bst = m->endBarLineType();
  switch(bst)
    {
    case NORMAL_BAR:
      printf("normal bar\n");
      break;
    case START_REPEAT:
      printf("start-repeat-bar\n");
      //	os << "\\bar \"|:\"";
      // os << "\\repeat volta 2 { "; done by voltachekcing.
      break;
    case DOUBLE_BAR:
      printf("double bar\n");
      indent();
      os << "\\bar \"||\"";
      break;
    case END_REPEAT:
      printf("endrepeat \n");
      if ((repeatactive==true) or (firstalt==true) or (secondalt==true))
	repeatactive=false; // os << " } %end of volta \n"; done by voltachecking.
      else
	os << "\\bar \":|\"";
      break;
    case BROKEN_BAR:
      printf("brokenbar\n");
      os << "\\bar \"| |:\"";
      break;
    case END_BAR:
      printf("endbar\n");
      os << "\\bar \"|.\"";
      break;
    case END_START_REPEAT:
      printf("both repeats bar\n");
      //	os << "\\bar \":|:\"";
      break;
    default:
      printf("other barline %d\n", bst);
      break;
    }
}


//---------------------------------------------------------
//   writeClef
//---------------------------------------------------------

void ExportLy::writeClef(int clef)
{
  os << "\\clef ";
  switch(clef) {
  case CLEF_G:      os << "treble";       break;
  case CLEF_F:      os << "bass";         break;
  case CLEF_G1:     os << "\"treble^8\""; break;
  case CLEF_G2:     os << "\"treble^15\"";break;
  case CLEF_G3:     os << "\"treble_8\""; break;
  case CLEF_F8:     os << "\"bass_8\"";   break;
  case CLEF_F15:    os << "\"bass_15\"";  break;
  case CLEF_F_B:    os << "bass";         break;
  case CLEF_F_C:    os << "bass";         break;
  case CLEF_C1:     os <<  "soprano";     break;
  case CLEF_C2:     os <<  "mezzo-soprano";break;
  case CLEF_C3:     os <<  "alto";        break;
  case CLEF_C4:     os <<  "tenor";       break;
  case CLEF_TAB:    os <<  "tab";         break;
  case CLEF_PERC:   os <<  "percussion";  break;
  }
  os << " ";
}

//---------------------------------------------------------
//   writeTimeSig
//---------------------------------------------------------

void ExportLy::writeTimeSig(TimeSig* sig)
      {
      int z, n;
      sig->getSig(&n, &z);
      os << "\\time " << z << "/" << n << " ";
      }

//---------------------------------------------------------
//   writeKeySig
//---------------------------------------------------------

void ExportLy::writeKeySig(int st)
      {
      st = char(st & 0xff);
      if (st == 0)
            return;
      os << "\\key ";
      switch(st) {
            case 6:  os << "fis"; break;
            case 5:  os << "h";   break;
            case 4:  os << "e";   break;
            case 3:  os << "a";   break;
            case 2:  os << "d";   break;
            case 1:  os << "g";   break;
            case 0:  os << "c";   break;
            case -7: os << "ces"; break;
            case -6: os << "ges"; break;
            case -5: os << "des"; break;
            case -4: os << "as";  break;
            case -3: os << "es";  break;
            case -2: os << "bes"; break;
            case -1: os << "f";   break;
            default:
                  printf("illegal key %d\n", st);
                  break;
            }
      os << " \\major ";
      }

//---------------------------------------------------------
//   tpc2name
//---------------------------------------------------------

QString ExportLy::tpc2name(int tpc)
      {
      const char names[] = "fcgdaeb";
      int acc   = ((tpc+1) / 7) - 2;
      QString s(names[(tpc + 1) % 7]);
      switch(acc) {
            case -2: s += "eses"; break;
            case -1: s += "es";  break;
            case  1: s += "is";  break;
            case  2: s += "isis"; break;
            case  0: break;
            default: s += "??"; break;
            }
      return s;
      }


//---------------------------------------------------------
//   writeChord
//---------------------------------------------------------

void ExportLy::writeChord(Chord* c)
{
  bool graceslur=false;
  int  purepitch;
  QString purename;


  // only the stem direction of the first chord in a
  // beamed chord group is relevant
  if (c->beam() == 0 || c->beam()->getElements().front() == c)
    {
      Direction d = c->stemDirection();
      if (d != stemDirection) {
	stemDirection = d;
	if (d == UP)
	  os << "\\stemUp ";
	else if (d == DOWN)
	  os << "\\stemDown ";
	else if (d == AUTO)
	  {
	    if (graceswitch == true) os << "] ";
	    os << "\\stemNeutral ";
	  }
      }
    }

  NoteList* nl = c->noteList();

  if (nl->size() > 1)
    os << "<";

  for (iNote i = nl->begin();;)
    {
      Note* n = i->second;
      NoteType gracen;

      gracen = n->noteType();
      /*OLAV: her må vi vel sjekke om noten er en grace*/
      switch(gracen)
	{
	case NOTE_INVALID:
	case NOTE_NORMAL: if (graceswitch==true)
	    {
	      graceswitch=false;
	      graceslur=true;
	      os << " } "; //end of grace
	    }
	  break;
	case NOTE_ACCIACCATURA:
	case NOTE_APPOGGIATURA:
	case NOTE_GRACE4:
	case NOTE_GRACE16:
	case NOTE_GRACE32:
	  if (graceswitch==false)
	    {
	      os << "\\grace{";
	      graceswitch=true;
	    }
	  else
	    if (graceswitch==true)
	      {
		os << "[( ";
	      }
	  break;
	} //end of switch(gracen)

      //her må vi finne ut om vi begynner på en tuplet
      findtuplets(n);

      os << tpc2name(n->tpc());

      purepitch = n->pitch();
      purename = tpc2name(n->tpc());

      if      (purename.contains("eses")==1) purepitch=purepitch+2;
      else if (purename.contains("es")==1) purepitch=purepitch+1;
      else if (purename.contains("isis")==1) purepitch=purepitch-2;
      else if (purename.contains("is")==1) purepitch=purepitch-1;

      oktavdiff=prevpitch - purepitch;

      int oktreit = (numval(oktavdiff) / 12);
      int oktavmod = (numval(oktavdiff) % 12);
      if (oktavmod < 6) oktreit=oktreit-1;

      if (oktavdiff < -5) {os << "'"; }
      else if (oktavdiff > 6)  {os << ",";}


      while (oktreit > 0)
	{
	  --oktreit;
	  if (oktavdiff < -5) {os << "'"; }
	  else if (oktavdiff > 6)  {os << ",";}
	}

      prevpitch=n->pitch();

      if (i == nl->begin()) chordpitch=prevpitch;


      ++i; //antall noter i akkorden: vi går til neste
      if (i == nl->end())
	break;
      os << " ";
    } //end of notelist: ferdig med akkorden

  if (nl->size() > 1)
    os << ">"; //slutt på akkorden

  if (tupletcount==1)
    {
      os << " } ";
      tupletcount=0;
    }

  prevpitch=chordpitch;

  writeLen(c->tickLen());

  if (graceslur==true)
    {
      os << " ) "; //slur skulle vært avslutta etter hovednoten.
      graceslur=false;
    }

  foreach(Articulation* a, *c->getArticulations()) {
    switch(a->subtype()) {
    case UfermataSym:
      os << "\\fermata";
      break;
    case DfermataSym:
      os << "_\\fermata";
      break;
    case ThumbSym:
      os << "\\thumb";
      break;
    case SforzatoaccentSym:
      os << "->";
      break;
    case EspressivoSym:
      os << "\\espressivo";
      break;
    case StaccatoSym:
      os << "-.";
      break;
    case UstaccatissimoSym:
      os << "-|";
      break;
    case DstaccatissimoSym:
      os << "_|";
      break;
    case TenutoSym:
      os << "--";
      break;
    case UportatoSym:
      os << "-_";
      break;
    case DportatoSym:
      os << "__";
      break;
    case UmarcatoSym:
      os << "-^";
      break;
    case DmarcatoSym:
      os << "_^";
      break;
    case OuvertSym:
      os << "\\open";
      break;
    case PlusstopSym:
      os << "-+";
      break;
    case UpbowSym:
      os << "\\upbow";
      break;
    case DownbowSym:
      os << "\\downbow";
      break;
    case ReverseturnSym:
      os << "\\reverseturn";
      break;
    case TurnSym:
      os << "\\turn";
      break;
    case TrillSym:
      os << "\\trill";
      break;
    case PrallSym:
      os << "\\prall";
      break;
    case MordentSym:
      os << "\\mordent";
      break;
    case PrallPrallSym:
      os << "\\prallprall";
      break;
    case PrallMordentSym:
      os << "\\prallmordent";
      break;
    case UpPrallSym:
      os << "\\prallup";
      break;
    case DownPrallSym:
      os << "\\pralldown";
      break;
    case UpMordentSym:
      os << "\\upmordent";
      break;
    case DownMordentSym:
      os << "\\downmordent";
      break;
#if 0 // TODO: this are now Repeat() elements
    case SegnoSym:
      os << "\\segno";
      break;
    case CodaSym:
      os << "\\coda";
      break;
    case VarcodaSym:
      os << "\\varcoda";
      break;
#endif
    default:
      printf("unsupported note attribute %d\n", a->subtype());
      break;
    }
  }

  checkSlur(c->tick(), c->staffIdx() * VOICES + c->voice());

  os << " ";
}// end of writechord


//---------------------------------------------------------
//   getLen
//---------------------------------------------------------

int ExportLy::getLen(int l, int* dots)
//triplets added, lacking functions for tuplets in general
//I don't know any better method than defining each possible
//notelength. How could this be reduced to one simple algorithm?

      {
      int len  = 4;

      if      (l == 6 * division)
	{
	  len  = 1;
	  *dots = 1;
	}
      else if (l == 5 * division)
	{
	  len = 1;
	  *dots = 2;
	}
      else if (l == 4 * division)
            len = 1;
      else if (l == 3 * division) // dotted half
	{
	  len = 2;
	  *dots = 1;
	}
      else if (l == 2 * division)
            len = 2;
      else if (l == division)
            len = 4;
      else if (l == division *3 /2)
	{
	  len=4;
	  *dots=1;
	}
      else if (l == division / 2)
            len = 8;
      else if (l == division*3 /4) //dotted 8th
	{
	  len = 8;
	  *dots=1;
	}
      else if (l == division / 4)
            len = 16;
      else if (l == division / 8)
            len = 32;
      else if (l == division * 3 /8) //dotted 16th.
	{
	  len = 16;
	  *dots = 1;
	}
      else if (l == division / 16)
            len = 64;
      else if (l == division /32)
	    len = 128;
      //triplets, lily uses nominal value surrounded by \times 2/3 {  }
      //so we set len equal to nominal value
      else if (l == division * 4 /3)
	len = 2;
      else if (l == division * 2 /3)
	len = 4;
      else if (l == division /3)
	len = 8;
      else if (l == division /3*2)
	len = 16;
      else if (l == division /3*4)
	len = 32;
      else printf("unsupported len %d (%d,%d)\n", l, l/division, l % division);
      return len;
      }

//---------------------------------------------------------
//   writeLen
//---------------------------------------------------------

void ExportLy::writeLen(int ticks)
      {
      int dots = 0;
      int len = getLen(ticks, &dots);
      if (ticks != curTicks) {
            os << len;
            for (int i = 0; i < dots; ++i)
                  os << ".";
            curTicks = ticks;
            }
      }

//---------------------------------------------------------
//   writeRest
//    type = 0    normal rest
//    type = 1    whole measure rest
//    type = 2    spacer rest
//---------------------------------------------------------

void ExportLy::writeRest(int tick, int l, int type)
      {
      if (type == 1) {
            // write whole measure rest
            int z, n;
            score->sigmap->timesig(tick, z, n);
            os << "R1*" << z << "/" << n;
            curTicks = -1;
            }
      else if (type == 2) {
            os << "s";
            writeLen(l);
            }
      else {
            os << "r";
            writeLen(l);
            }
      os << " ";
      }




//--------------------------------------------------------
//
//   writevolta
//
//--------------------------------------------------------

void ExportLy::writevolta(int measurenumber, int lastind)

{
  bool utgang=false;
  int i=0;

  while ((voltarray[i].barno < measurenumber) and (i<=lastind))
    {
      //find the present measure
      i++;
      printf("i:%d, measurenumber:%d, array.barno:%d, array.voltart: %d\n",i,measurenumber,voltarray[i].barno,voltarray[i].voltart);
    }

  if (measurenumber==voltarray[i].barno)
    {
      while (utgang==false)
	{
	  if (voltarray[i].voltart==startrepeat)  //mscore's startrepeat is lily's startvolta.
	    {
	      printf("startrepeat funnet i array\n");
	      indent();
	      os << "\\repeat volta 2 {";
	      firstalt=false;
	      secondalt=false;
	    }

	  if ((voltarray[i].voltart==endrepeat) and (firstalt==false))
	    {
	      printf("endrepeat funnet i array\n");
	      os << "} % end of reprise\n";
	    }

	  if ((voltarray[i].voltart==bothrepeat) and (firstalt==false))
	    {
	      printf("bothrepeat funnet i array\n");
	      os << "} % end of reprise\n";
	      indent();
	      os << "\\repeat volta 2 {";
	      firstalt=false;
	      secondalt=false;
	    }

	  if (voltarray[i].voltart==startending)
	    {printf("startending funnet i array\n");
	      if (firstalt==false)
		{
		  os << "} % end of reprise except alternate endings\n";
		  indent();
		  os << "\\alternative{ {  ";
		  firstalt=true;
		}
	      else
		{ indent();
		  os << "{ %start alt2\n";
		  indent();
		  firstalt=false;
		  secondalt=true;
		}
	    }


	  if (voltarray[i].voltart==endending)
	    {
	      printf("end-of-ending funnet i array\n");
	      if (firstalt)
		{
		  os << "} %close alt1\n";
		  secondalt=true;
		}
	      else
		{
		  os << "} %close alt2\n";
		  indent();
		  os << "} %close alternatives\n";
		  secondalt=false;
		  firstalt=true;
		}
	    }

	  if (voltarray[i+1].barno==measurenumber)
	    {
	      i++;
	      printf("i++:%d, measurenumber:%d, array.barno:%d, array.voltart: %d\n",
		     i,measurenumber,voltarray[i].barno,voltarray[i].voltart);
	    }
	  else utgang=true;
	}// end of while utgang false;
    }// if barno=measurenumber
}// end writevolta




//---------------------------------------------------------
//   writeMeasure
//---------------------------------------------------------


void ExportLy::writeMeasure(Measure* m, int staffIdx)

{

  bool voiceActive[4];
  int i=0;

  measurenumber=m->no()+1;
  printf("measurenumber: %d\n", measurenumber);

  if (measurenumber==1)
    {
      i=0;
      printf("Vi er i første takt\n");
      //   a)finner startrepeat: avlys søket
      //   b)finner endrepeat: skriv \volta 2{ og sett brytere for leting etter alternativer og endrepeat.

      while ((voltarray[i].voltart != startrepeat) and (voltarray[i].voltart != endrepeat)
	     and (voltarray[i].voltart !=bothrepeat) and (i<=lastind))
	{
	  i++;
	}
      if (i<=lastind)
	{
	  if ((voltarray[i].voltart==endrepeat) or (voltarray[i].voltart==bothrepeat))
	    {
	      printf("starter med volta i første takt\n");
	      indent();
	      os << "\\repeat volta 2 { \n";
	      repeatactive=true;
	    }
	}
    }

  indent();

  for (int voice = 0; voice < VOICES; ++voice)
    {
      voiceActive[voice] = false;

      for(Segment* s = m->first(); s; s = s->next()) {
	Element* e = s->element(staffIdx * VOICES + voice);
	if (e) {
	  voiceActive[voice] = true;
	  break;
	}
      }
    }

  int activeVoices = 0;

  for (int voice = 0; voice < VOICES; ++voice)
    {
      if (voiceActive[voice])
	++activeVoices;
    }

  if (activeVoices > 1)
    {
      indent();
      os << "<<"; //nope, to be done in score-block.
    }

  int nvoices = activeVoices;

  for (int voice = 0; voice < VOICES; ++voice)
    {// for all voices

      if (!voiceActive[voice])
	continue;

      if (activeVoices > 1)
	{
	  indent();
	  os << "{ ";
	}

      int tick = m->tick();

      for(Segment* s = m->first(); s; s = s->next())
	{// for all segments
	  Element* e = s->element(staffIdx * VOICES + voice);
	  if (e == 0 || e->generated())
	    continue;
	  switch(e->type()) {
	  case CLEF:
	    writeClef(e->subtype());
	    break;
	  case TIMESIG:
	    {
	      writeTimeSig((TimeSig*)e);
	      os << "\n\n";
	      indent();
	      break;
	    }
	  case KEYSIG:
	    writeKeySig(e->subtype());
	    break;
	  case CHORD:
	    {
	      if (voice) {
		int ntick = e->tick() - tick;
		if (ntick > 0)
		  writeRest(tick, ntick, 2);
		tick += ntick;
	      }
	      writeChord((Chord*)e);
	      tick += e->tickLen();
	    }
	    break;
	  case REST:
	    {
	      int l = e->tickLen();
	      if (l == 0) {
		l = ((Rest*)e)->segment()->measure()->tickLen();
		writeRest(e->tick(), l, 1);
	      }
	      else
		writeRest(e->tick(), l, 0);
	      tick += l;
	    }
	    break;
	  case BAR_LINE: //We never arrive here!!??
	    printf("barline?????\n");
	    //		writeBarline(m);
	    break;
	  case BREATH:
	    os << "\\breathe ";
	    break;
	  default:
	    printf("Export Lilypond: unsupported element <%s>\n", e->name());
	    break;
	  }
	} //end for all segments

      if (activeVoices > 1)
	{
	  os << "}\n";
	  indent();
	}

      --nvoices;

      if (nvoices == 0)
	break;

      indent();
      os << "\\\\";

    }// end for all voices


  if (activeVoices > 1)
    {
      os << ">>";
      --level;
    }

  writevolta(measurenumber, lastind);

  //hvordan vet jeg at jeg er sist i takten her?
  printf("at end-of-measure: barline\n");
  writeBarline(m);
  os << " | % " << m->no()+1; //barcheck og taktnr

} //end write measure


//---------------------------------------------------------
//   writeScore
//---------------------------------------------------------

void ExportLy::writeScore()
{
  firstalt=false;
  secondalt=false;
  tupletcount=0;


  char  cpartnum;
  const char* relativ;
  QString  partname[32], partshort[32];
  chordpitch=41;
  repeatactive=false;

  int staffIdx = 0;
  int np = score->parts()->size();
  graceswitch=false;

  if (np > 1)
    {

    }

  foreach(Part* part, *score->parts()) {
    int n = part->staves()->size();

    partname[staffIdx] =  part->longName();
    partshort[staffIdx] = part->shortName();

    if (n > 1)
      {
	//OLAV TODO: må lagres for score-block på slutten isteden:
	//	os << "\\new PianoStaff <<\n";
	pianostaff=true;
	//	++level;
	//	indent();
      }

    foreach(Staff* staff, *part->staves())
      {

	os << "\n";

	switch(staff->clef(0)) {
	case CLEF_G:
	  relativ="'";
	  prevpitch=12*5;
	  break;
	case CLEF_TAB:
	case CLEF_PERC:
	case CLEF_G3:
	case CLEF_F:
	  relativ="";
	  prevpitch=12*4;
	  break;
	case CLEF_G1:
	case CLEF_G2:
	  relativ="''";
	  prevpitch=12*6;
	  break;
	case CLEF_F_B:
	case CLEF_F_C:
	case CLEF_F8:
	  relativ=",";
	  prevpitch=12*3;
	  break;
	case CLEF_F15:
	  relativ=",,";
	  prevpitch=12*2;
	  break;
	case CLEF_C1:
	case CLEF_C2:
	case CLEF_C3:
	case CLEF_C4:
	  relativ="'";
	  prevpitch=12*5;
	  break;
	}

	cpartnum = staffIdx + 65;
	voiceid[staffIdx] = partshort[staffIdx];
	voiceid[staffIdx].append(cpartnum);
	voiceid[staffIdx].prepend("A");
	voiceid[staffIdx].remove(QRegExp("[0-9]"));
	voiceid[staffIdx].remove(QChar('.'));
	voiceid[staffIdx].remove(QChar(' '));

	os << voiceid[staffIdx]  << " = \\relative c" << relativ <<  "\n";
	os << "{\n";

	++level;
	indent();


	writeClef(staff->clef(0));
	writeKeySig(staff->keymap()->key(0));
	// ^^^^ done also in write measure,
	// -- because there can be new keysigs unterwegs?
	// But it makes output file look dirty.

	findVolta();
	/*find all voltas and repeat-barlines prior to writing anything.
	  If first repeat sign is end-repeat, there is an "implicit" start-repeat sign at the
	  beginning of the first bar, and we have to insert "\volta 2" { before we start writing
	  any content.*/

	for (MeasureBase* m = score->layout()->first(); m; m = m->next())
	  {
	    if (m->type() != MEASURE)
	      continue;

	    os << "\n";

	    writeMeasure((Measure*)m, staffIdx);
	  }


	os << "\n";

	level=0;
	indent();
	os << "}%etter siste takt\n";

	++staffIdx;
      }
    voiceid[staffIdx]="laststaff";
    if (n > 1) {
      --level;
      indent();
      //      os << "laststaff\n";
    }
  }
}

//---------------------------------------------------------
//   write
//---------------------------------------------------------

bool ExportLy::write(const QString& name)
      {
	pianostaff=false;
      f.setFileName(name);
      if (!f.open(QIODevice::WriteOnly))
            return false;
      os.setDevice(&f);
      os << "%=============================================\n"
            "%   created by MuseScore Version: " << VERSION << "\n"
            "%=============================================\n"
            "\n"
            "\\version \"2.10.5\"\n\n";     // target lilypond version

      //---------------------------------------------------
      //    Page format
      //---------------------------------------------------

      PageFormat* pf = score->pageFormat();
      os << "#(set-default-paper-size ";
      switch(pf->size) {
            default:
            case  0: os << "\"a4\""; break;
            case  2: os << "\"letter\""; break;
            case  8: os << "\"a3\""; break;
            case  9: os << "\"a5\""; break;
            case 10: os << "\"a6\""; break;
            case 29: os << "\"11x17\""; break;
            }

      if (pf->landscape) os << " 'landscape";

      os << ")\n\n";

      double lw = pf->width() - pf->evenLeftMargin - pf->evenRightMargin;
      os << "\\paper {\n"
            "  line-width    = " << lw * INCH << "\\mm\n"
            "  left-margin   = " << pf->evenLeftMargin * INCH << "\\mm\n"
            "  top-margin    = " << pf->evenTopMargin * INCH << "\\mm\n"
            "  bottom-margin = " << pf->evenBottomMargin * INCH << "\\mm\n"
            "  }\n\n";

      //---------------------------------------------------
      //    score
      //---------------------------------------------------

      os << "\\header {\n";

     ++level;
      const MeasureBase* m = score->layout()->first();
      foreach(const Element* e, *m->el()) {
            if (e->type() != TEXT)
                  continue;
            QString s = ((Text*)e)->getText();
	    indent();
            switch(e->subtype()) {
                  case TEXT_TITLE:
                        os << "title = ";
                        break;
                  case TEXT_SUBTITLE:
                        os << "subtitle = ";
                        break;
                  case TEXT_COMPOSER:
                        os << "composer = ";
                        break;
                  case TEXT_POET:
                        os << "poet = ";
                        break;
                  default:
                        printf("text-type %d not supported\n", e->subtype());
                        os << "subtitle = ";
                        break;
                  }
            os << "\"" << s << "\"\n";
            }
      indent();
      os << "}\n";


      writeScore();

      /*-------------olav:--------------*/
      // her må det lages en funksjon som produserer en egen
      // score-block ut fra data vi har samlet før:
      os << "\n\\score { \\relative << \n";

      if (pianostaff)
	{
	  os << "\\context PianoStaff <<";
	}

      indx=0;
      ++level;
      indent();
      while (voiceid[indx]!="laststaff")
	{
	  os << "\\context Staff = O" << voiceid[indx] << "G" << "  << \n";
	  ++level;
	  indent();
	  os << "\\context Voice = O" << voiceid[indx] << "G \\" << voiceid[indx] << "\n";
	  --level;
	  indent();
	  os << ">>\n\n";
	  ++indx;
	}

      if (pianostaff)
	{
	  --level;
	  indent();
	  os << ">>\n";
	  pianostaff=false;
	}

      --level;
      indent();
      os << ">>\n";
      --level;
      indent();
      os <<"}\n";

      f.close();
      return f.error() == QFile::NoError;
      }



//---------------------------------------------------------
//   checkSlur
//---------------------------------------------------------

void ExportLy::checkSlur(int tick, int track)
{
  for (const MeasureBase* mb = score->layout()->first(); mb; mb = mb->next()) {
    if (mb->type() != MEASURE)
      continue;
    Measure* m = (Measure*) mb;
    foreach(Element* e, *m->el()) {
      if (e->type() != SLUR)
	{
	  continue;
	}
      Slur* s = (Slur*)e;
      if ((s->tick() == tick) && (s->track() == track))
	{
	  os << "(";
	  printf("startSlur %d-%d  %d-%d\n", tick, track, s->tick2(), s->track2());
	}
      if ((s->tick2() == tick) && (s->track2() == track))
	{
	  os << ")";
	  printf("endSlur %d-%d  %d-%d\n", s->tick(), s->track(), tick, track);
	}
    }
  }
}



/* NEW 10.oct.2008:
- voltas and endings
- dotted 8ths and 4ths. Problem: Do I have to calculate each and every notelength:
  is it not possible to find a general algorithm?
- triplets, but not general tuplets. Same problem as in previous point.
- PianoStaff reactivated.*/


/*NEW:
1. dotted 16th notes
2. Relative pitches
3. More grace-note types
4. Slurs and beams in grace-notes
5. Some adjustments in articulations
6. separation of staffs/voices from score-block. Unfinished for pianostaffs/GrandStaffs and voices on one staff.
7. completed the clef-secton.
Points 2 and 6, and also in a smaller degree 5, enhances human readability of lilypond-code.
*/

/* TODO: projects
1. Piano staffs/GrandStaffs, system brackets and braces.
2. Slurs: existing slurcheck does not work. Have to look at exportxml.cpp to find a way.
3. General tuplets
4. Segno etc.                      -----"-------
5. complete separation of voices on the same staff, recombination in score-block. requires more
   extensive re-writing, evt. buffering of other than first voice for later export to outputstream.
   Present use of local polyphony causes to many crashes between voices. And does not permit use of
   partcombine.
6. Clean up now nearly obsolete writeBarlines();
7.

*/

/*TODO: bugs
1. octave-shift problems. Reason: calculates distance in scale half-steps, should
   have calculated graphical position on staff.
2. \stemUp \stemDown : sometimes correct sometimes not??? Maybe I have not understood
   Lily's rules for the use of these commands?
   Lily's own stem direction algorithms are good enough. Until a better idea comes
   along: drop export of stem direction and leave it to LilyPond.
*/
