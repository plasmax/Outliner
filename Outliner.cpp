
#include "DDImage/Op.h"
#include "DDImage/Knob.h"
#include "DDImage/Knobs.h"
#include "DDImage/GeoOp.h"
#include "DDImage/Scene.h"
#include "DDImage/gl.h"
#include "DDImage/ViewerContext.h"
#include "DDImage/TableKnobI.h"     //Defines the table knob interface used by, surprisingly enough, the Table_knob.
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
  Hash hashish;
  // static void draw_indicators_cb(void* p, ViewerContext* ctx) {
  //   ((Outliner*)p)->draw_indicators(ctx);
  // };
  // typedef std::vector<std::vector<Hash> > GeoHashes;
  // GeoHashes _geohashes;

public:
  static const Op::Description description;

  Outliner(Node* node) : GeoOp(node) {
    _table_updated = false;
    hashish = Hash(11823033677170338752);
  }

  const char* Class() const { return CLASS; }
  const char* node_help() const { return HELP; }
  // const char* node_shape() const { return "O"; }
  // int maximum_inputs() const { return -1; }

  void knobs(Knob_Callback f) {
    //Handles data storage and mem management itself. Needs to have relevant
    //cols added on creation only (hence f.makeKnobs check).
    Knob* tableKnob = Table_knob(f, "Table_knob");
    if (f.makeKnobs()) {
      Table_KnobI* tableKnobI = tableKnob->tableKnob();
      tableKnobI->addStringColumn("geo_names", "Geo Names", true, 170, false, true);
      tableKnobI->addColumn("geo_hashes", "geo_hashes", Table_KnobI::StringColumn, true);
      
      // if(tableKnobI->getRowCount() == 0) {
      //   tableKnobI->addRow();
      // }
    }
  }
  // virtual bool inputs_clockwise() const { return false; }

  // void _validate(bool for_real) {
  //   // update_geometry_hashes();
  //   // for (int i = 0; i < inputs(); i++) {
  //   //   Op* op = input(i);
  //   //   if (dynamic_cast<GeoOp*>(op)) {
  //   //     op->validate(for_real);
  //   //     }
  //     // }
  //   GeoOp::_validate(for_real);
  //   }

  // void update_geometry_hashes() {
  //   cache_hash 
  //   display3d_ 
  //   geo_hash 
  //   get_geometry_hash() 
  //   DD::Image::Op::node() 
  //   DD::Image::Op::node_gl_color() 
  //   rebuild_mask_ 
  //   render_mode_ 
  //   selectable_
  // }

  // void get_geometry_hash() {
  //   GeoOp::get_geometry_hash();
  //   geo_hash[Group_Points].append(Op::hash());
  //   for (int i = 0; i <= inputs(); i++) {
  //     if (!node_input(i, Op::OUTPUT_OP) == NULL) {
  //       geo_hash[Group_Points].append(input(i)->hash(Group_Points));
  //       geo_hash[Group_Primitives].append(input(i)->hash(Group_Primitives));
  //       geo_hash[Group_Matrix].append(input(i)->hash(Group_Matrix));
  //       geo_hash[Group_Attributes].append(input(i)->hash(Group_Attributes));
  //     }
  //   }
  //   }

  void update_table() {
    std::cout << "updating table" << std::endl;
    GeometryList& out = *scene_->object_list();

    Knob* tableKnob = knob("Table_knob");
    Table_KnobI* tk = tableKnob->tableKnob();
    // // tk->deleteAllItems();
    // // tk->reset();
    int numobjs = out.size();
    std::cout << "out:" << numobjs << std::endl;
    for (int r = 0; r < tk->getRowCount(); r++ ) {
      tk->deleteRow(r);
      // if (r > numobjs) {
      //   tk->deleteRow(r);
      //   }
      }

    for ( unsigned obj = 0; obj < out.size(); obj++ ) {
      GeoInfo& info = out[obj];
      const Hash hash = info.src_id();
      tk->addRow(obj);
      std::string stringhash = std::to_string(hash.value());
      tk->setCellString(obj, 1, stringhash);
      }
    
  }

  // void geometry_engine(Scene& scene, GeometryList& out) {
    // GeoOp::geometry_engine(scene, out);

    // for (int i = 0; i <= inputs(); i++) {
    //   if (!node_input(i, Op::OUTPUT_OP) == NULL) {
    //     input(i)->get_geometry(scene, out);
    //     }
    //   }

    // out.synchronize_objects();
    // }

  // HandlesMode doAnyHandles(ViewerContext* ctx) {
  //   if (ctx->transform_mode() != VIEWER_2D)
  //     return eHandlesCooked;

  //   return Op::doAnyHandles(ctx);
  //   }

  void build_handles(ViewerContext* ctx) {
    // std::cout << "Building handles..." << std::endl;
    if (ctx->transform_mode() == VIEWER_2D) return;
    if (ctx->viewer_mode() != VIEWER_2D && !node_disabled()) {
      if (ctx->connected() >= SHOW_OBJECT) {
        validate(false);
        // construct output geometry and add callbacks to draw it in the viewer.
        add_draw_geometry(ctx); 
        // ctx->connected(CONNECTED); // do not pass through objects
        // ctx->connected(SHOW_PUSHED_OBJECT); // Connected, draw object if node is pushed

        // 
        // ctx->add_draw_handle(draw_indicators_cb, this, node()); //, eDrawHandleAlways);

        }
      }

    for (int i = 0; i <= inputs(); i++) {
      if (!node_input(i, Op::OUTPUT_OP) == NULL) {
        add_input_handle(i, ctx); // calls build_handles on input ops

        // hack - select the geometry
        // GeoOp& gop = input(i);
        // Scene* scn = gop->scene();
        // GeometryList& geolist = *input(i)->scene()->object_list();
        // for (unsigned o = 0; o <= geolist.size(); o++) {
        //   GeoInfo& info = geolist[o];
        //   info.selected = true;
        // }
      }
    }

    build_knob_handles(ctx);

    add_draw_handle(ctx);

    if (!_table_updated) {
      update_table();
      _table_updated = true;
    }

    }


  // // custom function
  // void draw_indicators(ViewerContext* ctx) {
  //   std::cout << "Drawing my indicators! Probably should put some OpenGL stuff in here...\n";
  //   }

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
    // tk->clearSelection();

    GeometryList& out = *scene_->object_list();
    for (unsigned o = 0; o <= out.size(); o++) {
      GeoInfo& info = out[o];
      
      // char chr = char(info.src_id().value());
      Hash hash = info.src_id();
      // std::cout << hash;
      // if (info.selected) {
      //   std::cout << "  Is selected!";
      //   }
      // std::cout << std::endl;
      // _geohashes.push_back(hash);
      // _geohashes.push_back(hash.value());
        
      // int num = int(o);
      // if (info.selected) {
      //   // tk->selectRow(int(o));
      // } else {
      for (unsigned i = 0; i < selrows.size(); i++) {
        int sr = selrows[i];
        std::cout << sr << std::endl;
        if (sr == int(o)) {
          info.selected = true; // my favourite line
        } 
        // else {
        //   info.selected = false;
        // }
      }
      // }

 

      // info.selected = true; // my favourite line
      // if (num == 2) {
      //   info.selected = true; // my favourite line
      // }
    }
  }

  void draw_handle(ViewerContext* ctx) {
    GeoOp::draw_handle(ctx);
    highlight();


  //   for (int i = 0; i <= inputs(); i++) {
  //     if (!node_input(i, Op::OUTPUT_OP) == NULL) {

  //       GeometryList& geolist = *input(i)->scene()->object_list();
  //       for (unsigned o = 0; o <= geolist.size(); o++) {
  //         GeoInfo& info = geolist[o];
  //         info.selected = true;
  //       }
  //     }
  //   }
  }

  int knob_changed(Knob* k) {
    std::cout << k->name() << std::endl;
    if (k->name() == "inputChange") {
      // update_table();
      _table_updated = false;
      return 1;
    }
    if (k->name() == "showPanel") {
      // update_table();
      _table_updated = false;
      return 1;
    }
    // // GeoOp::knob_changed(k);
    return 0;
  }
    // 

    // construct output geometry and add callbacks to draw it in the viewer.
  // void add_draw_geometry(ViewerContext* ctx) {

//     // this function references the following 
//     // (although implementation is a total mystery...)
//     // scene_
//     // ViewerContext::add_draw_handle()
//     // Box3::empty() 
//     // ViewerContext::expand_bbox() 
//     // Matrix4::makeIdentity() 
//     // GeoInfo::matrix 
//     // ViewerContext::modelmatrix 
//     // Op::node()
//     // Op::node_selected() 
//     // Scene::object_list_ 
//     // GeoInfo::select_geo 
//     // select_geometry(ViewerContext* ctx, GeometryList &scene_objects)
//     // GeoInfo::selected 
//     // setupScene() 
//     // SHOW_BBOX 
//     // GeoInfo::source_geo 
//     // Box3::transform()

    // the above referenced functions inside this one conspire 
    // to allow objects to have their "selected" attribute set when 
    // the node is selected. I suspect that it only loops through 
    // this node's geometry
    // GeoOp::add_draw_geometry(ctx);

//     GeometryList& geolist = *scene_->object_list();
//     // Search up the tree to set the selected nodes and bounding box based on whether user has nodes selected and/or open.
//     // References DD::Image::GeoInfo::display3d, 
//       // DD::Image::Op::input(), 
//       // DD::Image::Op::node_selected(), 
//       // DD::Image::GeoInfo::select_geo, 
//       // select_geometry(), 
//       // DD::Image::GeoInfo::selectable, 
//       // and DD::Image::GeoInfo::selected.
//     // select_geometry(ctx, geolist);
//     for (unsigned o = 0; o <= geolist.size(); o++) {
//       GeoInfo& info = geolist[o];
//       info.selected = true;
//     }

//     // these crash it :( probably accessing stuff out of scope
    // for (int i = 0; i <= inputs(); i++) {
    //   if (!node_input(i, Op::OUTPUT_OP) == NULL) {
    //     add_input_handle(i, ctx); // calls build_handles on input ops

    //     // hack - select the geometry
    //     // GeoOp& gop = input(i);
    //     // Scene* scn = gop->scene();
    //     GeometryList& geolist = *input(i)->scene()->object_list();
    //     for (unsigned o = 0; o <= geolist.size(); o++) {
    //       GeoInfo& info = geolist[o];
    //       info.selected = true;
    //     }
    //   }
    // }

// //       // void add_draw_handle(DrawHandleCallbackFunc cb, 
// //                               // void*, 
// //                               // Node*, // set the colours and choose which node to be selected if the user clicks on any of the graphics drawn by the callback.
// //                               // DrawHandleTypeMask drawHandleType = eDrawHandleNodeSelection);
// //       // ViewerContext::add_draw_handle(ctx);
// //       // ctx->add_draw_handle(handleCallback, node(), eDrawHandleAlways);
// //       ctx->add_draw_handle(handleCallback, node(), eDrawHandleAlways);


    // }
};

static Op* build(Node *node) {
    return new Outliner(node); 
}

const Op::Description Outliner::description(CLASS, 0, build);













// #include "DDImage/Op.h"
// #include "DDImage/Knob.h"
// #include "DDImage/GeoOp.h"
// #include "DDImage/Scene.h"
// #include "DDImage/gl.h"
// #include "DDImage/ViewerContext.h"
// #include "DDImage/Scene.h"

// using namespace DD::Image;

// static const char* const CLASS = "Outliner";
// static const char* const HELP = 
// "Filter and Transform Objects in a Scene. Selecting the object "
// "in the SceneView Knob selects it in the Scene, and vice-versa."
// "Additionally, object transformations can be parented in the SceneView"
// "And arbitrary transform handles can be added in the view."
// "And selecting multiple pieces of geometry allows you to move them all at once";

// class Outliner : public GeoOp, public Scene
// {
// public:
//   static const Op::Description description;

//   Outliner(Node* node) : GeoOp(node) {}

//   const char* Class() const { return CLASS; }
//   const char* node_help() const { return HELP; }
// };

// static Op* build(Node *node) {
//     return new Outliner(node); 
// }

// const Op::Description Outliner::description(CLASS, 0, build);


















// #include "Python.h"

// #include <DDImage/Render.h>
// #include <DDImage/Knobs.h>
// #include <DDImage/Knob.h>
// #include <DDImage/ModifyGeo.h>
// #include <DDImage/ViewerContext.h>
// #include <DDImage/DDMath.h>
// #include <DDImage/GeometryList.h>
// #include <assert.h>
// #include "structmember.h"
// #include <iostream>
// //
// // GLOBALS
// //

// extern const char* const CLASS;
// extern const char* const HELP;


// using namespace DD::Image;

// //
// // TYPES
// //

// class Outliner;


// // Define a custom knob which allows us to add handles in the viewer 
// class Outliner_Knob : public DD::Image::Knob, public DD::Image::PluginPython_KnobI
// {
// // private:
// //   ViewerContext* myctx;

// public:
//   const char* Class() const;
//   Outliner_Knob(DD::Image::Knob_Closure* kc, Outliner* pgOp, const char* n);
//   ~Outliner_Knob();

//   bool build_handle(DD::Image::ViewerContext*);
//   void draw_handle(DD::Image::ViewerContext*);

//   bool isHandleSelected( int i );

//   PyObject* getGeometry();
//   PyObject* getSelection();

//   //! Expose this knob to python
//   virtual DD::Image::PluginPython_KnobI* pluginPythonKnob();

// private:
//   static bool handleCallback(DD::Image::ViewerContext*, DD::Image::Knob*, int);

// private:
//   typedef std::vector<std::vector<unsigned int> > GeoSelection;
//   GeoSelection _selection;
//   Outliner *_pgOp;
//   DD::Image::Scene *_scene;
// };


// // The Op class.
// class Outliner : public DD::Image::ModifyGeo
// {
// public:
//   static const DD::Image::Op::Description description;

//   bool _allowSelection;
//   bool _select_objs;
  
//   Outliner(Node* node);

//   void knobs(DD::Image::Knob_Callback f);
//   void get_geometry_hash();
//   void modify_geometry(int obj, DD::Image::Scene& scene, DD::Image::GeometryList& out);
  
//   const char* Class() const {return "Outliner";}
//   const char* node_help() const {return "internal";}
// };


// typedef struct {
//   PyObject_HEAD
//   DD::Image::GeometryList *_geo;
// } PyGeometryListObject;


// typedef struct {
//   PyObject_HEAD
//   PyGeometryListObject *_geo; // We refer back to the parent object so that when it goes out of scope, so do all the GeoInfos.
//   unsigned int _index;
// } PyGeoInfoObject;


// typedef struct {
//   PyObject_HEAD
//   Outliner_Knob* _knob;
// } OutlinerKnobObject;


// //
// // FUNCTIONS
// //

// // Geometry list functions.
// PyObject* GeometryList_new(PyTypeObject *type, PyObject *args, PyObject *kwds);
// PyGeometryListObject* PyGeometryListObject_FromGeometryList(DD::Image::GeometryList *geometryList);
// void GeometryList_dealloc(PyObject *o);
// Py_ssize_t GeometryList_slength(PyGeometryListObject *o);
// PyObject* GeometryList_item(PyGeometryListObject *o, register Py_ssize_t i);

// // GeoInfo functions.
// PyObject *GeoInfo_new( PyTypeObject *type, PyObject *args, PyObject *kwds );
// PyGeoInfoObject *PyGeoInfoObject_FromGeoInfo( PyGeometryListObject *geometryList, int index );
// void GeoInfo_dealloc(PyObject *o);
// PyObject *GeoInfo_points( PyGeoInfoObject *self, PyObject *args, PyObject *kwds );
// PyObject *GeoInfo_primitives( PyGeoInfoObject *self, PyObject *args, PyObject *kwds );
// PyObject *GeoInfo_normals( PyGeoInfoObject *self, PyObject *args, PyObject *kwds );

// // Outliner_Knob functions
// Outliner_Knob* Outliner_knob(DD::Image::Knob_Callback f, Outliner* p, const char* name);
// PyObject* OutlinerKnob_new(PyTypeObject* type, PyObject* args, PyObject* kwargs);
// void OutlinerKnob_dealloc(PyObject *o);
// PyObject* OutlinerKnob_getGeometry(PyObject* self, PyObject* args);
// PyObject* OutlinerKnob_getSelection(PyObject* self, PyObject* args);

// // Outliner functions
// PyGeoInfoObject* PyGeoInfoObject_FromGeoInfo(PyGeometryListObject *o, int index);

// // Other functions.
// DD::Image::Op* build(Node* node);






// //
// // Global variables.
// //

// const char* const CLASS = "Outliner";
// const char* const HELP = "@i;Outliner@n; An example plugin which manipulates 3D geometry using Python.";


// //
// // Macros
// //

// #define PyGeometryListObjectCheckValid(o) if (!o->_geo) { PyErr_SetString(PyExc_IndexError, "object out of scope"); return 0; }
// #define PyGeoInfoObjectCheckValid(o)      if (!o->_geo || !o->_geo->_geo) { PyErr_SetString(PyExc_IndexError, "object out of scope"); return NULL; }
// #define OutlinerKnobObjectCheckValid(o)  if (!o->_knob) { PyErr_SetString(PyExc_IndexError, "object out of scope"); return NULL; }


// extern PyTypeObject PyGeoInfo_Type;
// extern PyTypeObject PyGeometryList_Type;
// extern PyTypeObject PyPrimitive_Type;
// extern PyTypeObject OutlinerKnob_Type;


// //
// // GeometryList python definitions.
// //

// static PySequenceMethods GeometryList_as_sequence = {
//   (lenfunc)GeometryList_slength, /*sq_length*/
//   0, /*sq_concat*/
//   0, /*sq_repeat*/
//   (ssizeargfunc)GeometryList_item, /*sq_item*/
//   0, /*sq_slice*/
//   0,    /*sq_ass_item*/
//   0,    /*sq_ass_slice*/
//   0 /*sq_contains*/
// };

// static PyMethodDef GeometryList_methods[] = {
//   { NULL, NULL }
// };

// PyTypeObject PyGeometryList_Type = {
//   PyObject_HEAD_INIT(&PyType_Type)
//   0,
//   "GeometryList",
//   sizeof(PyGeometryListObject),
//   0,
//   GeometryList_dealloc,     /* tp_dealloc */
//   0,//(printfunc)GeometryList_print,    /* tp_print */
//   0,          /* tp_getattr */
//   0,          /* tp_setattr */
//   0,          /* tp_compare */
//   0,//(reprfunc)GeometryList_repr,      /* tp_repr */
//   0,          /* tp_as_number */
//   &GeometryList_as_sequence,        /* tp_as_sequence */
//   0,          /* tp_as_mapping */
//   0,//(hashfunc)GeometryList_hash,    /* tp_hash */
//   0,          /* tp_call */
//   0,//(reprfunc)GeometryList_str,     /* tp_str */
//   PyObject_GenericGetAttr,    /* tp_getattro */
//   0,          /* tp_setattro */
//   0,          /* tp_as_buffer */
//   Py_TPFLAGS_DEFAULT, /* tp_flags */
//   0,        /* tp_doc */
//   0,          /* tp_traverse */
//   0,          /* tp_clear */
//   0,          /* tp_richcompare */
//   0,          /* tp_weaklistoffset */
//   0,          /* tp_iter */
//   0,          /* tp_iternext */
//   GeometryList_methods,     /* tp_methods */
//   0,          /* tp_members */
//   0,      /* tp_getset */
//   0,          /* tp_base */
//   0,          /* tp_dict */
//   0,          /* tp_descr_get */
//   0,          /* tp_descr_set */
//   0,          /* tp_dictoffset */
//   0,          /* tp_init */
//   PyType_GenericAlloc,      /* tp_alloc */
//   GeometryList_new,       /* tp_new */
//   PyObject_Del,               /* tp_free */
// };


// //
// // GeoInfo definitions
// //

// static PyMethodDef GeoInfo_methods[] = {
//   { "points", (PyCFunction)GeoInfo_points, METH_NOARGS, "points" },
//   { "primitives", (PyCFunction)GeoInfo_primitives, METH_NOARGS, "primitives" },
//   { "normals", (PyCFunction)GeoInfo_normals, METH_NOARGS, "normals" },
//   { NULL, NULL }
// };

// PyTypeObject PyGeoInfo_Type = {
//   PyObject_HEAD_INIT(&PyType_Type)
//   0,
//   "Geometry",
//   sizeof(PyGeoInfoObject),
//   0,
//   GeoInfo_dealloc,      /* tp_dealloc */
//   0,//(printfunc)GeoInfo_print,   /* tp_print */
//   0,          /* tp_getattr */
//   0,          /* tp_setattr */
//   0,          /* tp_compare */
//   0,      /* tp_repr */
//   0,          /* tp_as_number */
//   0,        /* tp_as_sequence */
//   0,          /* tp_as_mapping */
//   0,    /* tp_hash */
//   0,          /* tp_call */
//   0,      /* tp_str */
//   PyObject_GenericGetAttr,    /* tp_getattro */
//   0,          /* tp_setattro */
//   0,          /* tp_as_buffer */
//   Py_TPFLAGS_DEFAULT, /* tp_flags */
//   0,        /* tp_doc */
//   0,          /* tp_traverse */
//   0,          /* tp_clear */
//   0,          /* tp_richcompare */
//   0,          /* tp_weaklistoffset */
//   0,          /* tp_iter */
//   0,          /* tp_iternext */
//   GeoInfo_methods,      /* tp_methods */
//   0,          /* tp_members */
//   0,      /* tp_getset */
//   0,          /* tp_base */
//   0,          /* tp_dict */
//   0,          /* tp_descr_get */
//   0,          /* tp_descr_set */
//   0,          /* tp_dictoffset */
//   0,          /* tp_init */
//   PyType_GenericAlloc,/* tp_alloc */
//   GeoInfo_new,        /* tp_new */
//   PyObject_Del,       /* tp_free */
// };


// //
// // Outliner_Knob python definitions
// //
// static PyMethodDef OutlinerKnob_methods[] = {
//   { "getGeometry",  OutlinerKnob_getGeometry,  METH_VARARGS, "getGeometry() -> geometry list." },
//   { "getSelection", OutlinerKnob_getSelection, METH_VARARGS, "getSelection() -> list of selected geometry indices." },
//   { NULL, NULL, 0, NULL }
// };

// PyTypeObject OutlinerKnob_Type = {
//   PyObject_HEAD_INIT(&PyType_Type)
//   0,
//   "Outliner_Knob",
//   sizeof(OutlinerKnobObject),
//   0,
//   OutlinerKnob_dealloc,      /* tp_dealloc */
//   0,    /* tp_print */
//   0,          /* tp_getattr */
//   0,          /* tp_setattr */
//   0,          /* tp_compare */
//   0,      /* tp_repr */
//   0,          /* tp_as_number */
//   0,        /* tp_as_sequence */
//   0,          /* tp_as_mapping */
//   0,    /* tp_hash */
//   0,          /* tp_call */
//   0,      /* tp_str */
//   PyObject_GenericGetAttr,    /* tp_getattro */
//   PyObject_GenericSetAttr,          /* tp_setattro */
//   0,          /* tp_as_buffer */
//   Py_TPFLAGS_DEFAULT, /* tp_flags */
//   0,        /* tp_doc */
//   0,          /* tp_traverse */
//   0,          /* tp_clear */
//   0,          /* tp_richcompare */
//   0,          /* tp_weaklistoffset */
//   0,          /* tp_iter */
//   0,          /* tp_iternext */
//   OutlinerKnob_methods,      /* tp_methods */
//   0,          /* tp_members */
//   0,      /* tp_getset */
//   0,          /* tp_base */
//   0,          /* tp_dict */
//   0,          /* tp_descr_get */
//   0,          /* tp_descr_set */
//   0,          /* tp_dictoffset */
//   0,          /* tp_init */
//   PyType_GenericAlloc,      /* tp_alloc */
//   OutlinerKnob_new,        /* tp_new */
//   PyObject_Del,               /* tp_free */
// };


// //
// // Python wrapper classes for 3D geometry
// //

// // These are very basic and only serve as an example of how we could do this.
// // The plugin provides the Python script with a class "GeometryList" which behaves as a sequence
// // containing "Geometry" instances. "Geometry" has methods to return the Primitives as a tuple containing
// // tuples of vertices, the points as a tuple of XYZ triples and the normals in the same format. It would be better
// // to have wrapper classes for these as sequences as well so that a script could just pull out, say, primitive 0
// // without having to create the whole massive tuple, but time is pressing and we don't want to make this example
// // too complicated. We should also add a class for Primitives as so on....

// // We manage the lifetime of the GeometryList class by the primitive expedient of making it invalid once the
// // script returns. This prevents Python retaining a pointer onto geometry which may be freed unexpectedly.

// //
// // GeometryList python functions
// //

// PyObject *GeometryList_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
// {
//   if ( !PyArg_ParseTuple( args, ":GeometryList" ) )
//     return NULL;

//   PyGeometryListObject *o = (PyGeometryListObject *) PyObject_MALLOC(sizeof(PyGeometryListObject));
//   if (o == NULL)
//     return PyErr_NoMemory();
//   PyObject_INIT( o, &PyGeometryList_Type );
//   o->_geo = NULL;
//   return (PyObject *)o;
// }

// PyGeometryListObject *PyGeometryListObject_FromGeometryList( GeometryList *geometryList )
// {
//   PyGeometryListObject *o = (PyGeometryListObject *) PyObject_MALLOC(sizeof(PyGeometryListObject));
//   PyObject_INIT( o, &PyGeometryList_Type );
//   o->_geo = geometryList;
//   return o;
// }

// void GeometryList_dealloc( PyObject *o )
// {
// }

// Py_ssize_t GeometryList_slength( PyGeometryListObject *o )
// {
//   PyGeometryListObjectCheckValid( o );
//   return o->_geo->size();
// }

// PyObject *GeometryList_item( PyGeometryListObject *o, register Py_ssize_t i )
// {
//   PyGeometryListObjectCheckValid( o );
//   if ( i < 0 || i >= (Py_ssize_t)o->_geo->size() ) {
//     PyErr_SetString(PyExc_IndexError, "index out of range");
//     return NULL;
//   }
//   return (PyObject *)PyGeoInfoObject_FromGeoInfo( o, static_cast<int>(i) );
// }


// //
// // GeoInfo python functions
// //

// PyObject *GeoInfo_new( PyTypeObject *type, PyObject *args, PyObject *kwds )
// {
//   if ( !PyArg_ParseTuple( args, ":Geometry" ) )
//     return NULL;

//   PyGeoInfoObject *o = (PyGeoInfoObject *) PyObject_MALLOC(sizeof(PyGeoInfoObject));
//   if (o == NULL)
//     return PyErr_NoMemory();
//   PyObject_INIT( o, &PyGeoInfo_Type );
//   o->_geo = NULL;
//   return (PyObject *)o;
// }


// PyGeoInfoObject *PyGeoInfoObject_FromGeoInfo( PyGeometryListObject *geometryList, int index )
// {
//   PyGeoInfoObject *o = (PyGeoInfoObject *) PyObject_MALLOC(sizeof(PyGeoInfoObject));
//   PyObject_INIT( o, &PyGeoInfo_Type );
//   o->_geo = geometryList;
//   o->_index = index;
//   Py_INCREF( o->_geo );
//   return o;
// }


// void GeoInfo_dealloc( PyObject *o )
// {
//   Py_XDECREF( ((PyGeoInfoObject*)o)->_geo );
// }


// PyObject *GeoInfo_points( PyGeoInfoObject *self, PyObject *args, PyObject *kwds ) {
//   PyGeoInfoObjectCheckValid( self );
//   // const GeoInfo& info = (*self->_geo->_geo)[self->_index];

//   // HACK
//   GeoInfo& info = (*self->_geo->_geo)[self->_index]; // error C3490: 'selected' cannot be modified because it is being accessed through a const object
//   info.selected = true; 
//   std::cout << "Is this the real life?\n";

//   const PointList* points = info.point_list();
//   size_t n = points->size();

//   PyObject *list = PyTuple_New( n*3 );
//   size_t j = 0;
//   for ( size_t i = 0; i < n; i++ ) {
//     const Vector3& v = (*points)[i];
//     PyTuple_SetItem( list, j++, PyFloat_FromDouble( v.x ) );
//     PyTuple_SetItem( list, j++, PyFloat_FromDouble( v.y ) );
//     PyTuple_SetItem( list, j++, PyFloat_FromDouble( v.z ) );
//   }
//   return list;
// }

// PyObject *GeoInfo_primitives( PyGeoInfoObject *self, PyObject *args, PyObject *kwds ) {
//   PyGeoInfoObjectCheckValid( self );
//   const GeoInfo& info = (*self->_geo->_geo)[self->_index];
//   const Primitive** PRIMS = info.primitive_array();
//   const unsigned prims = info.primitives();
//   PyObject *list = PyTuple_New( prims );
//   for ( unsigned i = 0; i < prims; i++ ) {
//     const Primitive* prim = *PRIMS++;
//     PyObject *vertices = PyTuple_New( prim->vertices() );
//     for ( unsigned j = 0; j < prim->vertices(); j++ )
//       PyTuple_SetItem( vertices, j, PyInt_FromLong( prim->vertex(j) ) );
// //    Py_INCREF( vertices );
//     PyTuple_SetItem( list, i, vertices );
//   }
//   return list;
// }

// PyObject *GeoInfo_normals( PyGeoInfoObject *self, PyObject *args, PyObject *kwds ) {
//   PyGeoInfoObjectCheckValid( self );
//   const GeoInfo& info = (*self->_geo->_geo)[self->_index];
//   int N_group = Group_None;
//   const AttribContext* N_attrib  = info.get_attribcontext("N");
//   if (N_attrib &&
//       (!N_attrib->attribute || !N_attrib->attribute->size()))
//     N_attrib = 0;
//   if (N_attrib) {
//     N_group = N_attrib->group;
//     // TODO - here we need to worry about thwether we're dealing with point or vertex normals.
//     // For the purposes of this example, I'm assuming point normals.
//     //    if (N_group == Group_Vertices)
//     //    else if (N_group == Group_Points)
//     unsigned normals = N_attrib->attribute->size();
//     PyObject *list = PyTuple_New( normals*3 );
//     int j = 0;
//     for ( unsigned i = 0; i < normals; i++ ) {
//       Vector3 N = N_attrib->attribute->normal(i);
//       PyTuple_SetItem( list, j++, PyFloat_FromDouble( N.x ) );
//       PyTuple_SetItem( list, j++, PyFloat_FromDouble( N.y ) );
//       PyTuple_SetItem( list, j++, PyFloat_FromDouble( N.z ) );
//     }
//     return list;
//   }
//   Py_RETURN_NONE;
// }


// //
// // Outliner_Knob methods.
// //


// const char* Outliner_Knob::Class() const
// {
//   return "Outliner_Knob";
// }


// Outliner_Knob::Outliner_Knob(DD::Image::Knob_Closure *kc, Outliner *pgOp, const char* n) :
//   Knob(kc, n),
//   PluginPython_KnobI(),
//   _pgOp(pgOp),
//   _scene(0)
// {
//   setPythonType(&OutlinerKnob_Type);
//   set_flag( DO_NOT_WRITE );
//   // myctx = nullptr;
// }


// Outliner_Knob::~Outliner_Knob()
// {
//   delete _scene;
// }


// bool Outliner_Knob::build_handle(ViewerContext* ctx)
// {
//   return ctx->transform_mode() != VIEWER_2D;
// }


// void Outliner_Knob::draw_handle(ViewerContext* ctx) {
//   // All we do here is create a selectable viewer handle for each point in the incoming geometry.
//   // To go further, we could provide handles for edges and faces, or whatever.
//   // myctx = ctx;
//   if ( ctx->draw_knobs() && _pgOp->_allowSelection ) {
//     Scene *scene = _pgOp->scene();
//     GeometryList& out = *scene->object_list();

//     _selection.clear();
//     int startPoint = 0;
//     for ( unsigned obj = 0; obj < out.size(); obj++ ) {
//       GeoInfo& info = out[obj];
//       if (_pgOp->_select_objs) {
//         info.selected = true; // the only way that works (on linux and pc!) inside draw_handle
//       }
//       const PointList* points = info.point_list();
//       const int n = static_cast<int>(points->size());
//       std::vector<unsigned int> objSelection;
//       for ( int i = 0; i < n; i++ ) {
//         const Vector3 v = (*points)[i];
//         make_handle( SELECTABLE, ctx, handleCallback, i + startPoint, v.x, v.y, v.z, ViewerContext::kCrossCursor );
//         // Here we save the list of selected handles. This is done way too often, but we need a ViewerContext
//         // to find out if a handle is selected, so we can't just query the selection when the user clicks on the
//         // button.
//         if ( is_selected( ctx, handleCallback, i + startPoint ) )
//           objSelection.push_back( i );
//       }
//       startPoint =+ n;
//       _selection.push_back( objSelection );
//     }
//   }
// }


// /* This was never defined...?
// bool Outliner_Knob::isHandleSelected(int i)
// {
// }
// */

// PyObject* Outliner_Knob::getGeometry()
// {
//   if (_pgOp == NULL)
//     Py_RETURN_NONE;

//   GeoOp *myOp = dynamic_cast<GeoOp*>( _pgOp->node_input( 0, Op::EXECUTABLE_SKIP ) );

//   if ( !myOp ) 
//     Py_RETURN_NONE;

//   delete _scene;
//   _scene = new Scene();
//   myOp->validate(true);
//   myOp->build_scene( *_scene );
  

//   // // GeometryList* out = *_scene->object_list();
//   // GeometryList& out = *_scene->object_list();
//   // // experiment
//   // for ( unsigned obj = 0; obj < out.size(); obj++ ) {
//   //   GeoInfo& info = out[obj];
//   //   // info.draw(myctx);
//   //   info.selected = true;
//   //   std::cout << info.src_id() << std::endl;
//   // }
//   // Py_RETURN_NONE; // premature termination after experiment

//   GeometryList* out = _scene->object_list();
//   if (out == NULL)
//     Py_RETURN_NONE;

//   PyGeometryListObject* geometryList = PyGeometryListObject_FromGeometryList(out);
//   if (geometryList == NULL)
//     Py_RETURN_NONE;

//   return (PyObject*)geometryList;
// }


// PyObject* Outliner_Knob::getSelection()
// {
//   PyObject* objList = PyTuple_New(_selection.size());
//   for (unsigned int curObj = 0; curObj < _selection.size(); ++curObj) {
//     std::vector<unsigned int>& selPoints = _selection[curObj];
//     PyObject* selectionList = PyTuple_New(selPoints.size());
//     for (unsigned int i = 0; i < selPoints.size(); ++i)
//       PyTuple_SetItem(selectionList, i, PyInt_FromLong(selPoints[i]));
//     PyTuple_SetItem(objList, curObj, selectionList);
//   }
//   return objList;
// }


// PluginPython_KnobI* Outliner_Knob::pluginPythonKnob()
// {
//   return this;
// }


// bool Outliner_Knob::handleCallback(ViewerContext* ctx, Knob* p, int index)
// {
//   // We're not handling any events for this example, but we could, say, provide a popup menu when
//   // a user clicks on a point, or call our Python script.
//   return false;
// }


// // A helper function to create the custom knob
// Outliner_Knob* Outliner_knob(Knob_Callback f, Outliner* pyGeo, const char* name)
// {
//   if ( f.makeKnobs() ) {
//     Outliner_Knob* knob = new Outliner_Knob(&f, pyGeo, name);
//     f(DD::Image::PLUGIN_PYTHON_KNOB, Custom, knob, name, NULL, pyGeo);
//     return knob;
//   } else {
//     f(DD::Image::PLUGIN_PYTHON_KNOB, Custom, NULL, name, NULL, pyGeo);
//     return 0;
//   }
// }


// PyObject* OutlinerKnob_new(PyTypeObject* type, PyObject* args, PyObject* kwargs)
// {
//   Py_RETURN_NONE;
// }


// void OutlinerKnob_dealloc(PyObject *o)
// {
// }


// PyObject* OutlinerKnob_getGeometry(PyObject* self, PyObject* args)
// {
//   OutlinerKnobObject* knob = (OutlinerKnobObject*)self;
//   OutlinerKnobObjectCheckValid(knob);
//   if (knob->_knob == NULL)
//     Py_RETURN_NONE;
//   return knob->_knob->getGeometry();
// }


// PyObject* OutlinerKnob_getSelection(PyObject* self, PyObject* args)
// {
//   OutlinerKnobObject* knob = (OutlinerKnobObject*)self;
//   OutlinerKnobObjectCheckValid(knob);
//   if (knob->_knob == NULL)
//     Py_RETURN_NONE;
//   return knob->_knob->getSelection();
// }


// //
// // Outliner methods
// //

// // We subclass ModifyGeo although we're not modifying anything. The same techniques can be used
// // to call Python to implement geometry modification and that's what this example originally did,
// // hence the superclass.
// Outliner::Outliner(Node *node) : ModifyGeo(node), _allowSelection(true), _select_objs(false)
// {
// }


// void Outliner::knobs(Knob_Callback f)
// {
//   ModifyGeo::knobs(f);
//   Bool_knob( f,  &_allowSelection, "allowSelection" );
//   Bool_knob( f,  &_select_objs, "_select_objs" );
//   SetFlags( f, Knob::STARTLINE );
//   Outliner_knob( f, this, "geo" );
// }


// //! Hash up knobs that may affect points
// void Outliner::get_geometry_hash()
// {
//   ModifyGeo::get_geometry_hash(); // Get all hashes up-to-date
//   geo_hash[Group_Points].append(Op::hash());
//   geo_hash[Group_Points].append(_allowSelection);
//   geo_hash[Group_Points].append(_select_objs);
// }


// void Outliner::modify_geometry(int obj, Scene& scene, GeometryList& out)
// {
//   // We don't do anything here, but we could call our Python script to modify the geometry if we wanted.
// }


// Op* build(Node *node)
// {
//   return new Outliner(node);
// }

// const Op::Description Outliner::description(CLASS, build);


