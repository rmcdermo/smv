#include "options.h"
#ifdef pp_HVAC
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include GLUT_H
#include <math.h>

#include "smokeviewvars.h"
#include "IOobjects.h"

unsigned char active_color[3] = {0, 255, 0}, inactive_color[3] = {255, 0, 0};
unsigned char *hvac_color_states[2] = {active_color, inactive_color};

/* ------------------ GetHVACState ------------------------ */

int GetHVACDuctState(hvacductdata *ducti){
  int i, n, *states;
  float *times;
  float current_time;

  if(global_times==NULL)return HVAC_STATE_INACTIVE;
  times = ducti->act_times;
  states = ducti->act_states;
  n = ducti->nact_times;
  if(n==0||times==NULL)return HVAC_STATE_INACTIVE;

  current_time = GetTime();

  if(current_time < times[0])return HVAC_STATE_INACTIVE;
  for(i = 0;i < n - 1;i++){
    if(current_time >= times[i] && current_time < times[i + 1])return states[i];
  }
  return states[n - 1];
}

/* ------------------ GetDuctDir ------------------------ */

int GetDuctDir(float *xyz){
  float eyedir[3];

  //cos(angle) = dir1 .dot. dir2 /(norm(dir1)*norm(dir2))
  eyedir[0] = ABS(xyz[0] - fds_eyepos[0]);
  eyedir[1] = ABS(xyz[1] - fds_eyepos[1]);
  eyedir[2] = ABS(xyz[2] - fds_eyepos[2]);
  if(eyedir[0]>MAX(eyedir[1],eyedir[2]))return 1;
  if(eyedir[1]>MAX(eyedir[0],eyedir[2]))return 0;
  return 2;
}

/* ------------------ DrawHVACDamper ------------------------ */

void DrawHVACDamper(hvacductdata *ducti, float *xyz, float diam, int state){
  float cyl_diam, cyl_height;
  unsigned char color2[3]         = {0, 0, 0};
  unsigned char *color;

  color = hvac_color_states[state];
  color2[0] = CLAMP(255 * foregroundcolor[0], 0, 255);
  color2[1] = CLAMP(255 * foregroundcolor[1], 0, 255);
  color2[2] = CLAMP(255 * foregroundcolor[2], 0, 255);

  cyl_diam = diam / 4.0;
  cyl_height = 3.0 * diam;
  glPushMatrix();
  glTranslatef(xyz[0], xyz[1], xyz[2]);
  DrawSphere(diam, color);
  int damper_dir;

  damper_dir = GetDuctDir(xyz);
  if(damper_dir == 0){
    glRotatef(45.0, 0.0, 1.0, 0.0);
  }
  else if(damper_dir == 1){
    glRotatef(45.0, 1.0, 0.0, 0.0);
  }
  else{
    glRotatef(90.0, 1.0, 0.0, 0.0);
    glRotatef(45.0, 0.0, 1.0, 0.0);
  }
  glTranslatef(0.0, 0.0, -cyl_height/2.0);
  DrawDisk(cyl_diam, cyl_height, color2);
  glPopMatrix();
}

/* ------------------ DrawHVACAircoil ------------------------ */

void DrawHVACAircoil(float *xyz, float size, float diam, int state){
  unsigned char *color;

  color = hvac_color_states[state];

  glPushMatrix();
  glTranslatef(xyz[0], xyz[1], xyz[2]);
  DrawSphere(diam, color);
  glLineWidth(2.0);
  glColor3fv(foregroundcolor);
  glScalef(size, size, size);
  glBegin(GL_LINES);
  glVertex3f(-1.0,  0.0,  0.0);
  glVertex3f(-0.75, 0.0, -0.5);

  glVertex3f(-0.75, 0.0, -0.5);
  glVertex3f(-0.25, 0.0,  0.5);

  glVertex3f(-0.25, 0.0,  0.5);
  glVertex3f( 0.25, 0.0, -0.5);

  glVertex3f(0.25, 0.0, -0.5);
  glVertex3f(0.75, 0.0,  0.5);

  glVertex3f(0.75, 0.0, 0.5);
  glVertex3f(1.0 , 0.0, 0.0);
  glEnd();
  glPopMatrix();
}

/* ------------------ DrawHVACFan ------------------------ */

void DrawHVACFan(float *xyz, float size, float diam, int state){
  int i;
  unsigned char *color;

  color = hvac_color_states[state];
  if(hvac_circ_x == NULL||hvac_circ_y==NULL){
    FREEMEMORY(hvac_circ_x);
    FREEMEMORY(hvac_circ_y);
    NewMemory((void **)&hvac_circ_x,2*HVAC_NCIRC*sizeof(float));
    NewMemory((void **)&hvac_circ_y,2*HVAC_NCIRC*sizeof(float));
    for(i=0;i<2*HVAC_NCIRC;i++){
      float arg;

      arg = 2.0*PI*(float)i/(float)(HVAC_NCIRC-1);
      hvac_circ_x[i] = cos(arg);
      hvac_circ_y[i] = sin(arg);
    }
  }
  glPushMatrix();
  glTranslatef(xyz[0], xyz[1], xyz[2]);
  DrawSphere(diam, color);
  glLineWidth(2.0);
  glScalef(size,size,size);
  glColor3fv(foregroundcolor);
  glBegin(GL_LINES);
  for(i=0;i<HVAC_NCIRC-1;i++){
    float x, y, xp1, yp1;

    x   = hvac_circ_x[i];
    xp1 = hvac_circ_x[i+1];
    y   = hvac_circ_y[i];
    yp1 = hvac_circ_y[i+1];
    glVertex3f(x,   0.0, y);
    glVertex3f(xp1, 0.0, yp1);
  }
  int ibeg, iend;
  float x0, y0;

  int ii;
  for(ii = 0;ii <3;ii++){
    if(ii == 0){
      ibeg = HVAC_NCIRC / 12;
      x0 = 0.0;
      y0 = -1.0;
    }
    else if(ii==1){
      ibeg = 5*HVAC_NCIRC/12;
      x0 = sqrt(3.0)/2.0;
      y0 = 0.5;
    }
    else{
      ibeg = HVAC_NCIRC / 12 + 2*HVAC_NCIRC/3;
      x0 = -sqrt(3.0)/ 2.0;
      y0 = 0.5;
    }
    iend = ibeg + HVAC_NCIRC / 3;

    for(i = ibeg;i < iend;i++){
      float x, y, xp1, yp1;

      x   = hvac_circ_x[i]     + x0;
      xp1 = hvac_circ_x[i + 1] + x0;
      y   = hvac_circ_y[i]     + y0;
      yp1 = hvac_circ_y[i + 1] + y0;
      glVertex3f(x, 0.0, y);
      glVertex3f(xp1, 0.0, yp1);
    }
  }
  glEnd();
  glPopMatrix();
}

/* ------------------ DrawHVACFilter ------------------------ */

void DrawHVACFilter(float *xyz, float size){
  glPushMatrix();
  glTranslatef(xyz[0], xyz[1], xyz[2]);
  glLineWidth(2.0);
  glScalef(size,size,size);
  glBegin(GL_LINES);
  glVertex3f(0.0, -0.5, -1.0);
  glVertex3f(0.0,  0.5, -1.0);

  glVertex3f(0.0, -0.5, -0.5);
  glVertex3f(0.0,  0.5, -0.5);

  glVertex3f(0.0, -0.5,  0.0);
  glVertex3f(0.0,  0.5,  0.0);

  glVertex3f(0.0, -0.5,  0.5);
  glVertex3f(0.0,  0.5,  0.5);

  glVertex3f(0.0, -0.5,  1.0);
  glVertex3f(0.0,  0.5,  1.0);

  glVertex3f(0.0, -0.5, -1.0);
  glVertex3f(0.0, -0.5,  1.0);
  
  glVertex3f(0.0,  0.5, -1.0);
  glVertex3f(0.0,  0.5,  1.0);
  glEnd();
  glPopMatrix();
}

/* ------------------ GetHVACDuctID ------------------------ */

hvacductdata *GetHVACDuctID(char *duct_name){
  int i;

  for(i = 0;i < nhvacductinfo;i++){
    hvacductdata *ducti;

    ducti = hvacductinfo + i;
    if(strcmp(ducti->duct_name, duct_name) == 0)return ducti;
  }
  return NULL;
}

/* ------------------ GetHVACNodeID ------------------------ */

hvacnodedata *GetHVACNodeID(char *node_name){
  int i;

  for(i = 0;i < nhvacnodeinfo;i++){
    hvacnodedata *nodei;

    nodei = hvacnodeinfo + i;
    if(strcmp(nodei->duct_name, node_name) == 0)return nodei;
  }
  return NULL;
}

/* ------------------ DrawHVAC ------------------------ */

void DrawHVAC(hvacdata *hvaci){
  int i;
  unsigned char uc_color[3];

  glPushMatrix();
  glScalef(SCALE2SMV(1.0), SCALE2SMV(1.0), SCALE2SMV(1.0));
  glTranslatef(-xbar0, -ybar0, -zbar0);

  // draw ducts
  
  glLineWidth(hvaci->duct_width);
  glBegin(GL_LINES);
  uc_color[0] = CLAMP(hvaci->duct_color[0], 0, 255);
  uc_color[1] = CLAMP(hvaci->duct_color[1], 0, 255);
  uc_color[2] = CLAMP(hvaci->duct_color[2], 0, 255);
  glColor3ubv(uc_color);
  for(i = 0; i < nhvacductinfo; i++){
    hvacductdata *hvacducti;
    hvacnodedata *node_from, *node_to;
    float* xyz0, * xyz1;

    hvacducti = hvacductinfo + i;
    if(strcmp(hvaci->network_name, hvacducti->network_name) != 0)continue;

    node_from = hvacnodeinfo + hvacducti->node_id_from;
    node_to   = hvacnodeinfo + hvacducti->node_id_to;
    if(node_from == NULL || node_to == NULL)continue;
    xyz0 = node_from->xyz;
    xyz1 = node_to->xyz;
    glVertex3f(xyz0[0], xyz0[1], xyz0[2]);
    if(hvac_metro_view == 1){
      glVertex3f(xyz1[0], xyz0[1], xyz0[2]);
      glVertex3f(xyz1[0], xyz0[1], xyz0[2]);
      glVertex3f(xyz1[0], xyz1[1], xyz0[2]);
      glVertex3f(xyz1[0], xyz1[1], xyz0[2]);
    }
    glVertex3f(xyz1[0], xyz1[1], xyz1[2]);
  }
  glEnd();
  if(hvaci->show_duct_labels == 1){
    for(i = 0; i < nhvacductinfo; i++){
      hvacductdata *ducti;
      hvacnodedata *node_from, *node_to;
      float xyz[3];
      char label[256];
      float offset;

      ducti = hvacductinfo + i;
      if(strcmp(hvaci->network_name, ducti->network_name) != 0)continue;

      strcpy(label, ducti->duct_name);
      node_from = hvacnodeinfo + ducti->node_id_from;
      node_to   = hvacnodeinfo + ducti->node_id_to;
      if(node_from == NULL || node_to == NULL)continue;
      float f1=0.33;
      xyz[0] = f1*node_from->xyz[0] + (1.0-f1)*node_to->xyz[0];
      xyz[1] = f1*node_from->xyz[1] + (1.0-f1)*node_to->xyz[1];
      xyz[2] = f1*node_from->xyz[2] + (1.0-f1)*node_to->xyz[2];
      offset = 0.01/xyzmaxdiff;
      Output3Text(foregroundcolor, xyz[0]+offset, xyz[1]+offset, xyz[2]+offset, label);
    }
  }
  if(hvaci->show_component == DUCT_COMPONENT_TEXT){
    for(i = 0; i < nhvacductinfo; i++){
      hvacductdata *ducti;
      hvacnodedata *node_from, *node_to;
      float xyz[3];
      char label[256];

      ducti = hvacductinfo + i;
      if(strcmp(hvaci->network_name, ducti->network_name) != 0)continue;

      strcpy(label, "");
      strcat(label, ducti->c_component);
      node_from = hvacnodeinfo + ducti->node_id_from;
      node_to   = hvacnodeinfo + ducti->node_id_to;
      if(node_from == NULL || node_to == NULL)continue;
      xyz[0] = (node_from->xyz[0] + node_to->xyz[0])/2.0;
      xyz[1] = (node_from->xyz[1] + node_to->xyz[1])/2.0;
      xyz[2] = (node_from->xyz[2] + node_to->xyz[2])/2.0;
      Output3Text(foregroundcolor, xyz[0], xyz[1], xyz[2]+0.01/xyzmaxdiff, label);
    }
  }
  if(hvaci->show_component == DUCT_COMPONENT_SYMBOLS){
    for(i = 0; i < nhvacductinfo; i++){
      hvacductdata *ducti;
      hvacnodedata *node_from, *node_to;
      float xyz[3];

      ducti = hvacductinfo + i;
      if(strcmp(hvaci->network_name, ducti->network_name) != 0)continue;
      node_from = hvacnodeinfo + ducti->node_id_from;
      node_to   = hvacnodeinfo + ducti->node_id_to;
      if(node_from == NULL || node_to == NULL)continue;
      xyz[0] = (node_from->xyz[0] + node_to->xyz[0])/2.0;
      xyz[1] = (node_from->xyz[1] + node_to->xyz[1])/2.0;
      xyz[2] = (node_from->xyz[2] + node_to->xyz[2])/2.0;
      float size;
      int state;

      state = GetHVACDuctState(ducti);
      size = xyzmaxdiff / 40.0;
      switch(ducti->component){
      case HVAC_NONE:
        break;
      case HVAC_FAN:
        DrawHVACFan(xyz, 2.0*size, size, state);
        break;
      case HVAC_AIRCOIL:
        DrawHVACAircoil(xyz, 2.0*size, size, state);
        break;
      case HVAC_DAMPER:
        DrawHVACDamper(ducti, xyz, size, state);
        break;
      default:
        ASSERT(FFALSE);
        break;
      }
    }
  }

  // draw nodes
  glPointSize(hvaci->node_size);
  glBegin(GL_POINTS);
  uc_color[0] = CLAMP(hvaci->node_color[0], 0, 255);
  uc_color[1] = CLAMP(hvaci->node_color[1], 0, 255);
  uc_color[2] = CLAMP(hvaci->node_color[2], 0, 255);
  glColor3ubv(uc_color);
  for(i = 0; i < nhvacnodeinfo; i++){
    hvacnodedata *nodei;

    nodei = hvacnodeinfo + i;
    if(strcmp(hvaci->network_name, nodei->network_name) != 0)continue;

    glVertex3fv(nodei->xyz);
  }
  glEnd();

  if(hvaci->show_node_labels == 1){
    for(i = 0; i < nhvacnodeinfo; i++){
      hvacnodedata* nodei;
      char label[256];
      float offset;

      nodei = hvacnodeinfo + i;
      if(strcmp(hvaci->network_name, nodei->network_name) != 0)continue;
      offset = 0.01/xyzmaxdiff;
      strcpy(label, nodei->node_name);
      Output3Text(foregroundcolor, nodei->xyz[0]+offset, nodei->xyz[1]+offset, nodei->xyz[2]+offset, label);
    }
  }
  if(hvaci->show_filters == NODE_INFO_LABELS){
    for(i = 0; i < nhvacnodeinfo; i++){
      hvacnodedata *nodei;
      char label[256];
      float offset;

      nodei = hvacnodeinfo + i;
      if(strcmp(hvaci->network_name, nodei->network_name) != 0)continue;
      strcpy(label, nodei->c_filter);
      offset = 0.01 / xyzmaxdiff;
      Output3Text(foregroundcolor, nodei->xyz[0]+offset, nodei->xyz[1] + offset, nodei->xyz[2] + offset, label);
    }
  }
  if(hvaci->show_filters == NODE_INFO_SYMBOLS){
    for(i = 0; i < nhvacnodeinfo; i++){
      hvacnodedata *nodei;
      float size;

      size = xyzmaxdiff / 20.0;
      nodei = hvacnodeinfo + i;
      if(strcmp(hvaci->network_name, nodei->network_name) != 0)continue;
      if(nodei->filter == HVAC_FILTER_NO)continue;
      DrawHVACFilter(nodei->xyz, size);
    }
  }

  glPopMatrix();
}

/* ------------------ DrawHVACS ------------------------ */

void DrawHVACS(void){
  int i;

  for(i=0; i<nhvacinfo; i++){
    hvacdata *hvaci;

    hvaci = hvacinfo + i;
    if(hvaci->display==0)continue;
    DrawHVAC(hvaci);
  }
}

#endif
