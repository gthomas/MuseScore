//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2010 Werner Schweer and others
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

#ifndef __EDITSTAFFTYPE_H__
#define __EDITSTAFFTYPE_H__

#include "ui_stafftype.h"

class StaffType;
class Staff;

//---------------------------------------------------------
//   EditStaffType
//---------------------------------------------------------

class EditStaffType : public QDialog, private Ui::EditStaffType {
      Q_OBJECT

      Staff* staff;
      QList<StaffType*> staffTypes;
      bool modified;
      void saveCurrent(QListWidgetItem*);

   private slots:
      void typeChanged(QListWidgetItem*, QListWidgetItem*);
      void createNewType();
      void nameEdited(const QString&);
      void presetTablatureClicked();

   public slots:
      virtual void accept();

   public:
      EditStaffType(QWidget* parent, Staff*);
      bool isModified() const                 { return modified;   }
      QList<StaffType*> getStaffTypes() const { return staffTypes; }
      };

#endif

