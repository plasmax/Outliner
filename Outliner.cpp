
#include "DDImage/Op.h"
#include "DDImage/Knob.h"
#include "DDImage/Knobs.h"
#include "DDImage/GeoOp.h"
#include "DDImage/Scene.h"
#include "DDImage/gl.h"
#include "DDImage/ViewerContext.h"
#include "DDImage/TableKnobI.h"

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
  bool _table_updated;

public:
  static const Op::Description description;

  Outliner(Node* node) : GeoOp(node) {
    _table_updated = false;
  }

  const char* Class() const { return CLASS; }
  const char* node_help() const { return HELP; }

  void knobs(Knob_Callback f) {
    //Handles data storage and mem management itself. Needs to have relevant
    //cols added on creation only (hence f.makeKnobs check).
    Knob* tableKnob = Table_knob(f, "Table_knob");
    if (f.makeKnobs()) {
      Table_KnobI* tableKnobI = tableKnob->tableKnob();
      tableKnobI->addStringColumn("geo_names", "Geo Names", true, 170, false, true);
      tableKnobI->addColumn("geo_hashes", "geo_hashes", Table_KnobI::StringColumn, true);
    }
  }

  void update_table() {
    std::cout << "updating table" << std::endl;
    GeometryList& out = *scene_->object_list();

    Knob* tableKnob = knob("Table_knob");
    Table_KnobI* tk = tableKnob->tableKnob();

    int numobjs = out.size();
    std::cout << "out:" << numobjs << std::endl;
    for (int r = 0; r < tk->getRowCount(); r++ ) {
      tk->deleteRow(r);
      }

    for ( unsigned obj = 0; obj < out.size(); obj++ ) {
      GeoInfo& info = out[obj];
      const Hash hash = info.src_id();
      tk->addRow(obj);
      std::string stringhash = std::to_string(hash.value());
      tk->setCellString(obj, 1, stringhash);
      }

  }

  void build_handles(ViewerContext* ctx) {
    if (ctx->transform_mode() == VIEWER_2D) return;
    if (ctx->viewer_mode() != VIEWER_2D && !node_disabled()) {
      if (ctx->connected() >= SHOW_OBJECT) {
        validate(false);

        // construct output geometry and add callbacks to draw it in the viewer.
        add_draw_geometry(ctx);

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

    if (!_table_updated) {
      update_table();
      _table_updated = true;
      }

    }


  // Not used but potentially very useful
  // (perhaps a more constant ID would be better)
  void select_by_hash(Hash& hash) {
    GeometryList& out = *scene_->object_list();
    for (unsigned o = 0; o <= out.size(); o++) {
      GeoInfo& info = out[o];
      Hash obj_hash = info.src_id();
      if (hash.value() == obj_hash.value()) {
        info.selected = true; // my favourite line
      }
    }
  }

  void highlight() {
    Knob* tableKnob = knob("Table_knob");
    Table_KnobI* tk = tableKnob->tableKnob();
    std::vector<int> selrows = tk->getSelectedRows();
    std::cout << "selrows" << std::endl;
    for (unsigned i = 0; i < selrows.size(); i++) {
      std::cout << selrows[i] << std::endl;
    }

    GeometryList& out = *scene_->object_list();
    for (unsigned o = 0; o <= out.size(); o++) {
      GeoInfo& info = out[o];

      Hash hash = info.src_id();
      for (unsigned i = 0; i < selrows.size(); i++) {
        int sr = selrows[i];
        std::cout << sr << std::endl;
        if (sr == int(o)) {
          info.selected = true; // my favourite line
        }
      }
    }
  }

  void draw_handle(ViewerContext* ctx) {
    GeoOp::draw_handle(ctx);
    highlight();

  }

  int knob_changed(Knob* k) {
    std::cout << k->name() << std::endl;
    if (k->name() == "inputChange") {
      _table_updated = false;
      return 1;
    }
    if (k->name() == "showPanel") {
      _table_updated = false;
      return 1;
    }
    return 0;
  }
};

static Op* build(Node *node) {
    return new Outliner(node);
}

const Op::Description Outliner::description(CLASS, 0, build);
