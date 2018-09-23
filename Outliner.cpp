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

// the goal (more or less)
static const char* const HELP =
"Highlight, Filter and Transform Objects in a Scene. Selecting the object "
"in the Table_knob selects it in the Scene, and vice-versa."
"And selecting multiple pieces of geometry allows you to move them all at once";


class Outliner : public GeoOp {
private:
  bool                       _table_initialized;
  std::map<Hash, Matrix4>    _matrixlookup;
  Matrix4                    _local;

  static void draw_indicators_cb(void* p, ViewerContext* ctx) {
    ((Outliner*)p)->draw_indicators(ctx);
  };

public:
  static const Op::Description description;

  Outliner(Node* node) : GeoOp(node) {
    _table_initialized = false;
    _local.makeIdentity();
  }

  const char* Class() const { return CLASS; }
  const char* node_help() const { return HELP; }
  const char* node_shape() const { return "O"; }
  int minimum_inputs() const { return 1; }
  int maximum_inputs() const { return -1; }
  virtual bool inputs_clockwise() const { return false; }

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

};


void Outliner::knobs(Knob_Callback f) {
  GeoOp::knobs(f);

  Knob* tableKnob = Table_knob(f, "Table_knob");
  if (f.makeKnobs()) {
    Table_KnobI* tableKnobI = tableKnob->tableKnob();
    tableKnobI->addColumn("src_id", "src_id", Table_KnobI::FloatColumn, false);
  }

  Axis_knob(f, &_local, "Axis_knob");
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
    if (op) { op->validate(for_real); }
  }
  GeoOp::_validate(for_real);
}


void Outliner::get_geometry_hash() {
  GeoOp::get_geometry_hash();
  geo_hash[Group_Points].append(Op::hash());

  // the local transform
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
    }
  }

  out.synchronize_objects();

  // Apply transformations here (_local for now, but the goal is per object)
  for (unsigned o = 0; o < out.size(); o++) {
    GeoInfo& info = out[o];
    info.matrix = _local * info.matrix;
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

      ctx->connected(CONNECTED); // do not pass through objects

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

  // Don't draw anything unless viewer is in 3d mode:
  if (ctx->transform_mode() == VIEWER_2D)
    return;

  // make it call draw_handle():
  add_draw_handle(ctx);

  }


void Outliner::select_geo_from_table(ViewerContext* ctx) {

  Knob* tableKnob = knob("Table_knob");
  Table_KnobI* tk = tableKnob->tableKnob();
  std::vector<int> selrows = tk->getSelectedRows();

  GeometryList& out = *scene_->object_list();
  for (unsigned o = 0; o <= out.size(); o++) {
    GeoInfo& info = out[o];
    if (!info.selectable) {
      continue;
    }

    Hash hash = info.src_id();

    bool selected_in_table = false;
    for (unsigned ts = 0; ts < selrows.size(); ts++) {
      int sr = selrows[ts];
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


bool Outliner::draw_indicators(ViewerContext* ctx) {
  if (ctx->transform_mode() == VIEWER_2D) return false;
  if (ctx->viewer_mode() == VIEWER_2D) return false;
  if (display3d_ == DISPLAY_OFF) return false;

  // looking for a better method tham node_selected to differentiate.
  if (node_selected()) {
    select_geo_from_table(ctx);
  } else {
    select_table_from_geo(ctx);
  }

  // just for fun, draw the current active bounding box. this actually seems to force it to update, giving a more accurate result when pressing 'f'
  Box3& cbx = ctx->active_bbox();
  glColor(2332084479);
  gl_boxf(cbx.x(), cbx.y(), cbx.n(), cbx.r(), cbx.t(), cbx.f());

  return false; // we're not interested in *every* event
}


static Op* build(Node *node) {
    return new Outliner(node);
}

const Op::Description Outliner::description(CLASS, 0, build);
