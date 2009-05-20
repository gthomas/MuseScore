//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2002-2009 Werner Schweer and others
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

#ifndef __EDITSTAFF_H__
#define __EDITSTAFF_H__

#include "ui_editstaff.h"

class Staff;

//---------------------------------------------------------
//   EditStaff
//    edit staff and part properties
//---------------------------------------------------------

class EditStaff : public QDialog, private Ui::EditStaffBase {
      Q_OBJECT

      Staff* staff;

      void apply();

   private slots:
      void bboxClicked(QAbstractButton* button);
      void editDrumsetClicked();

   public:
      EditStaff(Staff*, QWidget* parent = 0);
      };


#endif

