#define CPP
#include "options.h"

#include <stdio.h>
#include <string.h>
#include GLUT_H
#include <math.h>

#include "smokeviewvars.h"
#include "glui_tour.h"

static int tour_hide=0;
static char tour_label[sizeof(GLUI_String)];

GLUI *glui_tour=NULL;

GLUI_Rollout *ROLLOUT_keyframe = NULL;
GLUI_Rollout *ROLLOUT_settings = NULL;
GLUI_Rollout *ROLLOUT_circular = NULL;

GLUI_Panel *PANEL_misc = NULL;
GLUI_Panel *PANEL_node = NULL;
GLUI_Panel *PANEL_tour = NULL;
GLUI_Panel *PANEL_settingskeyframe=NULL;
GLUI_Panel *PANEL_path=NULL;
GLUI_Panel *PANEL_tour1=NULL;
GLUI_Panel *PANEL_tour3=NULL;
GLUI_Panel *PANEL_close_tour=NULL;
GLUI_Panel *PANEL_pos=NULL;
GLUI_Panel *PANEL_tourposition=NULL;
GLUI_Panel *PANEL_tournavigate=NULL;
GLUI_Panel *PANEL_tourview=NULL;
GLUI_Panel *PANEL_tour_circular_center;
GLUI_Panel *PANEL_tour_circular_view;

GLUI_Checkbox *CHECKBOX_view1 = NULL;
GLUI_Checkbox *CHECKBOX_view2 = NULL;
GLUI_Checkbox *CHECKBOX_tour_snap = NULL;
GLUI_Checkbox *CHECKBOX_showtourroute1 = NULL;
GLUI_Checkbox *CHECKBOX_showtourroute2 = NULL;
GLUI_Checkbox *CHECKBOX_showintermediate = NULL;
GLUI_Checkbox *CHECKBOX_tourhide=NULL;

GLUI_Spinner *SPINNER_tour_time = NULL;
#ifdef pp_TOUR
GLUI_Spinner *SPINNER_tour_pause_time = NULL;
#endif
GLUI_Spinner *SPINNER_x = NULL;
GLUI_Spinner *SPINNER_y = NULL;
GLUI_Spinner *SPINNER_z = NULL;
GLUI_Spinner *SPINNER_viewx = NULL;
GLUI_Spinner *SPINNER_viewy = NULL;
GLUI_Spinner *SPINNER_viewz = NULL;

GLUI_Spinner *SPINNER_tour_circular_view[3];
GLUI_Spinner *SPINNER_tour_circular_center[3];
GLUI_Spinner *SPINNER_tour_circular_radius = NULL;
GLUI_Spinner *SPINNER_tour_circular_angle0 = NULL;

GLUI_Button *BUTTON_next_tour=NULL;
GLUI_Button *BUTTON_prev_tour=NULL;
GLUI_Button *BUTTON_delete_tour = NULL;
GLUI_EditText *EDIT_label=NULL;

GLUI_Listbox *LISTBOX_tour=NULL;
GLUI_Listbox *LISTBOX_avatar=NULL;

procdata toursprocinfo[3];
int ntoursprocinfo = 0;

/* ------------------ ToursRolloutCB ------------------------ */

void ToursRolloutCB(int var){
  ToggleRollout(toursprocinfo, ntoursprocinfo, var);
  if(var == MODIFY_TOURS_ROLLOUT){
    if(ROLLOUT_circular->is_open == 1){
      selectedtour_index = 0;
      TourCB(TOUR_LIST);
      LISTBOX_tour->set_int_val(selectedtour_index);
    }
  }
}

/* ------------------ SetKeyframeViews ------------------------ */

void SetKeyFrameViews(float *view){
  keyframe *this_key, *first_key, *last_key;

  if(selected_tour==NULL)return;

  first_key = selected_tour->first_frame.next;
  last_key = selected_tour->last_frame.prev;
  for(this_key = first_key; this_key->next!=NULL; this_key = this_key->next){
    keyframe *next_key;

    next_key = this_key->next;
    if(view==NULL){
      if(this_key==last_key){
        float view_temp[3];
        float *this_xyz, *prev_xyz;

        this_xyz = this_key->xyz_smv;
        prev_xyz = this_key->prev->xyz_smv;

        view_temp[0] = this_xyz[0]+(this_xyz[0]-prev_xyz[0]);
        view_temp[1] = this_xyz[1]+(this_xyz[1]-prev_xyz[1]);
        view_temp[2] = this_xyz[2]+(this_xyz[2]-prev_xyz[2]);
        memcpy(this_key->view_smv, view_temp, 3*sizeof(float));
      }
      else{
        memcpy(this_key->view_smv, next_key->xyz_smv, 3*sizeof(float));
      }
    }
    else{
      memcpy(this_key->view_smv, view, 3*sizeof(float));
    }
  }
}

/* ------------------ UpdateTourState ------------------------ */

extern "C" void UpdateTourState(void){
  TourCB(SHOWTOURROUTE);
  TourCB(VIEWTOURFROMPATH);
}

/* ------------------ AddDeleteKeyframe ------------------------ */

void AddDeleteKeyframe(int flag){
  if(flag==ADD_KEYFRAME)TourCB(KEYFRAME_INSERT);
  if(flag==DELETE_KEYFRAME)TourCB(KEYFRAME_DELETE);
}


/* ------------------ UpdateEditTour ------------------------ */

extern "C" void UpdateEditTour(void){
  TourCB(SHOWTOURROUTE);
}

/* ------------------ UpdateTourParms ------------------------ */

extern "C" void UpdateTourParms(void){
  TourCB(KEYFRAME_tXYZ);
}

/* ------------------ AddNewTour ------------------------ */

extern "C" void AddNewTour(void){
  TourCB(TOUR_INSERT_NEW);
}

/* ------------------ GluiTourSetup ------------------------ */

extern "C" void GluiTourSetup(int main_window){

  int i;

  if(glui_tour!=NULL){
    glui_tour->close();
    glui_tour=NULL;
  }
  glui_tour = GLUI_Master.create_glui(_("Tours"),0,0,0);
  glui_tour->hide();

  ROLLOUT_circular = glui_tour->add_rollout(_("Modify circular tour"),false, MODIFY_TOURS_ROLLOUT, ToursRolloutCB);
  INSERT_ROLLOUT(ROLLOUT_circular, glui_tour);
  ADDPROCINFO(toursprocinfo, ntoursprocinfo, ROLLOUT_circular, MODIFY_TOURS_ROLLOUT, glui_tour);

  CHECKBOX_showtourroute2 = glui_tour->add_checkbox_to_panel(ROLLOUT_circular, _("Show tour"), &edittour, SHOWTOURROUTE2, TourCB);
  CHECKBOX_view2 = glui_tour->add_checkbox_to_panel(ROLLOUT_circular, _("View from tour path"), &viewtourfrompath, VIEWTOURFROMPATH2, TourCB);

  SPINNER_tour_circular_radius = glui_tour->add_spinner_to_panel(ROLLOUT_circular, "radius", GLUI_SPINNER_FLOAT, &tour_circular_radius,TOUR_CIRCULAR_UPDATE,TourCB);
  SPINNER_tour_circular_angle0 = glui_tour->add_spinner_to_panel(ROLLOUT_circular, "initial angle", GLUI_SPINNER_FLOAT, &tour_circular_angle0, TOUR_CIRCULAR_UPDATE, TourCB);
  glui_tour->add_spinner_to_panel(ROLLOUT_circular, "speedup factor", GLUI_SPINNER_FLOAT, &tour_speedup_factor, TOUR_CIRCULAR_UPDATE, TourCB);

  PANEL_tour_circular_center = glui_tour->add_panel_to_panel(ROLLOUT_circular,_("center"),true);
  SPINNER_tour_circular_center[0]=glui_tour->add_spinner_to_panel(PANEL_tour_circular_center,"x",GLUI_SPINNER_FLOAT,tour_circular_center,TOUR_CIRCULAR_UPDATE,TourCB);
  SPINNER_tour_circular_center[1]=glui_tour->add_spinner_to_panel(PANEL_tour_circular_center,"y",GLUI_SPINNER_FLOAT,tour_circular_center+1,TOUR_CIRCULAR_UPDATE,TourCB);
  SPINNER_tour_circular_center[2]=glui_tour->add_spinner_to_panel(PANEL_tour_circular_center,"z",GLUI_SPINNER_FLOAT,tour_circular_center+2,TOUR_CIRCULAR_UPDATE,TourCB);

  PANEL_tour_circular_view = glui_tour->add_panel_to_panel(ROLLOUT_circular,_("Target"),true);
  SPINNER_tour_circular_view[0]=glui_tour->add_spinner_to_panel(PANEL_tour_circular_view,"x",GLUI_SPINNER_FLOAT,tour_circular_view,TOUR_CIRCULAR_UPDATE,TourCB);
  SPINNER_tour_circular_view[1]=glui_tour->add_spinner_to_panel(PANEL_tour_circular_view,"y",GLUI_SPINNER_FLOAT,tour_circular_view+1,TOUR_CIRCULAR_UPDATE,TourCB);
  SPINNER_tour_circular_view[2]=glui_tour->add_spinner_to_panel(PANEL_tour_circular_view,"z",GLUI_SPINNER_FLOAT,tour_circular_view+2,TOUR_CIRCULAR_UPDATE,TourCB);

  ROLLOUT_keyframe = glui_tour->add_rollout("Modify general tour",true,KEYFRAME_TOURS_ROLLOUT, ToursRolloutCB);
  INSERT_ROLLOUT(ROLLOUT_keyframe, glui_tour);
  ADDPROCINFO(toursprocinfo, ntoursprocinfo, ROLLOUT_keyframe, KEYFRAME_TOURS_ROLLOUT, glui_tour);

  PANEL_tour = glui_tour->add_panel_to_panel(ROLLOUT_keyframe,"Tour", true);

  PANEL_tour1 = glui_tour->add_panel_to_panel(PANEL_tour, "", GLUI_PANEL_NONE);

  BUTTON_prev_tour = glui_tour->add_button_to_panel(PANEL_tour1, _("Previous"), TOUR_PREVIOUS, TourCB);
  glui_tour->add_button_to_panel(PANEL_tour1, _("Copy"), TOUR_INSERT_COPY, TourCB);
  BUTTON_delete_tour = glui_tour->add_button_to_panel(PANEL_tour1, _("Delete"), TOUR_DELETE, TourCB);
  glui_tour->add_column_to_panel(PANEL_tour1, false);
  BUTTON_next_tour = glui_tour->add_button_to_panel(PANEL_tour1, _("Next"), TOUR_NEXT, TourCB);
  glui_tour->add_button_to_panel(PANEL_tour1, _("New"), TOUR_INSERT_NEW, TourCB);
  glui_tour->add_button_to_panel(PANEL_tour1, _("Reverse"), TOUR_REVERSE, TourCB);

  if(ntourinfo > 0){
    selectedtour_index = TOURINDEX_MANUAL;
    selectedtour_index_old = TOURINDEX_MANUAL;
    LISTBOX_tour = glui_tour->add_listbox_to_panel(PANEL_tour, "Select: ", &selectedtour_index, TOUR_LIST, TourCB);

    LISTBOX_tour->add_item(TOURINDEX_MANUAL, "Manual");
    LISTBOX_tour->add_item(-999, "-");
    for(i = 0;i < ntourinfo;i++){
      tourdata *touri;

      touri = tourinfo + i;
      LISTBOX_tour->add_item(i, touri->label);
    }
    LISTBOX_tour->set_int_val(selectedtour_index);
    glui_tour->add_column_to_panel(PANEL_tour1, false);
  }
  EDIT_label = glui_tour->add_edittext_to_panel(PANEL_tour, "Label:", GLUI_EDITTEXT_TEXT, tour_label, TOUR_LABEL, TourCB);
  EDIT_label->set_w(200);
  glui_tour->add_button_to_panel(PANEL_tour, _("Update label"), TOUR_UPDATELABEL, TourCB);

  CHECKBOX_showtourroute1 = glui_tour->add_checkbox_to_panel(ROLLOUT_keyframe, _("Show tour"), &edittour, SHOWTOURROUTE1, TourCB);
  CHECKBOX_view1 = glui_tour->add_checkbox_to_panel(ROLLOUT_keyframe, _("View from tour"), &viewtourfrompath, VIEWTOURFROMPATH1, TourCB);
  CHECKBOX_tour_snap = glui_tour->add_checkbox_to_panel(ROLLOUT_keyframe, _("View from current tour position"), &tour_snap, TOUR_SNAP, TourCB);

  PANEL_node = glui_tour->add_panel_to_panel(ROLLOUT_keyframe, "", GLUI_PANEL_NONE);

  PANEL_tourposition = glui_tour->add_panel_to_panel(PANEL_node, _("Node"));

  SPINNER_tour_time = glui_tour->add_spinner_to_panel(PANEL_tourposition, "t:", GLUI_SPINNER_FLOAT, &glui_tour_time, KEYFRAME_tXYZ, TourCB);
#ifdef pp_TOUR
  SPINNER_tour_pause_time = glui_tour->add_spinner_to_panel(PANEL_tourposition, "pause:", GLUI_SPINNER_FLOAT, &glui_tour_pause_time, KEYFRAME_tXYZ, TourCB);
#endif
  SPINNER_x=glui_tour->add_spinner_to_panel(PANEL_tourposition,"x:",GLUI_SPINNER_FLOAT,glui_tour_xyz,  KEYFRAME_tXYZ,TourCB);
  SPINNER_y=glui_tour->add_spinner_to_panel(PANEL_tourposition,"y:",GLUI_SPINNER_FLOAT,glui_tour_xyz+1,KEYFRAME_tXYZ,TourCB);
  SPINNER_z=glui_tour->add_spinner_to_panel(PANEL_tourposition,"z:",GLUI_SPINNER_FLOAT,glui_tour_xyz+2,KEYFRAME_tXYZ,TourCB);

  PANEL_tourview = glui_tour->add_panel_to_panel(PANEL_node, _("Target"));
  SPINNER_viewx=glui_tour->add_spinner_to_panel(PANEL_tourview,"x",GLUI_SPINNER_FLOAT,glui_tour_view,  KEYFRAME_viewXYZ,TourCB);
  SPINNER_viewy=glui_tour->add_spinner_to_panel(PANEL_tourview,"y",GLUI_SPINNER_FLOAT,glui_tour_view+1,KEYFRAME_viewXYZ,TourCB);
  SPINNER_viewz=glui_tour->add_spinner_to_panel(PANEL_tourview,"z",GLUI_SPINNER_FLOAT,glui_tour_view+2,KEYFRAME_viewXYZ,TourCB);
  glui_tour->add_button_to_panel(PANEL_tourview, _("Use target at each node"),     VIEW_ALL_NODES, TourCB);
  glui_tour->add_button_to_panel(PANEL_tourview, _("Set each target using next node"), VIEW_NEXT_NODE, TourCB);

  PANEL_tournavigate = glui_tour->add_panel_to_panel(PANEL_node, "", GLUI_PANEL_NONE);

  glui_tour->add_button_to_panel(PANEL_tournavigate, _("Previous"), KEYFRAME_PREVIOUS, TourCB);
  glui_tour->add_button_to_panel(PANEL_tournavigate, _("Delete"), KEYFRAME_DELETE, TourCB);

  glui_tour->add_column_to_panel(PANEL_tournavigate, false);

  glui_tour->add_button_to_panel(PANEL_tournavigate, _("Next"), KEYFRAME_NEXT, TourCB);
  glui_tour->add_button_to_panel(PANEL_tournavigate, _("Insert after"), KEYFRAME_INSERT, TourCB);

  ROLLOUT_settings = glui_tour->add_rollout(_("Settings"), true, SETTINGS_TOURS_ROLLOUT, ToursRolloutCB);
  INSERT_ROLLOUT(ROLLOUT_settings, glui_tour);
  ADDPROCINFO(toursprocinfo, ntoursprocinfo, ROLLOUT_settings, SETTINGS_TOURS_ROLLOUT, glui_tour);

  PANEL_path = glui_tour->add_panel_to_panel(ROLLOUT_settings, _("Duration"), true);

  glui_tour->add_spinner_to_panel(PANEL_path, _("start time"), GLUI_SPINNER_FLOAT, &tour_tstart, VIEW_times, TourCB);
  glui_tour->add_spinner_to_panel(PANEL_path, _("stop time:"), GLUI_SPINNER_FLOAT, &tour_tstop, VIEW_times, TourCB);
  glui_tour->add_spinner_to_panel(PANEL_path, _("points"),     GLUI_SPINNER_INT,   &tour_ntimes, VIEW_times, TourCB);

  PANEL_misc = glui_tour->add_panel_to_panel(ROLLOUT_settings, "Misc", true);
  CHECKBOX_showintermediate = glui_tour->add_checkbox_to_panel(PANEL_misc, _("Show intermediate path nodes"), &show_path_knots);
  if(navatar_types > 0){
    glui_tour->add_checkbox_to_panel(PANEL_misc, _("Show avatar"), &show_avatar);
    LISTBOX_avatar = glui_tour->add_listbox_to_panel(PANEL_misc, _("Avatar:"), &glui_avatar_index, TOUR_AVATAR, TourCB);
    for(i = 0;i < navatar_types;i++){
      LISTBOX_avatar->add_item(i, avatar_types[i]->label);
    }
    if(tourlocus_type == 0){
      glui_avatar_index = -1;
    }
    else if(tourlocus_type == 1){
      glui_avatar_index = -2;
    }
    else{
      glui_avatar_index = iavatar_types;
    }
    LISTBOX_avatar->set_int_val(glui_avatar_index);
  }

  PANEL_close_tour = glui_tour->add_panel("",false);
  glui_tour->add_button_to_panel(PANEL_close_tour,_("Save settings"),SAVE_SETTINGS_TOUR,TourCB);
  glui_tour->add_column_to_panel(PANEL_close_tour,false);
#ifdef pp_CLOSEOFF
  GLUI_Button *BUTTON_button2 = glui_tour->add_button_to_panel(PANEL_close_tour,"Close",TOUR_CLOSE,TourCB);
  BUTTON_button2->disable();
#else
  glui_tour->add_button_to_panel(PANEL_close_tour,"Close",TOUR_CLOSE,TourCB);
#endif

  ROLLOUT_keyframe->close();
  ROLLOUT_settings->close();

  glui_tour->set_main_gfx_window( main_window );

  TourCB(VIEW1);

  UpdateTourControls();
  update_tour_list =1;
}

/* ------------------ UpdateTourList(void) ------------------------ */

extern "C" void UpdateTourList(void){

  update_tour_list =0;
  TourCB(TOUR_LIST);
}

/* ------------------ HideGluiTour ------------------------ */

extern "C" void HideGluiTour(void){
  CloseRollouts(glui_tour);
  showtour_dialog = 0;
}

/* ------------------ ShowGluiTour ------------------------ */

extern "C" void ShowGluiTour(void){
  showtour_dialog=1;
  if(glui_tour!=NULL)glui_tour->show();
  updatemenu=1;
}

/* ------------------ TrimVal ------------------------ */

extern "C" float TrimVal(float val){
  if(ABS(val)<0.000001){
    return 0.0;
  }
  else{
    return val;
  }
}

/* ------------------ UpdateGluiKeyframe ------------------------ */

extern "C" void UpdateGluiKeyframe(void){
  glui_tour_time = selected_frame->time;
  SPINNER_tour_time->set_float_val(glui_tour_time);
#ifdef pp_TOUR
  glui_tour_pause_time = selected_frame->pause_time;
  SPINNER_tour_pause_time->set_float_val(glui_tour_pause_time);
#endif
  SPINNER_x->set_float_val(glui_tour_xyz[0]);
  SPINNER_y->set_float_val(glui_tour_xyz[1]);
  SPINNER_z->set_float_val(glui_tour_xyz[2]);
}

/* ------------------ SetGluiTourKeyframe ------------------------ */

extern "C" void SetGluiTourKeyframe(void){
  tourdata *ti;
  float *eye,*xyz_view;

  if(selected_frame==NULL)return;

  ti = selected_tour;
  if(ti==NULL)return;

  tour_hide=1-ti->display;
  if(selected_tour!=NULL)strcpy(tour_label,selected_tour->label);
  glui_avatar_index=ti->glui_avatar_index;
  TourCB(TOUR_AVATAR);
  LISTBOX_avatar->set_int_val(glui_avatar_index);
  eye = selected_frame->xyz_smv;
  xyz_view = selected_frame->view_smv;

  glui_tour_time    = selected_frame->time;
#ifdef pp_TOUR
  glui_tour_pause_time = selected_frame->pause_time;
#endif
  glui_tour_xyz[0]  = TrimVal(SMV2FDS_X(eye[0]));
  glui_tour_xyz[1]  = TrimVal(SMV2FDS_Y(eye[1]));
  glui_tour_xyz[2]  = TrimVal(SMV2FDS_Z(eye[2]));
  glui_tour_view[0] = TrimVal(SMV2FDS_X(xyz_view[0]));
  glui_tour_view[1] = TrimVal(SMV2FDS_Y(xyz_view[1]));
  glui_tour_view[2] = TrimVal(SMV2FDS_Z(xyz_view[2]));
  if(SPINNER_tour_time==NULL)return;

  {
    float time_temp;

    time_temp=glui_tour_time;
    SPINNER_tour_time->set_float_limits(selected_frame->prev->time,selected_frame->next->time);
    glui_tour_time=time_temp;
    SPINNER_tour_time->set_float_val(glui_tour_time);
  }

#ifdef pp_TOUR
  SPINNER_tour_pause_time->set_float_val(glui_tour_pause_time);
#endif
  SPINNER_tour_time->set_float_val(glui_tour_time);
  SPINNER_x->set_float_val(glui_tour_xyz[0]);
  SPINNER_y->set_float_val(glui_tour_xyz[1]);
  SPINNER_z->set_float_val(glui_tour_xyz[2]);
  SPINNER_viewx->set_float_val(glui_tour_view[0]);
  SPINNER_viewy->set_float_val(glui_tour_view[1]);
  SPINNER_viewz->set_float_val(glui_tour_view[2]);
  if(CHECKBOX_tourhide!=NULL)CHECKBOX_tourhide->set_int_val(tour_hide);
  EDIT_label->set_text(tour_label);
}

/* ------------------ UpdateTourIndex ------------------------ */

extern "C" void UpdateTourIndex(void){
  update_selectedtour_index=0;
  selectedtour_index=selectedtour_index_ini;
  TourCB(TOUR_LIST);
}

/* ------------------ NextTour ------------------------ */

int NextTour(void){
  int i;

  i = selectedtour_index + 1;
  if(i > ntourinfo - 1)i = 0;
  if(i >= 0 && i < ntourinfo){
    selectedtour_index = i;
    selected_tour = tourinfo + i;
    selected_frame = selected_tour->first_frame.next;
    return 1;
  }
  return 0;
}

/* ------------------ PrevTour ------------------------ */

int PrevTour(void){
  int i;

  i = selectedtour_index - 1;
  if(i < 0)i = ntourinfo - 1;
  if(i >= 0 && i < ntourinfo){
    selectedtour_index = i;
    selected_tour = tourinfo + i;
    selected_frame = selected_tour->first_frame.next;
    return 1;
  }
  return 0;
}

/* ------------------ TourCB ------------------------ */

void TourCB(int var){
  keyframe *thiskey,*nextkey,*newframe;
  keyframe *lastkey;
  tourdata *thistour=NULL;
  float *xyz_view,*eye;
  int selectedtour_index_save;

  float key_xyz[3];
  float key_time_in, key_view[3];

  if(ntourinfo==0&&var!=TOUR_INSERT_NEW&&var!=TOUR_INSERT_COPY&&var!=TOUR_CLOSE&&var!=SAVE_SETTINGS_TOUR){
    return;
  }
  if(selected_frame!=NULL){
    thistour=selected_tour;
  }

  switch(var){
  case TOUR_CIRCULAR_UPDATE:
    if(edittour==0){
      edittour=1;
      CHECKBOX_showtourroute1->set_int_val(edittour);
      TourCB(SHOWTOURROUTE1);
      if(tour_circular_index!=-1){
        LISTBOX_tour->set_int_val(tour_circular_index);
        TourCB(TOUR_LIST);
      }
    }
    DeleteTourFrames(tourinfo);
    InitCircularTour(tourinfo,ncircletournodes,UPDATE);
    TourCB(KEYFRAME_UPDATE_ALL);
    UpdateTourMenuLabels();
    CreateTourPaths();
    UpdateTimes();
    CreateTourList();
    glutPostRedisplay();
    break;
  case KEYFRAME_UPDATE_ALL:
    {
      keyframe *frame;

      if(selected_tour == NULL)return;
      update_tour_path=0;
      for(frame=selected_tour->first_frame.next;frame->next!=NULL;frame=frame->next){
        glui_tour_xyz[0] = frame->xyz_fds[0];
        glui_tour_xyz[1] = frame->xyz_fds[1];
        glui_tour_xyz[2] = frame->xyz_fds[2];
        SPINNER_x->set_float_val(glui_tour_xyz[0]);
        SPINNER_y->set_float_val(glui_tour_xyz[1]);
        SPINNER_z->set_float_val(glui_tour_xyz[2]);
        TourCB(KEYFRAME_tXYZ);

        glui_tour_view[0] = tour_circular_view[0];
        glui_tour_view[1] = tour_circular_view[1];
        glui_tour_view[2] = tour_circular_view[2];
        SPINNER_viewx->set_float_val(glui_tour_view[0]);
        SPINNER_viewy->set_float_val(glui_tour_view[1]);
        SPINNER_viewz->set_float_val(glui_tour_view[2]);
        TourCB(KEYFRAME_tXYZ);

        TourCB(KEYFRAME_NEXT);
      }
      update_tour_path=1;
    }
    break;
  case TOUR_USECURRENT:
    break;
  case TOUR_NEXT:
    if(NextTour()==1){
      selected_tour->display=0;
      TOURMENU(selectedtour_index);
      SetGluiTourKeyframe();
    }
    break;
  case TOUR_PREVIOUS:
    if(PrevTour()==1){
      selected_tour->display=0;
      TOURMENU(selectedtour_index);
      SetGluiTourKeyframe();
    }
    break;
  case TOUR_CLOSE:
    HideGluiTour();
    break;
  case SAVE_SETTINGS_TOUR:
    WriteIni(LOCAL_INI,NULL);
    break;
  case SHOWTOURROUTE:
    if(edittour==1&&selectedtour_index<0&&ntourinfo>0){
      selectedtour_index=0;
      TourCB(TOUR_LIST);
    }
    edittour = 1 - edittour;
    TOURMENU(MENU_TOUR_SHOWDIALOG);
    UpdateTourControls();
    TourCB(VIEW1);
    updatemenu=0;
    break;
  case SHOWTOURROUTE1:
    TourCB(SHOWTOURROUTE);
    CHECKBOX_showtourroute2->set_int_val(edittour);
    break;
  case SHOWTOURROUTE2:
    TourCB(SHOWTOURROUTE);
    CHECKBOX_showtourroute1->set_int_val(edittour);
    break;
  case VIEWTOURFROMPATH1:
    TourCB(VIEWTOURFROMPATH);
    CHECKBOX_view2->set_int_val(viewtourfrompath);
    break;
  case VIEWTOURFROMPATH2:
    TourCB(VIEWTOURFROMPATH);
    CHECKBOX_view1->set_int_val(viewtourfrompath);
    break;
  case TOUR_SNAP:
    if(tour_snap==1){
      if(global_times!=NULL){
        tour_snap_time = global_times[itimes];
      }
      else{
        tour_snap_time = 0.0;
      }
    }
    break;
  case VIEWTOURFROMPATH:
    viewtourfrompath = 1 - viewtourfrompath;
    TOURMENU(MENU_TOUR_VIEWFROMROUTE);
    break;
  case VIEW1:
    CreateTourPaths();
    break;
  case VIEW_times:
    ReallocTourMemory();
    CreateTourPaths();
    UpdateTimes();
    break;
  case VIEW_ALL_NODES:
    SetKeyFrameViews(selected_frame->view_smv);
    SetGluiTourKeyframe();
    break;
  case VIEW_NEXT_NODE:
    SetKeyFrameViews(NULL);
    SetGluiTourKeyframe();
    break;
  case KEYFRAME_viewXYZ:
    if(selected_frame!=NULL){
      if(selected_tour-tourinfo==0)dirtycircletour=1;
      selected_tour->startup=0;
      xyz_view = selected_frame->view_smv;
      FDS2SMV_XYZ(xyz_view,glui_tour_view);

      if(update_tour_path==1)CreateTourPaths();
      selected_frame->selected=1;
    }
    break;
  case KEYFRAME_tXYZ:
    if(selected_frame!=NULL){
      show_tour_hint = 0;
      if(selected_tour-tourinfo==0)dirtycircletour=1;
      selected_tour->startup=0;
      eye = selected_frame->xyz_smv;
      xyz_view = selected_frame->view_smv;

      FDS2SMV_XYZ(eye,glui_tour_xyz);
      memcpy(selected_frame->xyz_fds,           glui_tour_xyz, 3*sizeof(float));
      memcpy(selected_frame->xyz_smv, eye,      3*sizeof(float));
#ifdef pp_TOUR
      selected_frame->pause_time = glui_tour_pause_time;
#endif

      FDS2SMV_XYZ(xyz_view,glui_tour_view);
      if(update_tour_path==1)CreateTourPaths();
      selected_frame->selected=1;
      TourCB(KEYFRAME_viewXYZ);
    }
    break;
  case KEYFRAME_NEXT:
    show_tour_hint = 0;
    if(selected_frame==NULL&&tourinfo!=NULL){
      selected_frame=&(tourinfo[0].first_frame);
      selected_tour=tourinfo;
    }
    if(selected_frame!=NULL){
      thistour=selected_tour;
      if(selected_frame->next!=&thistour->last_frame){
        NewSelect(selected_frame->next);
      }
      else{
        NewSelect(thistour->first_frame.next);
      }
    }
    SetGluiTourKeyframe();
    break;
  case KEYFRAME_PREVIOUS:
    show_tour_hint = 0;
    if(selected_frame==NULL&&tourinfo!=NULL){
      selected_frame=&(tourinfo[0].last_frame);
      selected_tour=tourinfo;
    }
    if(selected_frame!=NULL){
      thistour=selected_tour;
      selected_tour=thistour;
      if(selected_frame->prev!=&thistour->first_frame){
        NewSelect(selected_frame->prev);
      }
      else{
        NewSelect(thistour->last_frame.prev);
      }
    }
    SetGluiTourKeyframe();
    break;
  case KEYFRAME_INSERT:
    show_tour_hint = 0;
    if(selected_frame!=NULL){
      thistour=selected_tour;
      thiskey=selected_frame;
      nextkey=thiskey->next;
      if(nextkey==&thistour->last_frame){
        lastkey=thiskey->prev;
        key_xyz[0] = SMV2FDS_X(2*thiskey->xyz_smv[0]-lastkey->xyz_smv[0]);
        key_xyz[1] = SMV2FDS_Y(2*thiskey->xyz_smv[1]-lastkey->xyz_smv[1]);
        key_xyz[2] = SMV2FDS_Z(2*thiskey->xyz_smv[2]-lastkey->xyz_smv[2]);
        key_time_in = thiskey->time;
#ifdef pp_TOUR
        thiskey->time=(thiskey->time+thiskey->pause_time+lastkey->time)/2.0;
#else
        thiskey->time=(thiskey->time+lastkey->time)/2.0;
#endif
        key_view[0] = SMV2FDS_X(2*thiskey->view_smv[0]-lastkey->view_smv[0]);
        key_view[1] = SMV2FDS_Y(2*thiskey->view_smv[1]-lastkey->view_smv[1]);
        key_view[2] = SMV2FDS_Z(2*thiskey->view_smv[2]-lastkey->view_smv[2]);
      }
      else{
        float t_avg;

        t_avg = (thiskey->time+nextkey->time)/2.0;
        GetKeyXYZ(t_avg,  thiskey, key_xyz);
        SMV2FDS_XYZ(key_xyz, key_xyz);
        key_time_in = (thiskey->time+nextkey->time)/2.0;
        GetKeyView(t_avg, thiskey, key_view);
        SMV2FDS_XYZ(key_view, key_view);
      }
#ifdef pp_TOUR
      newframe = AddFrame(selected_frame, key_time_in, 0.0, key_xyz, key_view);
#else
      newframe=AddFrame(selected_frame, key_time_in, key_xyz, key_view);
#endif
      CreateTourPaths();
      NewSelect(newframe);
      SetGluiTourKeyframe();
    }
    break;
  case KEYFRAME_DELETE:
    show_tour_hint = 0;
    if(selected_frame!=NULL){
      selected_frame=DeleteFrame(selected_frame);
      if(selected_frame!=NULL){
        selected_frame->selected=1;
        CreateTourPaths();
      }
      else{
        if(thistour!=NULL)DeleteTour(thistour-tourinfo);
      }
    }
    break;
  case TOUR_AVATAR:
    if(selected_tour->glui_avatar_index!=glui_avatar_index){
      selected_tour->glui_avatar_index=glui_avatar_index;
// hack to make touring avatar show up
      avatar_types[glui_avatar_index]->visible=1;
      updatemenu=1;
    }
    if(glui_avatar_index==-1){
      tourlocus_type=0;
    }
    else if(glui_avatar_index==-2){
      tourlocus_type=1;
    }
    else{
      tourlocus_type=2;
      iavatar_types=glui_avatar_index;
    }
    break;
  case TOUR_UPDATELABEL:
    if(selectedtour_index>=0&&selectedtour_index<ntourinfo){
      selectedtour_index_save = selectedtour_index;
      selectedtour_index = -1;
      TourCB(TOUR_LIST);
      selectedtour_index = selectedtour_index_save;
    }
    TourCB(TOUR_LIST);
    break;
  case TOUR_LIST:
    if(selectedtour_index==-999){
      selectedtour_index=selectedtour_index_old;
      if(selectedtour_index==-999)selectedtour_index = TOURINDEX_MANUAL;
      TourCB(TOUR_LIST);
      return;
    }
    switch(selectedtour_index){
    case TOURINDEX_ALL:
      TOURMENU(MENU_TOUR_SHOWALL); // show all tours
      SetGluiTourKeyframe();
      break;
    case TOURINDEX_MANUAL:
      edittour=0;
      TOURMENU(MENU_TOUR_CLEARALL);  // reset tour vis to ini values
      if(PANEL_node != NULL)PANEL_node->disable();
      if(PANEL_tournavigate!=NULL)PANEL_tournavigate->disable();
      if (SPINNER_tour_time!= NULL)SPINNER_tour_time->disable();
      break;
    case TOURINDEX_DEFAULT:
      TOURMENU(MENU_TOUR_DEFAULT);  // default tour
      break;
    default:
      selected_tour=tourinfo + selectedtour_index;
      selected_frame=selected_tour->first_frame.next;
      selected_tour->display=0;
      TOURMENU(selectedtour_index);
      SetGluiTourKeyframe();
      if(PANEL_node != NULL)PANEL_node->enable();
      if(PANEL_tournavigate!=NULL)PANEL_tournavigate->enable();
      if (SPINNER_tour_time!= NULL)SPINNER_tour_time->disable();
      break;
    }
    DeleteTourList();
    CreateTourList();
    UpdateViewTour();
    UpdateTourControls();
    selectedtour_index_old=selectedtour_index;
    break;
  case TOUR_DELETE:
    DeleteTour(selectedtour_index);
    break;
  case TOUR_REVERSE:
    if(selectedtour_index>=0&&selectedtour_index<ntourinfo){
      ReverseTour(tourinfo[selectedtour_index].label);
    }
    break;
  case TOUR_INSERT_NEW:
  case TOUR_INSERT_COPY:
    if(var==TOUR_INSERT_NEW){
      thistour=AddTour(NULL);
    }
    else{
      if(selectedtour_index>=0&&selectedtour_index<ntourinfo){
        char label[300];

        strcpy(label, tourinfo[selectedtour_index].label);
        thistour = AddTour(label);
      }
      else{
        thistour = AddTour(NULL);
      }
    }
    if(CHECKBOX_showtourroute1 != NULL&&edittour == 0)CHECKBOX_showtourroute1->set_int_val(1);
    if(CHECKBOX_showtourroute2 != NULL&&edittour == 0)CHECKBOX_showtourroute2->set_int_val(1);
    selected_frame=thistour->first_frame.next;
    selected_tour=thistour;
    selectedtour_index = thistour - tourinfo;
    selectedtour_index_old=selectedtour_index;
    SetGluiTourKeyframe();
    CreateTourPaths();
    UpdateViewTour();
    UpdateTourControls();
    selected_tour->display=0;
    TOURMENU(selectedtour_index);
    if(PANEL_node!=NULL)PANEL_node->enable();
    if(SPINNER_tour_time!=NULL)SPINNER_tour_time->disable();
    updatemenu=1;
    break;
  case TOUR_LABEL:
    if(thistour!=NULL){
      strcpy(thistour->label,tour_label);
      SetGluiTourKeyframe();
      if(LISTBOX_tour!=NULL){
        LISTBOX_tour->delete_item(thistour-tourinfo);
        LISTBOX_tour->add_item(thistour-tourinfo,thistour->label);
      }
      UpdateTourMenuLabels();
      updatemenu=1;
    }
    break;
  case TOUR_HIDE:
    if(thistour!=NULL){
      if(tour_hide==1){
        thistour->display=1;
        TOURMENU(thistour-tourinfo);
        NextTour();
        SetGluiTourKeyframe();
        thistour->display=0;
      }
      else{
        thistour->display=1;
      }
      updatemenu=1;
      DeleteTourList();
      CreateTourList();
      UpdateViewTour();
      UpdateTourControls();
    }
    break;
  default:
    ASSERT(FFALSE);
  }
}

/* ------------------ DeleteTourList ------------------------ */

extern "C" void DeleteTourList(void){
  int i;

  if(LISTBOX_tour==NULL)return;
  for(i=0;i<ntourinfo;i++){
    LISTBOX_tour->delete_item(i);
  }
  DeleteVolTourList(); //xx comment this line if smokebot fails with seg fault
}

/* ------------------ CreateTourList ------------------------ */

extern "C" void CreateTourList(void){
  int i;

  if(LISTBOX_tour==NULL)return;
  for(i=0;i<ntourinfo;i++){
    tourdata *touri;
    char label[1000];

    touri = tourinfo + i;
    strcpy(label,"");
    if(i==selectedtour_index)strcat(label,"*");
    if(strlen(touri->label)>0)strcat(label,touri->label);
    if(strlen(label)>0){
      LISTBOX_tour->add_item(i,label);
    }
    else{
      LISTBOX_tour->add_item(i,"error");
    }
  }
  if(selectedtour_index>=-1&&selectedtour_index<ntourinfo)LISTBOX_tour->set_int_val(selectedtour_index);

  CreateVolTourList(); //xx comment this line if smokebot fails with seg fault
}

/* ------------------ UpdateTourControls ------------------------ */

extern "C" void UpdateTourControls(void){

  if(BUTTON_next_tour==NULL)return;
  if(BUTTON_prev_tour==NULL)return;
  if(ROLLOUT_keyframe==NULL)return;
  if(SPINNER_x==NULL)return;
  if(CHECKBOX_showtourroute1 != NULL)CHECKBOX_showtourroute1->set_int_val(edittour);
  if(CHECKBOX_showtourroute2 != NULL)CHECKBOX_showtourroute2->set_int_val(edittour);
  if(CHECKBOX_view1!=NULL)CHECKBOX_view1->set_int_val(viewtourfrompath);
  if(CHECKBOX_view2!=NULL)CHECKBOX_view2->set_int_val(viewtourfrompath);
  if(ntourinfo>1){
    BUTTON_next_tour->enable();
    BUTTON_prev_tour->enable();
  }
  else{
    BUTTON_next_tour->disable();
    BUTTON_prev_tour->disable();
  }

  if(CHECKBOX_tourhide!=NULL){
    if(viewanytours>0&&edittour==1){
      CHECKBOX_tourhide->enable();
    }
    else{
      CHECKBOX_tourhide->disable();
    }
  }
  if(selected_tour!=NULL){
    selectedtour_index = selected_tour-tourinfo;
    LISTBOX_tour->set_int_val(selectedtour_index);
  }
  else{
    selectedtour_index = TOURINDEX_MANUAL;
    LISTBOX_tour->set_int_val(selectedtour_index);
  }
}
