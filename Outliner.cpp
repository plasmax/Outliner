#include <iostream>
#include <stdio.h>


#include "DDImage/Op.h"
#include "DDImage/Knob.h"
#include "DDImage/Knobs.h"
#include "DDImage/GeoOp.h"
#include "DDImage/Scene.h"
#include "DDImage/gl.h"
#include "DDImage/ViewerContext.h"
#include "DDImage/TableKnobI.h"
#include "DDImage/Matrix4.h"

using namespace DD::Image;

static const char* const CLASS = "Outliner";
static const char* const HELP =
"Filter and Transform Objects in a Scene. Selecting the object "
"in the SceneView Knob selects it in the Scene, and vice-versa."
"Additionally, object transformations can be parented in the SceneView"
"And arbitrary transform handles can be added in the view."
"And selecting multiple pieces of geometry allows you to move them all at once";


class Outliner : public GeoOp {
private:
  bool                       _table_initialized, _select_from_viewport, _selecting;
  std::map<Hash, Matrix4>    _matrixlookup;
  Matrix4                    _local;

  static void draw_indicators_cb(void* p, ViewerContext* ctx) {
    ((Outliner*)p)->draw_indicators(ctx);
  };


public:
  static const Op::Description description;

  Outliner(Node* node) : GeoOp(node) {
    _table_initialized = false;
    _select_from_viewport = false;
    _selecting = false;
    _local.makeIdentity();
  }

  const char* Class() const { return CLASS; }
  const char* node_help() const { return HELP; }
  const char* node_shape() const { return "O"; }
  int minimum_inputs() const { return 1; }
  int maximum_inputs() const { return -1; }
  virtual bool inputs_clockwise() const { return false; }
  // bool select_from_viewport() { return _select_from_viewport; }

  void knobs(Knob_Callback f);
  int knob_changed(Knob* k);

  void _validate(bool for_real);
  void get_geometry_hash();
  void geometry_engine(Scene& scene, GeometryList& out);

  void build_handles(ViewerContext* ctx);

  void init_table();
  void select_geo_from_table(ViewerContext* ctx);
  void select_table_from_geo(ViewerContext* ctx);
  bool draw_indicators(ViewerContext* ctx);
  bool set_selection_zone(ViewerContext* ctx);
  void print_event_type(ViewerContext* ctx);
};




class GlueKnob : public Knob
{
  Outliner* _outliner;
  const char* Class() const { return "Glue"; }
  static bool handle_cb(ViewerContext* ctx, Knob* knob, int index)
  {
    return ((GlueKnob*)knob)->_outliner->set_selection_zone(ctx);//, index);
  }

public:
  void draw_handle(ViewerContext* ctx) {
    begin_handle(Knob::ANYWHERE_MOUSEMOVES, ctx, handle_cb, 0 /*index*/, 0, 0, 0 /*xyz*/);
    // begin_handle(Knob::ANYWHERE, ctx, handle_cb, 0 /*index*/, 0, 0, 0 /*xyz*/);
    end_handle(ctx);
  }

  bool build_handle(ViewerContext* ctx) {
    return true;
  }

  GlueKnob(Knob_Closure* kc, Outliner* o, const char* n) : Knob(kc, n)
  {
    _outliner = o;
  }

};
























void Outliner::knobs(Knob_Callback f) {
  GeoOp::knobs(f);

  Knob* tableKnob = Table_knob(f, "Table_knob");
  if (f.makeKnobs()) {
    Table_KnobI* tableKnobI = tableKnob->tableKnob();
    tableKnobI->addColumn("src_id", "src_id", Table_KnobI::FloatColumn, false);
  }

  CustomKnob1(GlueKnob, f, this, "kludge");

  Axis_knob(f, &_local, "Axis_knob");
  Bool_knob(f, &_select_from_viewport, "In_Viewer");
  SetFlags(f, Knob::ALWAYS_SAVE);
  SetFlags(f, Knob::INVISIBLE);
}


int Outliner::knob_changed(Knob* k) {
  if (k->name() == "inputChange") {
    _table_initialized = false;
    return 1;
  }
  if (k->name() == "showPanel") {
    _table_initialized = false;
    return 1;
  }
  return 0;
}














void Outliner::_validate(bool for_real) {
  update_geometry_hashes();
  for (int i = 0; i < inputs(); i++) {
    GeoOp* op = dynamic_cast<GeoOp*>(input(i));
    if (op) {
      op->validate(for_real);
      }
    }
  GeoOp::_validate(for_real);
  }


void Outliner::get_geometry_hash() {
  //References geo_hash, DD::Image::Hash::reset(), and DD::Image::Op::validate().
  GeoOp::get_geometry_hash();
  geo_hash[Group_Points].append(Op::hash());


  // the local transform - must hash for transforms to work
  geo_hash[Group_Matrix].append(_local.a00);
  geo_hash[Group_Matrix].append(_local.a01);
  geo_hash[Group_Matrix].append(_local.a02);
  geo_hash[Group_Matrix].append(_local.a03);

  geo_hash[Group_Matrix].append(_local.a10);
  geo_hash[Group_Matrix].append(_local.a11);
  geo_hash[Group_Matrix].append(_local.a12);
  geo_hash[Group_Matrix].append(_local.a13);

  geo_hash[Group_Matrix].append(_local.a20);
  geo_hash[Group_Matrix].append(_local.a21);
  geo_hash[Group_Matrix].append(_local.a22);
  geo_hash[Group_Matrix].append(_local.a23);

  geo_hash[Group_Matrix].append(_local.a30);
  geo_hash[Group_Matrix].append(_local.a31);
  geo_hash[Group_Matrix].append(_local.a32);
  geo_hash[Group_Matrix].append(_local.a33);

  // // NOTE: probs wanna hash the table_knob if we're going to transform with xyz columns ...

  for (int i = 0; i <= inputs(); i++) {
    if (node_input(i, Op::OUTPUT_OP) != NULL) {
      GeoOp* op = input(i);
      op->validate(true);

      geo_hash[Group_Points].append(input(i)->hash(Group_Points));
      geo_hash[Group_Primitives].append(input(i)->hash(Group_Primitives));
      geo_hash[Group_Matrix].append(input(i)->hash(Group_Matrix));
      geo_hash[Group_Attributes].append(input(i)->hash(Group_Attributes));
    }
  }
}

void Outliner::geometry_engine(Scene& scene, GeometryList& out) {

  for (int i = 0; i <= inputs(); i++) {
    if (dynamic_cast<GeoOp*>(input(i))){
      GeoOp* op = input(i);
      op->get_geometry(scene, out);
      op->print_info(std::cout); // test

    }
  }

  out.synchronize_objects();

  // Apply transformations here (_local for now, but the goal is per object)
  for (unsigned o = 0; o < out.size(); o++) {
    GeoInfo& info = out[o];
    info.matrix = _local * info.matrix;
    info.print_info(std::cout); // test
  }

}
























/* fill the table with a row per GeoInfo in the scene.*/
void Outliner::init_table() {
  GeometryList& out = *scene_->object_list();

  Knob* tableKnob = knob("Table_knob");
  Table_KnobI* tk = tableKnob->tableKnob();

  tk->suspendKnobChangedEvents();

  // clear all items
  while (tk->getRowCount()!=0) {
    tk->deleteRow(0);
  }

  int src_column = tk->getColumnIndex("src_id");
  std::vector<int> rows;

  for ( unsigned o = 0; o < out.size(); o++ ) {
    GeoInfo& info = out[o];
    tk->addRow(o);

    int src_id = int(info.src_id().value());
    tk->setCellFloat(o, src_column, float(src_id));
    if (!info.selectable) { continue; }
    if (info.selected) {
      rows.push_back(int(o));
    }
  }

  // highlight currently selected geometry in table
  tk->selectRows(rows);

  tk->resumeKnobChangedEvents(true);
}





void Outliner::build_handles(ViewerContext* ctx) {

  if (ctx->transform_mode() == VIEWER_2D) return;
  if (ctx->viewer_mode() == VIEWER_2D) return;

  if (ctx->viewer_mode() != VIEWER_2D && !node_disabled()) {
    if (ctx->connected() >= SHOW_OBJECT) {
      validate(false);

      // construct output geometry and add callbacks to draw it in the viewer.
      add_draw_geometry(ctx);

      // ctx->connected(CONNECTED); // do not pass through objects

      ctx->add_draw_handle(draw_indicators_cb, this, node(), eDrawHandleAlways);
      }
    }

  for (int i = 0; i <= inputs(); i++) {
    if (!node_input(i, Op::OUTPUT_OP)) {
      add_input_handle(i, ctx); // calls build_handles on input ops
      }
    }

  build_knob_handles(ctx);

  if (!_table_initialized) {
    init_table();
    _table_initialized = true;
    }

  // make it call draw_handle():
  add_draw_handle(ctx);

  // Add our volume to the bounding box, so 'f' works to include this object:
  // get union bbox here for all selected objects - but this will also need to be updated I think
  // ctx->expand_bbox(node_selected(), x, y, z);
  // ctx->expand_bbox(node_selected(), -x, -y, -z);

}


void Outliner::select_geo_from_table(ViewerContext* ctx) {

  Knob* tableKnob = knob("Table_knob");
  Table_KnobI* tk = tableKnob->tableKnob();
  std::vector<int> selrows = tk->getSelectedRows();

  Box3 bbox = Box3(0, 0, 0);

  GeometryList& out = *scene_->object_list();
  for (unsigned o = 0; o <= out.size(); o++) {
    GeoInfo& info = out[o];
    if (!info.selectable) {
      continue;
    }

    Hash hash = info.src_id();

    bool selected_in_table = false;
    for (unsigned i = 0; i < selrows.size(); i++) {
      int sr = selrows[i];
      if (sr == int(o)) {
        selected_in_table = true;
      }
    }
    info.selected = selected_in_table;
  }

}


void Outliner::select_table_from_geo(ViewerContext* ctx) {
  Knob* tableKnob = knob("Table_knob");
  Table_KnobI* tk = tableKnob->tableKnob();

  tk->suspendKnobChangedEvents();
  int sel_column = tk->getColumnIndex("selected");

  GeometryList& out = *scene_->object_list();

  std::set<int> set;
  std::vector<int> rows;
  for (unsigned o = 0; o <= out.size(); o++) {
    GeoInfo& info = out[o];
    if (!info.selectable) { continue; }

    tk->setCellBool(o, sel_column, info.selected);
    if (info.selected) {
      rows.push_back(int(o));
      set.insert(int(o));
    }
  }
  tk->selectRows(rows);
  tk->resumeKnobChangedEvents(false);

}


bool Outliner::set_selection_zone(ViewerContext* ctx) {
  switch (ctx->event()) {
    case ENTER_VIEWER:
      glColor(1373335295); // green
      knob("In_Viewer")->set_value(true);
      break;
    case EXIT_VIEWER:
      glColor(3674210559); //red
      knob("In_Viewer")->set_value(false);
      break;
  }
  return false;
}



bool Outliner::draw_indicators(ViewerContext* ctx) {

  if (ctx->transform_mode() == VIEWER_2D) return false;
  if (ctx->viewer_mode() == VIEWER_2D) return false;
  if (display3d_ == DISPLAY_OFF) return false;

  // extra extra read all about it
  DD::Image::Display3DMode display3d = ctx->display3d(this->display3d());
  if (display3d == DISPLAY_OFF) return false;

  if (ctx->event() == KEY) {
    std::cout << "key event:" << ctx->key() << std::endl;
  }

  if (!_select_from_viewport) {
    select_geo_from_table(ctx);
  }

  if (_select_from_viewport) {
    select_table_from_geo(ctx);
  }


  // if (_select_from_viewport) {
  //   select_table_from_geo(ctx);
  // } else {
  //   select_geo_from_table(ctx);
  // }

  // just for fun, draw the current active bounding box.
  Box3& cbx = ctx->active_bbox();
  glColor(2332084479);
  gl_boxf(cbx.x(), cbx.y(), cbx.n(), cbx.r(), cbx.t(), cbx.f());

  return true; // we're not interested in *every* event
  // return false; // we're not interested in *every* event
}



void Outliner::print_event_type(ViewerContext* ctx) {

  switch (ctx->event()) {
    case NO_EVENT:
      std::cout << "NO_EVENT, event index: ";
      std::cout << ctx->event() << std::endl;
    case DRAW_OPAQUE:
      std::cout << "DRAW_OPAQUE, event index: ";
      std::cout << ctx->event() << std::endl;
    case DRAW_TRANSPARENT:
      std::cout << "DRAW_TRANSPARENT, event index: ";
      std::cout << ctx->event() << std::endl;
    case DRAW_STIPPLED:
      std::cout << "DRAW_STIPPLED, event index: ";
      std::cout << ctx->event() << std::endl;
    case DRAW_SHADOW:
      std::cout << "DRAW_SHADOW, event index: ";
      std::cout << ctx->event() << std::endl;
    case DRAW_LINES:
      std::cout << "DRAW_LINES, event index: ";
      std::cout << ctx->event() << std::endl;
    // case MOVE:
      // std::cout << "MOVE, event index: ";
      // std::cout << ctx->event() << std::endl;
    case PUSH:
      std::cout << "PUSH, event index: ";
      std::cout << ctx->event() << std::endl;
    case DRAG:
      std::cout << "DRAG, event index: ";
      std::cout << ctx->event() << std::endl;
    case RELEASE:
      std::cout << "RELEASE, event index: ";
      std::cout << ctx->event() << std::endl;
    case KEY:
      std::cout << "KEY, event index: ";
      std::cout << ctx->event() << std::endl;
    case KEYUP:
      std::cout << "KEYUP, event index: ";
      std::cout << ctx->event() << std::endl;
    case DROP:
      std::cout << "DROP, event index: ";
      std::cout << ctx->event() << std::endl;
    case DROP_CHECK:
      std::cout << "DROP_CHECK, event index: ";
      std::cout << ctx->event() << std::endl;
    case ENTER_VIEWER:
      std::cout << "ENTER_VIEWER, event index: ";
      std::cout << ctx->event() << std::endl;
    case EXIT_VIEWER:
      std::cout << "EXIT_VIEWER, event index: ";
      std::cout << ctx->event() << std::endl;
    case CURSOR:
      std::cout << "CURSOR, event index: ";
      std::cout << ctx->event() << std::endl;
    case HOVER_ENTER:
      std::cout << "HOVER_ENTER, event index: ";
      std::cout << ctx->event() << std::endl;
    case HOVER_MOVE:
      std::cout << "HOVER_MOVE, event index: ";
      std::cout << ctx->event() << std::endl;
    case HOVER_LEAVE:
      std::cout << "HOVER_LEAVE, event index: ";
      std::cout << ctx->event() << std::endl;
    case DRAG_SELECT:
      std::cout << "DRAG_SELECT, event index: ";
      std::cout << ctx->event() << std::endl;
    case DRAG_SELECT_FINISHED:
      std::cout << "DRAG_SELECT_FINISHED, event index: ";
      std::cout << ctx->event() << std::endl;
    case FIRST_MENU:
      std::cout << "FIRST_MENU, event index: ";
      std::cout << ctx->event() << std::endl;
  }
}


static Op* build(Node *node) {
    return new Outliner(node);
}

const Op::Description Outliner::description(CLASS, 0, build);
