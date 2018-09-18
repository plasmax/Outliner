
#include "DDImage/Op.h"
#include "DDImage/Knob.h"
#include "DDImage/Knobs.h"
#include "DDImage/GeoOp.h"
#include "DDImage/Scene.h"
#include "DDImage/gl.h"
#include "DDImage/ViewerContext.h"
#include "DDImage/TableKnobI.h"     //Defines the table knob interface used by, surprisingly enough, the Table_knob.
#include <iostream>


using namespace DD::Image;

static const char* const CLASS = "Outliner";
static const char* const HELP = 
"Filter and Transform Objects in a Scene. Selecting the object "
"in the SceneView Knob selects it in the Scene, and vice-versa."
"Additionally, object transformations can be parented in the SceneView"
"And arbitrary transform handles can be added in the view."
"And selecting multiple pieces of geometry allows you to move them all at once";



class Outliner : public GeoOp {

public:
  static const Op::Description description;

  Outliner(Node* node) : GeoOp(node) {}

  const char* Class() const { return CLASS; }
  const char* node_help() const { return HELP; }
  const char* node_shape() const { return "O"; }
  int maximum_inputs() const { return -1; }

  virtual bool inputs_clockwise() const { return false; }

  void validate(bool for_real) {
    GeoOp::validate(for_real);

    // for (int i = 0; i <= inputs(); i++) {
    //   if (node_input(i, Op::OUTPUT_OP)) {
    //     GeoOp* gop = input(i);
    //     Scene& gop_scene = *gop->scene();
    //     GeometryList& list = *gop_scene.object_list();

    //     GeometryList& geo_list = *scene_->object_list();

    //     for (unsigned o = 0; o < list.size(); o++) {
    //       GeoInfo& info = list[o];
    //       // geo_list.add_object(int(o));
    //       }
    //     }
    //   }
    }

  void knobs(Knob_Callback f) {
    GeoOp::knobs(f);

    //Handles data storage and mem management itself. Needs to have relevant
    //cols added on creation only (hence f.makeKnobs check).
    Knob* tableKnob = Table_knob(f, "Table_knob");
    if (f.makeKnobs()) {
      Table_KnobI* tableKnobI = tableKnob->tableKnob();
      tableKnobI->addColumn("name", "name", Table_KnobI::FloatColumn, true);
      tableKnobI->addColumn("hash", "hash", Table_KnobI::FloatColumn, true);
      
      if(tableKnobI->getRowCount() == 0) {
        tableKnobI->addRow();
      }
    }

  }

  void get_geometry_hash() {
    GeoOp::get_geometry_hash();
    geo_hash[Group_Points].append(Op::hash());
    // hash the knobs
    }

  void geometry_engine(Scene& scene, GeometryList& out) {
    GeoOp::geometry_engine(scene, out);

    for (int i = 0; i <= inputs(); i++) {
      if (node_input(i, Op::OUTPUT_OP)) {
        input(i)->get_geometry(scene, out);
        }
      }

    out.synchronize_objects();
    }

  HandlesMode doAnyHandles(ViewerContext* ctx) {
    if (ctx->transform_mode() != VIEWER_2D)
      return eHandlesCooked;

    return Op::doAnyHandles(ctx);
    }

  void build_handles(ViewerContext* ctx) {
    if (ctx->transform_mode() == VIEWER_2D) return;
    if (ctx->viewer_mode() != VIEWER_2D && !node_disabled()) {
      if (ctx->connected() >= SHOW_OBJECT) {
        validate(false);
        add_draw_geometry(ctx); // draw wireframes

        ctx->connected(CONNECTED); // do not pass through objects
        }
      }

    for (int i = 0; i <= inputs(); i++) {
      if (node_input(i, Op::OUTPUT_OP)) {
        add_input_handle(i, ctx); // calls build_handles on input ops
        }
      }

    build_knob_handles(ctx);
    add_draw_handle(ctx);

    }
  
  void draw_handle(ViewerContext* ctx) {
    GeoOp::draw_handle(ctx);

    // GeometryList& out = *scene_->object_list();
    // for (unsigned obj = 0; obj < out.size(); obj++) {
    //   GeoInfo& info = out[obj];
    //   info.selected = true;
    //   }

    GeometryList& out = *scene_->object_list();
    for (int i = 0; i <= inputs(); i++) {
      if (node_input(i, Op::OUTPUT_OP)) {
        GeoOp* gop = input(i);
        Scene& gop_scene = *gop->scene();
        GeometryList& list = *gop_scene.object_list();
        for (unsigned obj = 0; obj < out.size(); obj++) {
          GeoInfo& info = out[obj];
          std::cout << info.src_id() << std::endl;
          info.selected = true;
          }
        }
      }
  }
};

static Op* build(Node *node) {
    return new Outliner(node); 
}

const Op::Description Outliner::description(CLASS, 0, build);
