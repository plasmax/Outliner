
// #define _SECURE_SCL 0

// #include <string>
// #include <sstream>

// namespace patch
// {
//     template < typename T > std::string to_string( const T& n )
//     {
//         std::ostringstream stm ;
//         stm << n ;
//         return stm.str() ;
//     }
// }

// #include <iostream>


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
      // std::string stringhash = patch::to_string(hash.value());
      // tk->setCellString(obj, 1, stringhash);
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
      if (!node_input(i, Op::OUTPUT_OP)) {
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
      if (!info.selectable) {
        continue;
      }

      Hash hash = info.src_id();
      for (unsigned i = 0; i < selrows.size(); i++) {
        int sr = selrows[i];
        std::cout << sr << std::endl;
        if (sr == int(o)) {
          info.selected = true; // my favourite line
        } else { info.selected = false; }
      }
    }
  }

  void draw_handle(ViewerContext* ctx) {
    GeoOp::draw_handle(ctx);

    // if (ctx->event() == DRAW_OPAQUE)
    // std::cout << "ctx.event: ";
    // std::cout << ctx->event() << std::endl;

    switch (ctx->event()) {
      case NO_EVENT:
        std::cout << "NO_EVENT, event index: ";
        std::cout << ctx->event() << std::endl;
        break;
      case DRAW_OPAQUE:
        std::cout << "DRAW_OPAQUE, event index: ";
        std::cout << ctx->event() << std::endl;
        break;
      case DRAW_TRANSPARENT:
        std::cout << "DRAW_TRANSPARENT, event index: ";
        std::cout << ctx->event() << std::endl;
        break;
      case DRAW_STIPPLED:
        std::cout << "DRAW_STIPPLED, event index: ";
        std::cout << ctx->event() << std::endl;
        break;
      case DRAW_SHADOW:
        std::cout << "DRAW_SHADOW, event index: ";
        std::cout << ctx->event() << std::endl;
        break;
      case DRAW_LINES:
        std::cout << "DRAW_LINES, event index: ";
        std::cout << ctx->event() << std::endl;
        break;
      case MOVE:
        std::cout << "MOVE, event index: ";
        std::cout << ctx->event() << std::endl;
        break;
      case PUSH:
        std::cout << "PUSH, event index: ";
        std::cout << ctx->event() << std::endl;
        break;
      case DRAG:
        std::cout << "DRAG, event index: ";
        std::cout << ctx->event() << std::endl;
        break;
      case RELEASE:
        std::cout << "RELEASE, event index: ";
        std::cout << ctx->event() << std::endl;
        break;
      case KEY:
        std::cout << "KEY, event index: ";
        std::cout << ctx->event() << std::endl;
        break;
      case KEYUP:
        std::cout << "KEYUP, event index: ";
        std::cout << ctx->event() << std::endl;
        break;
      case DROP:
        std::cout << "DROP, event index: ";
        std::cout << ctx->event() << std::endl;
        break;
      case DROP_CHECK:
        std::cout << "DROP_CHECK, event index: ";
        std::cout << ctx->event() << std::endl;
        break;
      case ENTER_VIEWER:
        std::cout << "ENTER_VIEWER, event index: ";
        std::cout << ctx->event() << std::endl;
        break;
      case EXIT_VIEWER:
        std::cout << "EXIT_VIEWER, event index: ";
        std::cout << ctx->event() << std::endl;
        break;
      case CURSOR:
        std::cout << "CURSOR, event index: ";
        std::cout << ctx->event() << std::endl;
        break;
      case HOVER_ENTER:
        std::cout << "HOVER_ENTER, event index: ";
        std::cout << ctx->event() << std::endl;
        break;
      case HOVER_MOVE:
        std::cout << "HOVER_MOVE, event index: ";
        std::cout << ctx->event() << std::endl;
        break;
      case HOVER_LEAVE:
        std::cout << "HOVER_LEAVE, event index: ";
        std::cout << ctx->event() << std::endl;
        break;
      case DRAG_SELECT:
        std::cout << "DRAG_SELECT, event index: ";
        std::cout << ctx->event() << std::endl;
        break;
      case DRAG_SELECT_FINISHED:
        std::cout << "DRAG_SELECT_FINISHED, event index: ";
        std::cout << ctx->event() << std::endl;
        break;
      case FIRST_MENU:
        std::cout << "FIRST_MENU, event index: ";
        std::cout << ctx->event() << std::endl;
        break;
    }
    if (   ctx->event() == PUSH
        || ctx->event() == DRAG
        || ctx->event() == RELEASE
        || ctx->event() == DRAG_SELECT
        || ctx->event() == DRAG_SELECT_FINISHED ) 
    {
      highlight();
    }
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

  bool handle(ViewerContext* ctx, int index) {
    return true; // true means we are interested in the event
  }

};

/*
NO_EVENT "only call menu(), otherwise ignore";
DRAW_OPAQUE "solid objects/texture maps";
DRAW_TRANSPARENT "transparent (below alpha threshold) 3D pass";
DRAW_STIPPLED "draw things hidden behind solid objects";
DRAW_SHADOW "shadows under lines in the 2D pass";
DRAW_LINES "lines in 3D, and the entire 2D pass";
MOVE "draw_handles() is being called to find tooltip";
PUSH "user just pushed the mouse down, hit detection. Any handle callback which returns true for this event should also return true for DRAG and RELEASE";
DRAG "user is dragging mouse, region selection hit detection. Any handle callback which returns true for this event should also return true for PUSH and RELEASE";
RELEASE "user released the mouse. Any handle callback which returns true for this event should also return true for PUSH and DRAG";
KEY "user hit a key";
KEYUP "user let go of key";
DROP "user dropped some data in a drag'n'drop operation";
DROP_CHECK "user is dragging data over a handle";
ENTER_VIEWER "the mouse has entered the viewer";
EXIT_VIEWER "the mouse has exited the viewer";
CURSOR "hit-detecting for a handle cursor";
HOVER_ENTER "the hover was entered";
HOVER_MOVE "user is hovering";
HOVER_LEAVE "user has left hovering";
DRAG_SELECT "user is drag-selecting handles";
DRAG_SELECT_FINISHED "drag selection of handles has finished";
FIRST_MENU "recommended start of Menu events";
*/

/*
ViewerEvent {
  NO_EVENT, DRAW_OPAQUE, DRAW_TRANSPARENT, DRAW_STIPPLED,
  DRAW_SHADOW, DRAW_LINES, MOVE, PUSH,
  DRAG, RELEASE, KEY, KEYUP,
  DROP, DROP_CHECK, ENTER_VIEWER, EXIT_VIEWER,
  CURSOR, HOVER_ENTER, HOVER_MOVE, HOVER_LEAVE,
  DRAG_SELECT, DRAG_SELECT_FINISHED, FIRST_MENU
}
*/


static Op* build(Node *node) {
    return new Outliner(node);
}

const Op::Description Outliner::description(CLASS, 0, build);
