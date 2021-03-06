/*
  2016 Copyright (c) Ed Baak  All Rights Reserved.

  This code is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either
  version 3 of the License, or (at your option) any later version.

  This code is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  General Public License for more details.

  WARNING: Modifying a vehicle's dashboard and instrument panel
  may require vehicle engineering and re-certification according
  to local laws and regulations, such as the Australian Design
  Rules (ADR) and Vehicle Standards. This code does not make any
  claim to meet any such standard.

  You should have received a copy of the GNU General Public License
  along with this code; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-
  1301  USA
*/

#ifndef Widget_h
#define Widget_h

#if (ARDUINO >= 100)
#include "Arduino.h" // for Arduino 1.0
#else
#include "WProgram.h" // for Arduino 23
#endif

#include "Display.h"
#include "OBD.h"

class StringList
{
  public:
    StringList();
    virtual ~StringList();
    void addString(String s);
    word count();
    String stringAt(word index);
    bool deleteAt(word index);
    void deleteAll();

  private:
    struct StringListElement {
      String m_buf;
      StringListElement *m_next;
    };
    StringListElement *m_first;
    word m_count;
};

class TableWidget
{
  public:
    typedef enum BORDER_LINES {
      NONE = 0,
      LEFT_BORDER = 0x01,
      TOP_BORDER = 0x02,
      RIGHT_BORDER = 0x04,
      BOTTOM_BORDER = 0x08,
      ALL_BORDER = 0x0F,
      SHOW_GRID = 0x10
    } BORDER_LINES;

    TableWidget(String title, word borderLines, byte columns, byte rows, word minX, word minY, word maxX, word maxY);
    word X(byte column);
    word Y(byte row);

  private:
    String m_title;
    word m_title_height;
    word m_border_lines;
    word m_columns;
    word m_min_x;
    word m_max_x;
    word m_rows;
    word m_min_y;
    word m_max_y;
    word m_cell_width;
    word m_cell_height;
};

class SensorWidget
{
  public:
    SensorWidget(word pid, byte fontSize, word x, word y);
    virtual void update(OBDBaseData *sensor);
    void requestStaticRefresh();

  protected:
    friend class BaseScreen;
    word m_pid;
    byte m_fontSize;
    word m_x;
    word m_y;
    bool m_staticRefreshRequested;

    SensorWidget *m_next;
};

class RpmDialWidget : public SensorWidget
{
  public:
    RpmDialWidget(word pid, byte fontSize, word x, word y, word radius);
    void drawRpmDial();
    void updateRpm(word rpm, word color);
    virtual void update(OBDBaseData *sensor);

  private:
    word m_radius;
};

class PitchAndRollWidget : public SensorWidget
{
  public:
    PitchAndRollWidget(word pid, byte fontSize, word x, word y, bool pitchMode, byte interleave);
    void updateAngle(int angle);
    virtual void update(OBDBaseData *sensor);

  private:
    bool m_pitchMode;
    byte m_interleave;
};

class CompassWidget : public SensorWidget
{
  public:
    CompassWidget(word pid, byte fontSize, word x, word y);
    void updateHeading(word heading);
    virtual void update(OBDBaseData *sensor);
};

typedef enum TPMS_MODE {
  TPMS_PRESSURE,
  TPMS_TEMPERATURE
} TPMS_MODE;

class TpmsWidget : public SensorWidget
{
  public:
    TpmsWidget(word pid, TPMS_MODE mode, byte fontSize, word x, word y);
    virtual void update(OBDBaseData *sensor);

  protected:
    void updatePressure(String pressure, word color);
    void updateTemperature(String temperature, word color);

  private:
    TPMS_MODE m_mode;
};

class ListWidget : public SensorWidget
{
  public:
    ListWidget(String title, word pid, byte fontSize, word x1, word y1, word x2, word y2);
    void appendLine(String line);
    virtual void update(OBDBaseData *sensor);

  private:
    String m_title;
    word m_x2;
    word m_y2;
};

#endif



