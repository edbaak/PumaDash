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

Table::Table(String title, word borderLines, byte columns, byte rows, word minX, word maxX, word minY, word maxY)
{
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
    Display()->txt_FGcolour(PUMA_LABEL_COLOR);
    Display()->txt_Width(PUMA_LABEL_SIZE);
    Display()->txt_Height(PUMA_LABEL_SIZE);
    word w = Display()->charwidth('A') * PUMA_LABEL_SIZE;
    word x = m_min_x + (m_max_x - m_min_x) / 2;
    x -= w * title.length() / 2;
    m_title_height = Display()->charheight('A') * PUMA_LABEL_SIZE + 4;

    Display()->gfx_MoveTo(x, m_min_y + 2);
    Display()->print(title);
  }

  m_cell_width = (m_max_x - m_min_x) / m_columns;
  m_cell_height = ((m_max_y - m_min_y) - m_title_height) / m_rows;

  if ((m_border_lines & LEFT_BORDER) > 0) {
    Display()->gfx_Line(m_min_x, m_min_y, m_min_x, m_max_y, PUMA_LABEL_COLOR);
  }
  if ((m_border_lines & RIGHT_BORDER) > 0) {
    Display()->gfx_Line(m_max_x, m_min_y, m_max_x, m_max_y, PUMA_LABEL_COLOR);
  }
  if ((m_border_lines & TOP_BORDER) > 0) {
    Display()->gfx_Line(m_min_x, m_min_y, m_max_x, m_min_y, PUMA_LABEL_COLOR);
  }
  if ((m_border_lines & BOTTOM_BORDER) > 0) {
    Display()->gfx_Line(m_min_x, m_max_y, m_max_x, m_max_y, PUMA_LABEL_COLOR);
  }
  if ((m_border_lines & SHOW_GRID) > 0) {
    Display()->gfx_LinePattern(0x00aa);
    byte border = 30;
    if (m_columns > 1)
      for (byte column = 1; column <= m_columns; column++)
        Display()->gfx_Line(cellX(column), m_min_y + border, cellX(column), m_max_y - border, PUMA_LABEL_COLOR);
    if (m_rows > 1)
      for (byte row = 1; row <= m_rows; row++)
        Display()->gfx_Line(m_min_x + border, cellY(row), m_max_x - border, cellY(row), PUMA_LABEL_COLOR);
    Display()->gfx_LinePattern(0);
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

SensorWidget::SensorWidget(word pid, byte fontSize, word x, word y)
{
  m_x = x;
  m_y = y;
  m_pid = pid;
  m_fontSize = fontSize;
  m_next = 0;
}

void SensorWidget::update(OBDData *sensor)
{
  if (sensor == 0) return;

  word x1 = m_x;
  word y1 = m_y;
  // TODO: optimize by only repainting label and sublabel when needed.
  if (sensor->label() != "") {
    Display()->activeScreen()->printLabel(sensor->label(), x1, y1, PUMA_LABEL_COLOR, 1);
    x1 += Display()->activeScreen()->charWidth(PUMA_LABEL_SIZE) * 2;
    y1 += Display()->activeScreen()->charHeight(PUMA_LABEL_SIZE);
  }

  Display()->activeScreen()->printValue(sensor->toString(), sensor->stringLength(), x1, y1, sensor->color(), m_fontSize);

  if (sensor->subLabel() != "") {
    x1 = x1 + (Display()->activeScreen()->charWidth(1) / 2) + (Display()->activeScreen()->charWidth(m_fontSize) * sensor->stringLength());
    y1 = y1 + Display()->activeScreen()->charHeight(m_fontSize) - Display()->activeScreen()->charHeight(1);
    Display()->activeScreen()->printLabel(sensor->subLabel(), x1, y1, PUMA_LABEL_COLOR, 1);
  }
}


// ******************************************************************************************************
//                                              RpmDialWidget
// ******************************************************************************************************

RpmDialWidget::RpmDialWidget(word pid, byte fontSize, word x, word y, word radius) : SensorWidget(pid, fontSize, x, y)
{
  m_radius = radius;
  drawRpmDial();
  update(0);
}

// x = display_x_mid, y = maxHeight() - 100
void RpmDialWidget::drawRpmDial()
{
  Display()->gfx_MoveTo(m_x, m_y);

#define MAX_BUF 10
  word x_pos[MAX_BUF];
  word y_pos[MAX_BUF];
  word i = 0;
  for (word heading = 150; heading <= 390; heading += 40) {
    Display()->gfx_Orbit(heading, m_radius, &x_pos[i], &y_pos[i]);
    Display()->gfx_CircleFilled(x_pos[i], y_pos[i], 4, PUMA_LABEL_COLOR);
    i++;
  }

  for (word heading = 170; heading < 390; heading += 40) {
    word x_start, y_start, x_end, y_end;
    Display()->gfx_Orbit(heading, m_radius, &x_start, &y_start);
    Display()->gfx_Orbit(heading, m_radius + 10, &x_end, &y_end);
    Display()->gfx_Line(x_start, y_start, x_end, y_end, PUMA_LABEL_COLOR);
  }

  for (word heading = 150; heading < 390; heading += 40) {
    word x_start, y_start, x_end, y_end;
    for (word offset = 10; offset < 40; offset += 20) {
      Display()->gfx_Orbit(heading + offset, m_radius, &x_start, &y_start);
      Display()->gfx_Orbit(heading + offset, m_radius + 5, &x_end, &y_end);
      Display()->gfx_Line(x_start, y_start, x_end, y_end, PUMA_LABEL_COLOR);
    }
  }

  Display()->activeScreen()->printLabel("0", x_pos[0] - 23, y_pos[0] - 5, PUMA_LABEL_COLOR, 2);
  Display()->activeScreen()->printLabel("1", x_pos[1] - 20, y_pos[1] - 10, PUMA_LABEL_COLOR, 2);
  Display()->activeScreen()->printLabel("2", x_pos[2] - 25, y_pos[2] - 18, PUMA_LABEL_COLOR, 2);
  Display()->activeScreen()->printLabel("3", x_pos[3] - 6, y_pos[3] - 30, PUMA_LABEL_COLOR, 2);
  Display()->activeScreen()->printLabel("4", x_pos[4] + 10, y_pos[4] - 18, PUMA_LABEL_COLOR, 2);
  Display()->activeScreen()->printLabel("5", x_pos[5] + 10, y_pos[5] - 10, PUMA_LABEL_COLOR, 2);
  Display()->activeScreen()->printLabel("6", x_pos[6] + 9, y_pos[6] - 5, PUMA_LABEL_COLOR, 2);
}

void RpmDialWidget::update(OBDData *sensor)
{
  if (sensor == 0)
    updateRpm(0);
  else
    updateRpm(sensor->toWord());
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
    Display()->gfx_TriangleFilled(x_pos[0], y_pos[0], x_pos[1], y_pos[1], x_pos[2], y_pos[2], BLACK);

  word rpm_heading = 150 + round(rpm * 40.0 / 1000.0);
  Display()->gfx_MoveTo(m_x, m_y);
  Display()->gfx_Orbit(rpm_heading, m_radius - 7, &x_pos[0], &y_pos[0]);
  Display()->gfx_Orbit(rpm_heading - 4, m_radius - 30, &x_pos[1], &y_pos[1]);
  Display()->gfx_Orbit(rpm_heading + 4, m_radius - 30, &x_pos[2], &y_pos[2]);
  Display()->gfx_TriangleFilled(x_pos[0], y_pos[0], x_pos[1], y_pos[1], x_pos[2], y_pos[2], color);

  // TODO: fix print position
  Display()->activeScreen()->printValue(String(rpm), 3, m_x - 58, m_y - 95, color, 3);
}


// ******************************************************************************************************
//                                              PitchAndRollWidget
// ******************************************************************************************************

PitchAndRollWidget::PitchAndRollWidget(word pid, byte fontSize, word x, word y, bool pitchMode, byte interleave) : SensorWidget(pid, fontSize, x, y)
{
  m_pitchMode = pitchMode;
  m_interleave = interleave;
  update(0);
}

void PitchAndRollWidget::update(OBDData *sensor)
{
  if (sensor == 0)
    updateAngle(0);
  else
    updateAngle(sensor->toInt());
}

void PitchAndRollWidget::updateAngle(int angle)
{
  // update the static display items
  word y_ = m_y;
  word x_ = m_x;
  for (int i = -8; i <= 8; i++) {
    int w = 2;
    if (abs(i) == 2 || abs(i) == 4 || abs(i) == 6 || abs(i) == 8)
      w = 5;
    if (i == 0) {
      if (m_pitchMode)
        Display()->gfx_CircleFilled(m_x+4, y_, 3, PUMA_LABEL_COLOR);
      else
        Display()->gfx_CircleFilled(x_, m_y-4, 3, PUMA_LABEL_COLOR);
    } else {
      if (m_pitchMode)
        Display()->gfx_Line(m_x, y_, m_x + w, y_, PUMA_LABEL_COLOR);
      else
        Display()->gfx_Line(x_, m_y - w, x_, m_y, PUMA_LABEL_COLOR);
    }
    x_ += m_interleave;
    y_ += m_interleave;
  }

  int color = PUMA_NORMAL_COLOR;
  if (abs(angle) > 35)
    color = PUMA_ALARM_COLOR;
  else if (abs(angle) > 15)
    color = PUMA_WARNING_COLOR;

  if (m_pitchMode)
    Display()->activeScreen()->printValue(String(abs(angle)), 3, m_x - 1, m_y + m_interleave * 16 + 3, color, m_fontSize);
  else
    Display()->activeScreen()->printValue(String(abs(angle)), 3, m_x + m_interleave * 16 + 4, m_y - 15, color, m_fontSize);

  if (angle > 40)
    angle = 40;
  else if (angle < -40)
    angle = -40;

  float f = angle;
  f = f * m_interleave / 5.0;
  static word x_last_p = 0;
  static word y_last_p = 0;
  static word x_last_r = 0;
  static word y_last_r = 0;

  // Reset display area
    if (m_pitchMode) {
      if (x_last_p != 0)
        Display()->gfx_TriangleFilled(x_last_p, y_last_p, x_last_p + 20, y_last_p - 5, x_last_p + 20, y_last_p + 5, BLACK);
    } else {
      if (x_last_r != 0)
        Display()->gfx_TriangleFilled(x_last_r, y_last_r, x_last_r - 5, y_last_r - 20, x_last_r + 5, y_last_r - 20, BLACK);
    }

  if (m_pitchMode) {
    x_last_p = m_x + 10;
    y_last_p = m_y + m_interleave * 8 - round(f);
    Display()->gfx_TriangleFilled(x_last_p, y_last_p, x_last_p + 20, y_last_p - 5, x_last_p + 20, y_last_p + 5, color);
  } else {
    x_last_r = m_x + m_interleave * 8 + round(f);
    y_last_r = m_y - 10;
    Display()->gfx_TriangleFilled(x_last_r, y_last_r, x_last_r - 5, y_last_r - 20, x_last_r + 5, y_last_r - 20, color);
  }
}

// ******************************************************************************************************
//                                              CompassWidget
// ******************************************************************************************************

CompassWidget::CompassWidget(word pid, byte fontSize, word x, word y) : SensorWidget(pid, fontSize, x, y)
{
  update(0);
}

void CompassWidget::update(OBDData *sensor)
{
  if (sensor == 0)
    updateHeading(0);
  else
    updateHeading(sensor->toWord());
}

void CompassWidget::updateHeading(word heading)
{
  static long int old_heading = -1;
  if (heading != old_heading) {
    old_heading = heading;

    Display()->txt_Xgap(2);
    char hd[5];
    sprintf(hd, "%03d", heading);
    Display()->activeScreen()->printValue(hd, 3, 65, 40, PUMA_NORMAL_COLOR, m_fontSize);
    Display()->txt_Xgap(0);

    heading += 270;
    if (heading > 360)
      heading -= 360;

    int x_ = 245;
    int y_ = 25;
    int radius_ = 25;
    Display()->gfx_CircleFilled(x_, y_, radius_ - 1, BLACK);
    Display()->gfx_Circle(x_, y_, radius_, PUMA_LABEL_COLOR);
    Display()->gfx_MoveTo(x_, y_);
    word orbitX1, orbitY1;
    Display()->gfx_Orbit(heading, radius_, &orbitX1, &orbitY1);
    word orbitX2, orbitY2;
    Display()->gfx_Orbit(heading + 150, radius_ / 2, &orbitX2, &orbitY2);
    word orbitX3, orbitY3;
    Display()->gfx_Orbit(heading + 210, radius_ / 2, &orbitX3, &orbitY3);
    Display()->gfx_TriangleFilled(orbitX1, orbitY1, orbitX2, orbitY2, orbitX3, orbitY3, PUMA_NORMAL_COLOR);
  }
}

// ******************************************************************************************************
//                                              TpmsWidget
// ******************************************************************************************************

#define TPMS_X1_OFFSET 55
#define TPMS_Y1_OFFSET 75
#define TPMS_X2_OFFSET 85
#define TPMS_Y2_OFFSET 30

TpmsWidget::TpmsWidget(word pid, TPMS_MODE mode, byte fontSize, word x, word y) : SensorWidget(pid, fontSize, x, y)
{
  m_mode = mode;
  update(0);
}

void TpmsWidget::update(OBDData *sensor)
{
  if (m_mode == TPMS_PRESSURE) {
    if (sensor == 0)
      updatePressure("--");
    else
      updatePressure(sensor->toString());
  }
  else
  {
    if (sensor == 0)
      updateTemperature("--");
    else
      updateTemperature(sensor->toString());

  }
}

void TpmsWidget::updatePressure(String pressure)
{
  Display()->gfx_Line(m_x + TPMS_X1_OFFSET, m_y + TPMS_Y1_OFFSET , m_x + TPMS_X2_OFFSET, m_y + TPMS_Y2_OFFSET, PUMA_LABEL_COLOR);

  word x1 = m_x + TPMS_X1_OFFSET / 2;
  word y2 = m_y + TPMS_Y2_OFFSET;
  int color = PUMA_NORMAL_COLOR;
  //  if (Display()->m_tpms->tirePressureAlarm(tireLocation))
  //    color = PUMA_ALARM_COLOR;
  //  else if (Display()->m_tpms->tirePressureWarning(tireLocation))
  //    color = PUMA_WARNING_COLOR;
  Display()->activeScreen()->printValue(pressure, 2, x1, y2, color, m_fontSize);
}

void TpmsWidget::updateTemperature(String temperature)
{
  int color = PUMA_NORMAL_COLOR;
  word x2 = m_x + TPMS_X2_OFFSET;
  word y1 = m_y + TPMS_Y1_OFFSET - 20;
  //  if (Display()->m_tpms->tireTemperatureAlarm(tireLocation))
  //    color = PUMA_ALARM_COLOR;
  //  else if (Display()->m_tpms->tireTemperatureWarning(tireLocation))
  //    color = PUMA_WARNING_COLOR;
  Display()->activeScreen()->printValue(temperature, 2, x2, y1, color, m_fontSize);
}

// ******************************************************************************************************
//                                              TpmsWidget
// ******************************************************************************************************

ListWidget::ListWidget(String title, word pid, byte fontSize, word x1, word y1, word x2, word y2) : SensorWidget(pid, fontSize, x1, y1)
{
  m_title = title;
  m_x2 = x2;
  m_y2 = y2;
}

void ListWidget::update(OBDData *sensor)
{
  Serial.println("Update list widget");
}

void ListWidget::appendLine(String line)
{
  line = line;
}




