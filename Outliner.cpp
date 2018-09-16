
#include "DDImage/Op.h"
#include "DDImage/Knob.h"
#include "DDImage/GeoOp.h"
#include "DDImage/Scene.h"
#include "DDImage/gl.h"
#include "DDImage/ViewerContext.h"

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
  ViewerContext* myctx;

public:
  static const Op::Description description;

  Outliner(Node* node) : GeoOp(node) {
    myctx = NULL;
  }

  const char* Class() const { return CLASS; }
  const char* node_help() const { return HELP; }
  const char* node_shape() const { return "O"; }
  int maximum_inputs() const { return -1; }

  virtual bool inputs_clockwise() const { return false; }

  void get_geometry_hash() {
    GeoOp::get_geometry_hash();
    geo_hash[Group_Points].append(Op::hash());
    }

  void geometry_engine(Scene& scene, GeometryList& out) {
    GeoOp::geometry_engine(scene, out);

    for (int i = 0; i <= inputs(); i++) {
      if (!node_input(i, Op::OUTPUT_OP) == NULL) {
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
    myctx = ctx;

    if (ctx->transform_mode() == VIEWER_2D) return;
    if (ctx->viewer_mode() != VIEWER_2D && !node_disabled()) {
      if (ctx->connected() >= SHOW_OBJECT) {
        validate(false);
        add_draw_geometry(ctx); // draw wireframes
        // ctx->connected(CONNECTED); // do not pass through objects
        }
      }

    for (int i = 0; i <= inputs(); i++) {
      if (!node_input(i, Op::OUTPUT_OP) == NULL) {
        add_input_handle(i, ctx); // calls build_handles on input ops
      }
    }

    build_knob_handles(ctx);

    add_draw_handle(ctx);

    }
};

static Op* build(Node *node) {
    return new Outliner(node); 
}

const Op::Description Outliner::description(CLASS, 0, build);

