//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2011 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#include "scoreview.h"
#include "musescore.h"
#include "libmscore/undo.h"

#include "libmscore/score.h"
#include "libmscore/element.h"
#include "seq.h"
#include "libmscore/mscore.h"

#include "articulationprop.h"
#include "bendproperties.h"
#include "boxproperties.h"
#include "tupletproperties.h"
#include "voltaproperties.h"
#include "lineproperties.h"
#include "tremolobarprop.h"
#include "timesigproperties.h"
#include "textproperties.h"
#include "tempoproperties.h"
#include "imageproperties.h"
#include "dynamicprop.h"
#include "hairpinprop.h"
#include "sectionbreakprop.h"
#include "stafftextproperties.h"
#include "slurproperties.h"
#include "glissandoproperties.h"
#include "fretproperties.h"
#include "markerproperties.h"
#include "chordproperties.h"
#include "restproperties.h"
#include "jumpproperties.h"
#include "selinstrument.h"
#include "chordedit.h"
#include "chordeditor.h"
#include "editstyle.h"
#include "editstaff.h"

#include "libmscore/staff.h"
#include "libmscore/segment.h"
#include "libmscore/bend.h"
#include "libmscore/box.h"
#include "libmscore/text.h"
#include "libmscore/articulation.h"
#include "libmscore/tuplet.h"
#include "libmscore/volta.h"
#include "libmscore/tremolobar.h"
#include "libmscore/timesig.h"
#include "libmscore/accidental.h"
#include "libmscore/clef.h"
#include "libmscore/dynamic.h"
#include "libmscore/tempotext.h"
#include "libmscore/keysig.h"
#include "libmscore/stafftext.h"
#include "libmscore/staffstate.h"
#include "libmscore/note.h"
#include "libmscore/layoutbreak.h"
#include "libmscore/image.h"
#include "libmscore/hairpin.h"
#include "libmscore/chord.h"
#include "libmscore/rest.h"
#include "libmscore/harmony.h"
#include "libmscore/glissando.h"
#include "libmscore/fret.h"
#include "libmscore/instrchange.h"
#include "libmscore/slur.h"
#include "libmscore/repeat.h"

//---------------------------------------------------------
//   genPropertyMenu1
//---------------------------------------------------------

void ScoreView::genPropertyMenu1(Element* e, QMenu* popup)
      {
      if ((!e->generated() || e->type() == BAR_LINE) && (e->type() != LAYOUT_BREAK)) {
            if (e->visible())
                  popup->addAction(tr("Set Invisible"))->setData("invisible");
            else
                  popup->addAction(tr("Set Visible"))->setData("invisible");
            popup->addAction(tr("Color..."))->setData("color");
            if (e->flag(ELEMENT_HAS_TAG)) {
                  popup->addSeparator();

                  QMenu* menuLayer = new QMenu(tr("Layer"));
                  for (int i = 0; i < MAX_TAGS; ++i) {
                        QString tagName = score()->layerTags()[i];
                        if (!tagName.isEmpty()) {
                              QAction* a = menuLayer->addAction(tagName);
                              a->setData(QString("layer-%1").arg(i));
                              a->setCheckable(true);
                              a->setChecked(e->tag() & (1 << i));
                              }
                        }
                  popup->addMenu(menuLayer);
                  }
            }
      }

//---------------------------------------------------------
//   genPropertyMenuText
//---------------------------------------------------------

void ScoreView::genPropertyMenuText(Element* e, QMenu* popup)
      {
      if (e->visible())
            popup->addAction(tr("Set Invisible"))->setData("invisible");
      else
            popup->addAction(tr("Set Visible"))->setData("invisible");
      if (e->flag(ELEMENT_HAS_TAG)) {
            popup->addSeparator();

            QMenu* menuLayer = new QMenu(tr("Layer"));
            for (int i = 0; i < MAX_TAGS; ++i) {
                  QString tagName = score()->layerTags()[i];
                  if (!tagName.isEmpty()) {
                        QAction* a = menuLayer->addAction(tagName);
                        a->setData(QString("layer-%1").arg(i));
                        a->setCheckable(true);
                        a->setChecked(e->tag() & (1 << i));
                        }
                  }
            popup->addMenu(menuLayer);
            }
      popup->addAction(tr("Text Properties..."))->setData("text-props");
      }

//---------------------------------------------------------
//   createElementPropertyMenu
//---------------------------------------------------------

void ScoreView::createElementPropertyMenu(Element* e, QMenu* popup)
      {
      if (e->type() == BAR_LINE) {
            genPropertyMenu1(e, popup);
            }
      else if (e->type() == ARTICULATION) {
            genPropertyMenu1(e, popup);
            popup->addAction(tr("Articulation Properties..."))->setData("a-props");
            }
      else if (e->type() == BEND) {
            genPropertyMenu1(e, popup);
            popup->addAction(tr("Bend Properties..."))->setData("b-props");
            }
      else if (e->type() == TREMOLOBAR) {
            genPropertyMenu1(e, popup);
            popup->addAction(tr("TremoloBar Properties..."))->setData("tr-props");
            }
      else if (e->type() == HBOX) {
            QMenu* textMenu = popup->addMenu(tr("Add"));
            textMenu->addAction(getAction("frame-text"));
            textMenu->addAction(getAction("picture"));
            popup->addAction(tr("Frame Properties..."))->setData("f-props");
            }
      else if (e->type() == VBOX) {
            QMenu* textMenu = popup->addMenu(tr("Add"));
            textMenu->addAction(getAction("frame-text"));
            textMenu->addAction(getAction("title-text"));
            textMenu->addAction(getAction("subtitle-text"));
            textMenu->addAction(getAction("composer-text"));
            textMenu->addAction(getAction("poet-text"));
            textMenu->addAction(getAction("insert-hbox"));
            textMenu->addAction(getAction("picture"));
            popup->addAction(tr("Frame Properties..."))->setData("f-props");
            }
      else if (e->type() == TBOX) {
            popup->addAction(tr("Frame Properties..."))->setData("f-props");
            }
      else if (e->type() == TUPLET) {
            genPropertyMenu1(e, popup);
            popup->addAction(tr("Tuplet Properties..."))->setData("tuplet-props");
            }
      else if (e->type() == VOLTA_SEGMENT) {
            genPropertyMenu1(e, popup);
            popup->addAction(tr("Volta Properties..."))->setData("v-props");
            }
      else if (e->type() == TIMESIG) {
            genPropertyMenu1(e, popup);
            TimeSig* ts = static_cast<TimeSig*>(e);
            int _track = ts->track();
            // if the time sig. is not generated (= not courtesy) and is in track 0
            // add the specific menu item
            QAction* a;
            if (!ts->generated() && !_track) {
                  a = popup->addAction(ts->showCourtesySig()
                     ? QT_TRANSLATE_NOOP("TimeSig", "Hide Courtesy Time Signature")
                     : QT_TRANSLATE_NOOP("TimeSig", "Show Courtesy Time Signature") );
                  a->setData("ts-courtesy");
                  }
            popup->addSeparator();
            popup->addAction(tr("Time Signature Properties..."))->setData("ts-props");
            }
      else if (e->type() == ACCIDENTAL) {
            Accidental* acc = static_cast<Accidental*>(e);
            genPropertyMenu1(e, popup);
            QAction* a = popup->addAction(QT_TRANSLATE_NOOP("Accidental", "small"));
            a->setCheckable(true);
            a->setChecked(acc->small());
            a->setData("small");
            }
      else if (e->type() == CLEF) {
            genPropertyMenu1(e, popup);
            // if the clef is not generated (= not courtesy) add the specific menu item
            if (!e->generated()) {
                  QAction* a = popup->addAction(static_cast<Clef*>(e)->showCourtesyClef()
                     ? QT_TRANSLATE_NOOP("Clef", "Hide courtesy clef")
                     : QT_TRANSLATE_NOOP("Clef", "Show courtesy clef") );
                        a->setData("clef-courtesy");
                  }
            }
      else if (e->type() == DYNAMIC) {
            popup->addSeparator();
            if (e->visible())
                  popup->addAction(tr("Set Invisible"))->setData("invisible");
            else
                  popup->addAction(tr("Set Visible"))->setData("invisible");
            popup->addAction(tr("MIDI Properties..."))->setData("d-dynamics");
            popup->addAction(tr("Text Properties..."))->setData("d-props");
            }
      else if (e->type() == TEXTLINE_SEGMENT || e->type() == OTTAVA_SEGMENT) {
            if (e->visible())
                  popup->addAction(tr("Set Invisible"))->setData("invisible");
            else
                  popup->addAction(tr("Set Visible"))->setData("invisible");
            popup->addAction(tr("Line Properties..."))->setData("l-props");
            }
      else if (e->type() == STAFF_TEXT) {
            genPropertyMenuText(e, popup);
            popup->addAction(tr("Staff Text Properties..."))->setData("st-props");
            }
      else if (e->type() == TEXT || e->type() == FINGERING) {
            genPropertyMenuText(e, popup);
            }
      else if (e->type() == TEMPO_TEXT) {
            genPropertyMenu1(e, popup);
            popup->addAction(tr("Tempo Properties..."))->setData("tempo-props");
            popup->addAction(tr("Text Properties..."))->setData("text-props");
            }
      else if (e->type() == KEYSIG) {
            genPropertyMenu1(e, popup);
            KeySig* ks = static_cast<KeySig*>(e);
            if (!e->generated()) {
                  QAction* a = popup->addAction(ks->showCourtesySig()
                     ? QT_TRANSLATE_NOOP("KeySig", "Hide Courtesy Key Signature")
                     : QT_TRANSLATE_NOOP("KeySig", "Show Courtesy Key Signature") );
                  a->setData("key-courtesy");
                  a = popup->addAction(ks->showNaturals()
                     ? QT_TRANSLATE_NOOP("KeySig", "Hide Naturals")
                     : QT_TRANSLATE_NOOP("KeySig", "Show Naturals") );
                  a->setData("key-naturals");
                  }
            }
      else if (e->type() == STAFF_STATE && e->subtype() == STAFF_STATE_INSTRUMENT) {
            popup->addAction(tr("Change Instrument Properties..."))->setData("ss-props");
            }
      else if (e->type() == SLUR_SEGMENT) {
            genPropertyMenu1(e, popup);
            popup->addAction(tr("Edit Mode"))->setData("edit");
            popup->addAction(tr("Slur Properties..."))->setData("slur-props");
            }
      else if (e->type() == REST) {
            Rest* rest = static_cast<Rest*>(e);
            genPropertyMenu1(e, popup);
            popup->addSeparator();
            if (rest->tuplet()) {
                  QMenu* menuTuplet = popup->addMenu(tr("Tuplet..."));
                  menuTuplet->addAction(tr("Tuplet Properties..."))->setData("tupletProps");
                  menuTuplet->addAction(tr("Delete Tuplet"))->setData("tupletDelete");
                  }
            popup->addAction(tr("Rest Properties..."))->setData("rest-props");
            }
      else if (e->type() == NOTE) {
            Note* note = static_cast<Note*>(e);
            genPropertyMenu1(e, popup);
            popup->addSeparator();
            popup->addAction(tr("Note Properties..."))->setData("note-props");

            popup->addAction(tr("Style..."))->setData("style");

            if (note->chord()->tuplet()) {
                  QMenu* menuTuplet = popup->addMenu(tr("Tuplet..."));
                  menuTuplet->addAction(tr("Tuplet Properties..."))->setData("tupletProps");
                  menuTuplet->addAction(tr("Delete Tuplet"))->setData("tupletDelete");
                  }
            popup->addAction(tr("Chord Articulation..."))->setData("articulation");
            }
      else if (e->type() == MARKER) {
            genPropertyMenu1(e, popup);
            popup->addAction(tr("Marker Properties..."))->setData("marker-props");
            }
      else if (e->type() == JUMP) {
            genPropertyMenu1(e, popup);
            popup->addAction(tr("Jump Properties..."))->setData("jump-props");
            }
      else if (e->type() == LAYOUT_BREAK && e->subtype() == LAYOUT_BREAK_SECTION) {
            popup->addAction(tr("Section Break Properties..."))->setData("break-props");
            }
      else if (e->type() == INSTRUMENT_CHANGE) {
            genPropertyMenu1(e, popup);
            popup->addAction(tr("Change Instrument..."))->setData("ch-instr");
            }
      else if (e->type() == FRET_DIAGRAM) {
            if (e->visible())
                  popup->addAction(tr("Set Invisible"))->setData("invisible");
            else
                  popup->addAction(tr("Set Visible"))->setData("invisible");
            popup->addAction(tr("Color..."))->setData("color");
            popup->addAction(tr("Fret Diagram Properties..."))->setData("fret-props");
            }
      else if (e->type() == GLISSANDO) {
            genPropertyMenu1(e, popup);
            popup->addAction(tr("Glissando Properties..."))->setData("gliss-props");
            }
      else if (e->type() == HAIRPIN_SEGMENT) {
            QAction* a = popup->addSeparator();
            a->setText(tr("Dynamics"));
            if (e->visible())
                  a = popup->addAction(tr("Set Invisible"));
            else
                  a = popup->addAction(tr("Set Visible"));
            a->setData("invisible");
            popup->addAction(tr("Hairpin Properties..."))->setData("hp-props");
            }
      else if (e->type() == HARMONY) {
            genPropertyMenu1(e, popup);
            popup->addSeparator();
            popup->addAction(tr("Harmony Properties..."))->setData("ha-props");
            popup->addAction(tr("Text Properties..."))->setData("text-props");
            }
      else if (e->type() == IMAGE) {
            genPropertyMenu1(e, popup);
            QAction* a = popup->addSeparator();
            a->setText(tr("Image"));
            popup->addAction(tr("Image Properties..."))->setData("img-props");
            }
      else if (e->type() == INSTRUMENT_NAME) {
            popup->addAction(tr("Staff Properties..."))->setData("staff-props");
            }
      }

//---------------------------------------------------------
//   elementPropertyAction
//---------------------------------------------------------

void ScoreView::elementPropertyAction(const QString& cmd, Element* e)
      {
      if (cmd == "a-props") {
            ArticulationProperties rp(static_cast<Articulation*>(e));
            rp.exec();
            }
      else if (cmd == "b-props") {
            Bend* bend = static_cast<Bend*>(e);
            BendProperties bp(bend, 0);
            if (bp.exec())
                  score()->undo()->push(new ChangeBend(bend, bp.points()));
            }
      else if (cmd == "f-props") {
            BoxProperties vp(static_cast<Box*>(e), 0);
            vp.exec();
            }
      else if (cmd == "frame-text") {
            Text* s = new Text(score());
            s->setSubtype(TEXT_FRAME);
            s->setTextStyle(TEXT_STYLE_FRAME);
            s->setParent(e);
            score()->undoAddElement(s);
            score()->select(s, SELECT_SINGLE, 0);
            startEdit(s);
            score()->setLayoutAll(true);
            }
      else if (cmd == "picture") {
            mscore->addImage(score(), static_cast<HBox*>(e));
            }
      else if (cmd == "frame-text") {
            Text* t = new Text(score());
            t->setSubtype(TEXT_FRAME);
            t->setTextStyle(TEXT_STYLE_FRAME);
            t->setParent(e);
            score()->undoAddElement(t);
            score()->select(t, SELECT_SINGLE, 0);
            startEdit(t);
            }
      else if (cmd == "title-text") {
            Text* t = new Text(score());
            t->setSubtype(TEXT_TITLE);
            t->setTextStyle(TEXT_STYLE_TITLE);
            t->setParent(e);
            score()->undoAddElement(t);
            score()->select(t, SELECT_SINGLE, 0);
            startEdit(t);
            }
      else if (cmd == "subtitle-text") {
            Text* t = new Text(score());
            t->setSubtype(TEXT_SUBTITLE);
            t->setTextStyle(TEXT_STYLE_SUBTITLE);
            t->setParent(e);
            score()->undoAddElement(t);
            score()->select(t, SELECT_SINGLE, 0);
            startEdit(t);
            }
      else if (cmd == "composer-text") {
            Text* t = new Text(score());
            t->setSubtype(TEXT_COMPOSER);
            t->setTextStyle(TEXT_STYLE_COMPOSER);
            t->setParent(e);
            score()->undoAddElement(t);
            score()->select(t, SELECT_SINGLE, 0);
            startEdit(t);
            }
      else if (cmd == "poet-text") {
            Text* t = new Text(score());
            t->setSubtype(TEXT_POET);
            t->setTextStyle(TEXT_STYLE_POET);
            t->setParent(e);
            score()->undoAddElement(t);
            score()->select(t, SELECT_SINGLE, 0);
            startEdit(t);
            }
      else if (cmd == "insert-hbox") {
            HBox* s = new HBox(score());
            double w = width() - s->leftMargin() * DPMM - s->rightMargin() * DPMM;
            s->setBoxWidth(Spatium(w / s->spatium()));
            s->setParent(e);
            score()->undoAddElement(s);
            score()->select(s, SELECT_SINGLE, 0);
            startEdit(s);
            }
      else if (cmd == "picture")
            mscore->addImage(score(), e);
      else if (cmd == "tuplet-props") {
            TupletProperties vp(static_cast<Tuplet*>(e));
            if (vp.exec()) {
                  //
                  // apply changes to all selected tuplets
                  //
                  int bracketType = vp.bracketType();
                  int numberType  = vp.numberType();
                  foreach(Element* e, score()->selection().elements()) {
                        if (e->type() == TUPLET) {
                              Tuplet* tuplet = static_cast<Tuplet*>(e);
                              if ((bracketType != tuplet->bracketType()) || (numberType != tuplet->numberType()))
                                    score()->undo()->push(new ChangeTupletProperties(tuplet, numberType, bracketType));
                              }
                        }
                  }
            }
      else if (cmd == "v-props") {
            VoltaSegment* vs = static_cast<VoltaSegment*>(e);
            VoltaProperties vp;
            vp.setText(vs->volta()->text());
            vp.setEndings(vs->volta()->endings());
            int rv = vp.exec();
            if (rv) {
                  QString txt  = vp.getText();
                  QList<int> l = vp.getEndings();
                  if (txt != vs->volta()->text())
                        score()->undoChangeVoltaText(vs->volta(), txt);
                  if (l != vs->volta()->endings())
                        score()->undoChangeVoltaEnding(vs->volta(), l);
                  }
            }
      else if (cmd == "l-props") {
            TextLineSegment* vs = static_cast<TextLineSegment*>(e);
            TextLine* nTl  = vs->textLine()->clone();
            LineProperties lp(nTl);
            if (lp.exec()) {
                  score()->undoChangeElement(vs->textLine(), nTl);
                  // force new text
                  foreach(SpannerSegment* l, nTl->spannerSegments())
                        static_cast<TextLineSegment*>(l)->clearText();
                  }
            else
                  delete nTl;
            }
      else if (cmd == "tr-props") {
            TremoloBar* tb = static_cast<TremoloBar*>(e);
            TremoloBarProperties bp(tb, 0);
            if (bp.exec())
                  score()->undo()->push(new ChangeTremoloBar(tb, bp.points()));
            }
      if (cmd == "ts-courtesy") {
            TimeSig* ts = static_cast<TimeSig*>(e);
            score()->undo()->push(new ChangeTimesig(static_cast<TimeSig*>(e),
               !ts->showCourtesySig(), ts->sig(), ts->stretch(), ts->subtype(),
               ts->zText(), ts->nText()));
            }
      else if (cmd == "ts-props") {
            TimeSig* ts = static_cast<TimeSig*>(e);
            TimeSig r(*ts);
            TimeSigProperties vp(&r);
            int rv = vp.exec();
            if (rv) {
                  bool stretchChanged = r.stretch() != ts->stretch();
                  if (r.zText() != ts->zText() || r.nText() != ts->nText() || r.sig() != ts->sig() || stretchChanged || r.subtype() != ts->subtype()) {
                        score()->undo()->push(new ChangeTimesig(ts,
                           r.showCourtesySig(), r.sig(), r.stretch(), r.subtype(), r.zText(), r.nText()));
                        if (stretchChanged)
                              score()->timesigStretchChanged(ts, ts->measure(), ts->staffIdx());
                        }
                  }
            }
      else if (cmd == "small") {
            score()->undo()->push(
               new ChangeProperty(e, P_SMALL, !static_cast<Accidental*>(e)->small())
               );
            }
      else if (cmd == "clef-courtesy") {
            bool show = !static_cast<Clef*>(e)->showCourtesyClef();
            score()->undo()->push(new ChangeProperty(e, P_SHOW_COURTESY, show));
            }
      else if (cmd == "d-props") {
            Dynamic* dynamic = static_cast<Dynamic*>(e);
            Dynamic* nText = new Dynamic(*dynamic);
            TextProperties tp(nText, 0);
            int rv = tp.exec();
            if (rv)
                  score()->undoChangeElement(dynamic, nText);
            else
                  delete nText;
            }
      else if (cmd == "st-props") {
            StaffTextProperties rp(static_cast<StaffText*>(e));
            rp.exec();
            }
      else if (cmd == "d-dynamics") {
            Dynamic* dynamic = static_cast<Dynamic*>(e);
            int oldVelo    = dynamic->velocity();
            DynamicType ot = dynamic->dynType();
            DynamicProperties dp(dynamic);
            int rv = dp.exec();
            if (rv) {
                  int newVelo    = dynamic->velocity();
                  DynamicType nt = dynamic->dynType();
                  dynamic->setVelocity(oldVelo);
                  dynamic->setDynType(ot);
                  score()->undoChangeDynamic(dynamic, newVelo, nt);
                  }
            }
      else if (cmd == "text-props") {
            Text* ot    = static_cast<Text*>(e);
            Text* nText = static_cast<Text*>(ot->clone());
            TextProperties tp(nText);
            int rv = tp.exec();
            if (rv) {
                  QList<Element*> sl = score()->selection().elements();
                  QList<Element*> selectedElements;
                  foreach(Element* e, sl) {
                        if ((e->type() != ot->type()) || (e->subtype() != ot->subtype()))
                              continue;
                        Text* t  = static_cast<Text*>(e);
                        Text* tt = t->clone();

                        if (nText->styled() != ot->styled())
                              tt->setStyled(nText->styled());

                        if (nText->textStyle() != ot->textStyle()) {
                              tt->setTextStyle(nText->textStyle());
                              tt->styleChanged();
                              }

                        if (!nText->styled()) {
                              if (nText->hasFrame() != ot->hasFrame())
                                    tt->setHasFrame(nText->hasFrame());
                              if (nText->frameWidth() != ot->frameWidth())
                                    tt->setFrameWidth(nText->frameWidth());
                              if (nText->paddingWidth() != ot->paddingWidth())
                                    tt->setPaddingWidth(nText->paddingWidth());
                              if (nText->frameColor() != ot->frameColor())
                                    tt->setFrameColor(nText->frameColor());
                              if (nText->frameRound() != ot->frameRound())
                                    tt->setFrameRound(nText->frameRound());
                              if (nText->circle() != ot->circle())
                                    tt->setCircle(nText->circle());
                              if (nText->style().foregroundColor() != ot->style().foregroundColor()) {
                                    tt->localStyle().setForegroundColor(nText->style().foregroundColor());
                                    }

                              if (nText->localStyle().family() != ot->localStyle().family())
                                    tt->localStyle().setFamily(nText->localStyle().family());
                              if (nText->localStyle().size() != ot->localStyle().size())
                                    tt->localStyle().setSize(nText->localStyle().size());
                              if (nText->localStyle().bold() != ot->localStyle().bold())
                                    tt->localStyle().setBold(nText->localStyle().bold());
                              if (nText->localStyle().italic() != ot->localStyle().italic())
                                    tt->localStyle().setItalic(nText->localStyle().italic());
                              if (nText->localStyle().underline() != ot->localStyle().underline())
                                    tt->localStyle().setUnderline(nText->localStyle().underline());

                              if (nText->align() != ot->align())
                                    tt->setAlign(nText->align());
                              if (nText->xoff() != ot->xoff())
                                    tt->setXoff(nText->xoff());
                              if (nText->yoff() != ot->yoff())
                                    tt->setYoff(nText->yoff());
                              if (nText->reloff() != ot->reloff())
                                    tt->setReloff(nText->reloff());
                              if (nText->offsetType() != ot->offsetType())
                                    tt->setOffsetType(nText->offsetType());
                              if(nText->sizeIsSpatiumDependent() != ot->sizeIsSpatiumDependent())
                                    tt->setSizeIsSpatiumDependent(nText->sizeIsSpatiumDependent());
                              }

                        tt->doc()->setModified(true);
                        if (t->selected())
                              selectedElements.append(tt);
                        score()->undoChangeElement(t, tt);
                        }
                  score()->select(0, SELECT_SINGLE, 0);
                  foreach(Element* e, selectedElements)
                        score()->select(e, SELECT_ADD, 0);
                  }
            delete nText;
            }
      else if (cmd == "tempo-props") {
            TempoProperties rp(static_cast<TempoText*>(e));
            rp.exec();
            }
      else if (cmd == "key-courtesy") {
            KeySig* ks = static_cast<KeySig*>(e);
            score()->undo()->push(new ChangeKeySig(ks, ks->keySigEvent(), !ks->showCourtesySig(), ks->showNaturals()));
            }
      else if (cmd == "key-naturals") {
            KeySig* ks = static_cast<KeySig*>(e);
            score()->undo()->push(new ChangeKeySig(ks, ks->keySigEvent(), ks->showCourtesySig(), !ks->showNaturals()));
            }
      else if (cmd == "ss-props") {
            StaffState* ss = static_cast<StaffState*>(e);
            SelectInstrument si(ss->instrument(), 0);
            if (si.exec()) {
                  const InstrumentTemplate* it = si.instrTemplate();
                  if (it) {
                        // TODO: undo/redo
                        ss->setInstrument(Instrument::fromTemplate(it));
                        ss->staff()->part()->setInstrument(ss->instrument(), ss->segment()->tick());
                        score()->rebuildMidiMapping();
                        seq->initInstruments();
                        score()->setLayoutAll(true);
                        }
                  else
                        printf("no template selected?\n");
                  }
            }
      else if (cmd == "slur-props") {
            SlurSegment* ss = static_cast<SlurSegment*>(e);
            SlurProperties sp(0);
            sp.setLineType(ss->slurTie()->lineType());
            int rv = sp.exec();
            if (rv) {
                  int lt = sp.getLineType();
                  if (lt != ss->slurTie()->lineType()) {
                        score()->undo()->push(new ChangeProperty(ss->slurTie(), P_LINE_TYPE, lt));
                        }
                  }
            }
      else if (cmd == "rest-props") {
            Rest* orest = static_cast<Rest*>(e);
            Rest r(*orest);
            RestProperties vp(&r);
            int rv = vp.exec();
            if (rv) {
                  bool sizeChanged  = r.small() != orest->small();
                  bool spaceChanged = r.extraLeadingSpace() != orest->extraLeadingSpace()
                     || r.extraTrailingSpace() != orest->extraTrailingSpace();

                  foreach(Element* e, score()->selection().elements()) {
                        if (e->type() != REST)
                              continue;
                        Rest* rest = static_cast<Rest*>(e);

                        if (sizeChanged)
                              score()->undoChangeChordRestSize(rest, r.small());

                        if (spaceChanged) {
                              score()->undoChangeChordRestSpace(rest, r.extraLeadingSpace(),
                              r.extraTrailingSpace());
                              }
                        }
                  }
            }
      else if (cmd == "tupletProps") {
            Tuplet* ot = static_cast<ChordRest*>(e)->tuplet();
            TupletProperties vp(ot);
            if (vp.exec()) {
                  int bracketType = vp.bracketType();
                  int numberType  = vp.numberType();

                  foreach(Element* e, score()->selection().elements()) {
                        if (e->type() == REST) {
                              Rest* r = static_cast<Rest*>(e);
                              if (r->tuplet()) {
                                    Tuplet* tuplet = r->tuplet();
                                    if ((bracketType != tuplet->bracketType()) || (numberType != tuplet->numberType()))
                                          score()->undo()->push(new ChangeTupletProperties(tuplet, numberType, bracketType));
                                    }
                              }
                        }
                  }
            }
      else if (cmd == "tupletDelete") {
            foreach(Element* e, score()->selection().elements()) {
                  if (e->type() == REST) {
                        Rest* r = static_cast<Rest*>(e);
                        if (r->tuplet())
                              score()->cmdDeleteTuplet(r->tuplet(), true);
                        }
                  }
            }
      else if (cmd == "note-props") {
            Note* note = static_cast<Note*>(e);

            ChordProperties vp(note);
            int rv = vp.exec();
            if (rv) {
                  foreach(Note* note, score()->selection().noteList()) {
                        Chord* chord = note->chord();
                        if (vp.small() != chord->small())
                              score()->undoChangeChordRestSize(chord, vp.small());
                        if (Spatium(vp.leadingSpace()) != chord->extraLeadingSpace()
                           || Spatium(vp.trailingSpace()) != chord->extraTrailingSpace()) {
                              score()->undoChangeChordRestSpace(chord, Spatium(vp.leadingSpace()),
                              Spatium(vp.trailingSpace()));
                              }
                        if (vp.noStem() != chord->noStem())
                              score()->undoChangeChordNoStem(chord, vp.noStem());
                        if (vp.getStemDirection() != chord->stemDirection())
                              score()->undo()->push(new ChangeProperty(chord, P_STEM_DIRECTION, Direction(vp.getStemDirection())));
                        if (vp.tuning() != note->tuning())
                              score()->undoChangeTuning(note, vp.tuning());
                        if (DirectionH(vp.getUserMirror()) != note->userMirror())
                              score()->undoChangeUserMirror(note, DirectionH(vp.getUserMirror()));
                        if (vp.getHeadType() != note->headType() || vp.getHeadGroup() != note->headGroup())
                              score()->undo()->push(new ChangeNoteHead(note, vp.getHeadGroup(), vp.getHeadType()));
                        if (note->veloType() != vp.veloType() || note->veloOffset() != vp.veloOffset()
                           || note->onTimeUserOffset() != vp.onTimeUserOffset()
                           || note->offTimeUserOffset() != vp.offTimeUserOffset()) {
                              score()->undo()->push(new ChangeNoteProperties(note,
                              vp.veloType(), vp.veloOffset(),
                              vp.onTimeUserOffset(),
                              vp.offTimeUserOffset()));
                              }
                        }
                  }
            }
      else if (cmd == "articulation") {
            Note* note = static_cast<Note*>(e);
            Chord* nc = new Chord(*note->chord());
            ChordEditor ce(nc);
            mscore->disableCommands(true);
            if (ce.exec())
                  score()->undoChangeElement(note->chord(), nc);
            else
                  delete nc;
            mscore->disableCommands(false);
            }
      else if (cmd == "style") {
            EditStyle es(e->score(), 0);
            es.setPage(EditStyle::PAGE_NOTE);
            es.exec();
            }
      else if (cmd == "marker-props") {
            MarkerProperties rp(static_cast<Marker*>(e));
            rp.exec();
            }
      else if (cmd == "jump-props") {
            JumpProperties rp(static_cast<Jump*>(e));
            rp.exec();
            }
      else if (cmd == "break-props") {
            LayoutBreak* lb = static_cast<LayoutBreak*>(e);
            SectionBreakProperties sbp(lb, 0);
            if (sbp.exec()) {
                  if (lb->pause() != sbp.pause()
                     || lb->startWithLongNames() != sbp.startWithLongNames()
                     || lb->startWithMeasureOne() != sbp.startWithMeasureOne()) {
                        LayoutBreak* nlb = new LayoutBreak(*lb);
                        nlb->setParent(lb->parent());
                        nlb->setPause(sbp.pause());
                        nlb->setStartWithLongNames(sbp.startWithLongNames());
                        nlb->setStartWithMeasureOne(sbp.startWithMeasureOne());
                        score()->undoChangeElement(lb, nlb);
                        }
                  }
            }
      else if (cmd == "ch-instr") {
            InstrumentChange* ic = static_cast<InstrumentChange*>(e);
            SelectInstrument si(ic->instrument(), 0);
            if (si.exec()) {
                  const InstrumentTemplate* it = si.instrTemplate();
                  if (it) {
                        ic->setInstrument(Instrument::fromTemplate(it));
                        score()->undo()->push(new ChangeInstrument(ic, ic->instrument()));
                        }
                  else
                        printf("no template selected?\n");
                  }
           }
      else if (cmd == "fret-props") {
            FretDiagram* fd = static_cast<FretDiagram*>(e);
            FretDiagram* nFret = const_cast<FretDiagram*>(fd->clone());
            FretDiagramProperties fp(nFret, 0);
            int rv = fp.exec();
            if (rv) {
                  nFret->layout();
                  score()->undoChangeElement(fd, nFret);
                  return;
                  }
            delete nFret;
            }
      else if (cmd == "gliss-props") {
            GlissandoProperties vp(static_cast<Glissando*>(e));
            vp.exec();
            }
      else if (cmd == "hp-props") {
            HairpinSegment* hps = static_cast<HairpinSegment*>(e);
            Hairpin* hp = hps->hairpin();
            HairpinProperties dp(hp);
            int rv = dp.exec();

            int vo = dp.changeVelo();
            DynamicType dt = dp.dynamicType();
            if (rv && ((vo != hp->veloChange())
               || (dt != hp->dynType())
               || (dp.allowDiagonal() != hp->diagonal())
               )) {
                  score()->undo()->push(new ChangeHairpin(hp, vo, dt, dp.allowDiagonal()));
                  }
              }
       else if (cmd == "ha-props") {
            Harmony* ha = static_cast<Harmony*>(e);
            ChordEdit ce(ha->score());
            ce.setHarmony(ha);
            int rv = ce.exec();
            if (rv) {
                  Harmony* h = ce.harmony()->clone();
                  h->render();
                  score()->undoChangeElement(ha, h);
                  }
            }
      else if (cmd == "img-props") {
            Image* img = static_cast<Image*>(e);
            ImageProperties vp(img);
            int rv = vp.exec();
            if (rv) {
                  if (vp.getLockAspectRatio() != img->lockAspectRatio()
                     || vp.getAutoScale() != img->autoScale() || vp.getZ() != img->z()) {
                        score()->undo()->push(new ChangeImage(img,
                           vp.getLockAspectRatio(), vp.getAutoScale(), vp.getZ()));
                        }
                  }
            }
      else if (cmd == "staff-props") {
            EditStaff editStaff(e->staff(), 0);
            editStaff.exec();
            }
      else if (cmd == "invisible")
            score()->undoChangeInvisible(e, !e->visible());
      else if (cmd == "color")
            score()->colorItem(e);
      else if (cmd.startsWith("layer-")) {
            int n = cmd.mid(6).toInt();
            uint mask = 1 << n;
            e->setTag(mask);
            }
      }
