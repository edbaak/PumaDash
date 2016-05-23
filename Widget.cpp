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

  You should have received a copy of the GNU General Public License
  along with this code; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-
  1301  USA
*/

#include "Utils.h"
#include "Widget.h"
#include "Display.h"

// ************************************************************************

Table::Table(PumaDisplay *display, String title, word borderLines, byte columns, byte rows, word minX, word maxX, word minY, word maxY)
{
  m_display = display;
  m_title = title;
  m_border_lines = borderLines;
  if (columns < 1) columns = 1;
  m_columns = columns;
  m_min_x = minX;
  m_max_x = maxX;
  if (rows < 1) rows = 1;
  m_rows = rows;
  m_min_y = minY;
  m_max_y = maxY;
  m_title_height = 0;
  if (title.length() > 0) {
    display->txt_FGcolour(PUMA_LABEL_COLOR);
    display->txt_Width(PUMA_LABEL_SIZE);
    display->txt_Height(PUMA_LABEL_SIZE);
    word w = display->charwidth('A') * PUMA_LABEL_SIZE;
    word x = m_min_x + (m_max_x - m_min_x) / 2;
    x -= w * title.length() / 2;
    m_title_height = display->charheight('A') * PUMA_LABEL_SIZE + 4;

    display->gfx_MoveTo(x, m_min_y + 2);
    display->print(title);
  }

  m_cell_width = (m_max_x - m_min_x) / m_columns;
  m_cell_height = ((m_max_y - m_min_y) - m_title_height) / m_rows;

  if ((m_border_lines & LEFT_BORDER) > 0) {
    display->gfx_Line(m_min_x, m_min_y, m_min_x, m_max_y, PUMA_LABEL_COLOR);
  }
  if ((m_border_lines & RIGHT_BORDER) > 0) {
    display->gfx_Line(m_max_x, m_min_y, m_max_x, m_max_y, PUMA_LABEL_COLOR);
  }
  if ((m_border_lines & TOP_BORDER) > 0) {
    display->gfx_Line(m_min_x, m_min_y, m_max_x, m_min_y, PUMA_LABEL_COLOR);
  }
  if ((m_border_lines & BOTTOM_BORDER) > 0) {
    display->gfx_Line(m_min_x, m_max_y, m_max_x, m_max_y, PUMA_LABEL_COLOR);
  }
  if ((m_border_lines & SHOW_GRID) > 0) {
    display->gfx_LinePattern(0x00aa);
    byte border = 30;
    if (m_columns > 1)
      for (byte column = 1; column <= m_columns; column++)
        display->gfx_Line(cellX(column), m_min_y + border, cellX(column), m_max_y - border, PUMA_LABEL_COLOR);
    if (m_rows > 1)
      for (byte row = 1; row <= m_rows; row++)
        display->gfx_Line(m_min_x + border, cellY(row), m_max_x - border, cellY(row), PUMA_LABEL_COLOR);
    display->gfx_LinePattern(0);
  }
}

word Table::cellX(byte column)
{
  return m_min_x + (column * m_cell_width);
}

word Table::cellY(byte row)
{
  return m_min_y + m_title_height + (row * m_cell_height);
}

// ******************************************************************************************************
//                                              SensorWidget
// ******************************************************************************************************

SensorWidget::SensorWidget(PumaDisplay *display, word pid, byte fontSize, word x, word y)
{
  m_display = display;
  m_x = x;
  m_y = y;
  m_pid = pid;
  m_fontSize = fontSize;
  m_next = 0;
}


// ******************************************************************************************************
//                                              RpmDialWidget
// ******************************************************************************************************

RpmDialWidget::RpmDialWidget(PumaDisplay *display, word pid, byte fontSize, word x, word y, word radius) : SensorWidget(display, pid, fontSize, x, y)
{
  m_radius = radius;
}

// x = display_x_mid, y = maxHeight() - 100
void RpmDialWidget::drawRpmDial()
{
  m_display->gfx_MoveTo(m_x, m_y);

#define MAX_BUF 10
  word x_pos[MAX_BUF];
  word y_pos[MAX_BUF];
  word i = 0;
  for (word heading = 150; heading <= 390; heading += 40) {
    m_display->gfx_Orbit(heading, m_radius, &x_pos[i], &y_pos[i]);
    m_display->gfx_CircleFilled(x_pos[i], y_pos[i], 4, PUMA_LABEL_COLOR);
    i++;
  }

  for (word heading = 170; heading < 390; heading += 40) {
    word x_start, y_start, x_end, y_end;
    m_display->gfx_Orbit(heading, m_radius, &x_start, &y_start);
    m_display->gfx_Orbit(heading, m_radius + 10, &x_end, &y_end);
    m_display->gfx_Line(x_start, y_start, x_end, y_end, PUMA_LABEL_COLOR);
  }

  for (word heading = 150; heading < 390; heading += 40) {
    word x_start, y_start, x_end, y_end;
    for (word offset = 10; offset < 40; offset += 20) {
      m_display->gfx_Orbit(heading + offset, m_radius, &x_start, &y_start);
      m_display->gfx_Orbit(heading + offset, m_radius + 5, &x_end, &y_end);
      m_display->gfx_Line(x_start, y_start, x_end, y_end, PUMA_LABEL_COLOR);
    }
  }

  m_display->activeScreen()->printLabel("0", x_pos[0] - 23, y_pos[0] - 5, PUMA_LABEL_COLOR, 2);
  m_display->activeScreen()->printLabel("1", x_pos[1] - 20, y_pos[1] - 10, PUMA_LABEL_COLOR, 2);
  m_display->activeScreen()->printLabel("2", x_pos[2] - 25, y_pos[2] - 18, PUMA_LABEL_COLOR, 2);
  m_display->activeScreen()->printLabel("3", x_pos[3] - 6, y_pos[3] - 30, PUMA_LABEL_COLOR, 2);
  m_display->activeScreen()->printLabel("4", x_pos[4] + 10, y_pos[4] - 18, PUMA_LABEL_COLOR, 2);
  m_display->activeScreen()->printLabel("5", x_pos[5] + 10, y_pos[5] - 10, PUMA_LABEL_COLOR, 2);
  m_display->activeScreen()->printLabel("6", x_pos[6] + 9, y_pos[6] - 5, PUMA_LABEL_COLOR, 2);
}

void RpmDialWidget::updateRpm(word rpm)
{
  word color = PUMA_NORMAL_COLOR;
  if (rpm >= 4000)
    color = PUMA_ALARM_COLOR;
  else if (rpm < 1200)
    color = PUMA_WARNING_COLOR;

  static word last_rpm = 0;
  static word x_pos[3] = {0, 0, 0};
  static word y_pos[3] = {0, 0, 0};

  if (x_pos[0] != 0 && last_rpm != rpm)
    m_display->gfx_TriangleFilled(x_pos[0], y_pos[0], x_pos[1], y_pos[1], x_pos[2], y_pos[2], BLACK);

  word rpm_heading = 150 + round(rpm * 40.0 / 1000.0);
  m_display->gfx_MoveTo(m_x, m_y);
  m_display->gfx_Orbit(rpm_heading, m_radius - 7, &x_pos[0], &y_pos[0]);
  m_display->gfx_Orbit(rpm_heading - 4, m_radius - 30, &x_pos[1], &y_pos[1]);
  m_display->gfx_Orbit(rpm_heading + 4, m_radius - 30, &x_pos[2], &y_pos[2]);
  m_display->gfx_TriangleFilled(x_pos[0], y_pos[0], x_pos[1], y_pos[1], x_pos[2], y_pos[2], color);

  // TODO: fix print position
  m_display->activeScreen()->printValue(String(rpm), m_x - 58, m_y - 95, color, 3);
}


// ******************************************************************************************************
//                                              PitchAndRollWidget
// ******************************************************************************************************

PitchAndRollWidget::PitchAndRollWidget(PumaDisplay *display, word pid, byte fontSize, word x, word y, bool pitchMode, byte interleave) : SensorWidget(display, pid, fontSize, x, y)
{
  m_pitchMode = pitchMode;
  m_interleave = interleave;
}

void PitchAndRollWidget::updateAngle(int angle)
{
  // update the static display items
  if (m_pitchMode) {
    m_display->gfx_Line(m_x, m_y, m_x, m_y + m_interleave * 18, PUMA_LABEL_COLOR);
  } else {
    m_display->gfx_Line(m_x, m_y, m_x + m_interleave * 18, m_y, PUMA_LABEL_COLOR);
  }

  word y_ = m_y;
  word x_ = m_x;
  for (int i = -9; i <= 9; i++) {
    int w = 2;
    if (i == 0  || abs(i) == 9)
      w = 8;
    else if (abs(i) == 2 || abs(i) == 4 || abs(i) == 6 || abs(i) == 8)
      w = 5;
    if (m_pitchMode)
      m_display->gfx_Line(m_x, y_, m_x + w, y_, PUMA_LABEL_COLOR);
    else
      m_display->gfx_Line(x_, m_y - w, x_, m_y, PUMA_LABEL_COLOR);
    x_ += m_interleave;
    y_ += m_interleave;
  }

  int color = PUMA_NORMAL_COLOR;
  if (abs(angle) > 35)
    color = PUMA_ALARM_COLOR;
  else if (abs(angle) > 15)
    color = PUMA_WARNING_COLOR;

  if (m_pitchMode)
    m_display->activeScreen()->printValue(String(abs(angle)), m_x - 1, m_y + m_interleave * 18 + 1, color, 2);
  else
    m_display->activeScreen()->printValue(String(abs(angle)), m_x + m_interleave * 18 + 4, m_y - 13, color, 2);

  if (angle > 45)
    angle = 45;
  else if (angle < -45)
    angle = -45;

  float f = angle;
  f = f * m_interleave / 5.0;
  static word x_last = 0;
  static word y_last = 0;

  // Reset display area
  if (x_last != 0) {
    if (m_pitchMode)
      m_display->gfx_TriangleFilled(x_last, y_last, x_last + 20, y_last - 5, x_last + 20, y_last + 5, BLACK);
    else
      m_display->gfx_TriangleFilled(x_last, y_last, x_last - 5, y_last - 20, x_last + 5, y_last - 20, BLACK);
  }

  if (m_pitchMode) {
    x_last = m_x + 10;
    y_last = m_y + m_interleave * 9 - round(f);
    m_display->gfx_TriangleFilled(x_last, y_last, x_last + 20, y_last - 5, x_last + 20, y_last + 5, color);
  } else {
    x_last = m_x + m_interleave * 9 + round(f);
    y_last = m_y - 10;
    m_display->gfx_TriangleFilled(x_last, y_last, x_last - 5, y_last - 20, x_last + 5, y_last - 20, color);
  }
}

// ******************************************************************************************************
//                                              CompassWidget
// ******************************************************************************************************

CompassWidget::CompassWidget(PumaDisplay *display, word pid, byte fontSize, word x, word y) : SensorWidget(display, pid, fontSize, x, y)
{
}

void CompassWidget::updateHeading(word heading)
{
  static long int old_heading = -1;
  if (heading != old_heading) {
    old_heading = heading;

    m_display->txt_Xgap(2);
    char hd[5];
    sprintf(hd, "%0d3", heading);
    m_display->activeScreen()->printValue(hd, 65, 40, PUMA_NORMAL_COLOR, 6);
    m_display->txt_Xgap(0);

    heading += 270;
    if (heading > 360)
      heading -= 360;

    int x_ = 245;
    int y_ = 25;
    int radius_ = 25;
    m_display->gfx_CircleFilled(x_, y_, radius_ - 1, BLACK);
    m_display->gfx_Circle(x_, y_, radius_, PUMA_LABEL_COLOR);
    m_display->gfx_MoveTo(x_, y_);
    word orbitX1, orbitY1;
    m_display->gfx_Orbit(heading, radius_, &orbitX1, &orbitY1);
    word orbitX2, orbitY2;
    m_display->gfx_Orbit(heading + 150, radius_ / 2, &orbitX2, &orbitY2);
    word orbitX3, orbitY3;
    m_display->gfx_Orbit(heading + 210, radius_ / 2, &orbitX3, &orbitY3);
    m_display->gfx_TriangleFilled(orbitX1, orbitY1, orbitX2, orbitY2, orbitX3, orbitY3, PUMA_NORMAL_COLOR);
  }
}


