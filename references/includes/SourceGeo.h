// SourceGeo.h
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef DDImage_SourceGeo_h
#define DDImage_SourceGeo_h

#include "NullGeo.h"

namespace DD
{
  namespace Image
  {

    /*! \class DD::Image::SourceGeo
       Subclass of GeoOp that does not take another GeoOp as input. This
       alters the control panel knobs and tooltips slightly to avoid
       references to the input geometry.
     */
    class DDImage_API SourceGeo : public NullGeo
    {
      FrameRange _frameRange;

    public:

      /*! Constructs a single texture input. */
      SourceGeo(Node*);

      /*! Returns the Iop connected to the texture input. */
      Iop* input_iop() const;
      virtual Iop* default_material_iop() const { return input_iop(); }
      /*virtual*/ int minimum_inputs() const { return 1; }
      /*virtual*/ int maximum_inputs() const { return 1; }

      /*virtual*/ Op* default_input(int) const;
      /*virtual*/ bool test_input(int input, Op* op) const;
      /*virtual*/ const char* input_label(int input, char* buffer) const;

      /*! Calls the GeoOp baseclass with \b source_geo=true so that
          the display knobs are correct for a Source object.
       */
      /*virtual*/ void knobs(Knob_Callback);

      /*! Constructs the transform to apply before the iop input is followed.
       */
      /*virtual*/ void build_input_handles(ViewerContext*);

      /*virtual*/ const char* Class() const;
      /*virtual*/ const char* node_help() const;


      /*! Assign all the base parameters for each GeoInfo in the output GeometryList.
       */
      virtual void init_geoinfo_parms(Scene&, GeometryList&);

      bool render_checkmark() const { return render_; }

    protected:
      //! Only constructors should call this, unless knobs() is not called
      void render_checkmark(bool v) { render_ = v; }

      //! build_handles with the context model matrix multiplied by the local
      //! matrix so nodes up the tree are displayed correctly
      //! Deprecated.  Use GeoOp::build_matrix_handles with a local matrix pointer instead.
      void build_matrix_handles(ViewerContext* ctx, Matrix4 localMatrix);


      /*! Hash up knobs that affect the geo's material.
       */
      /*virtual*/ void get_geometry_hash();


      /*! SourceGeo subclass implementation calls geometry_engine() on each
          GeoInfo in the local cache list.
       */
      /*virtual*/ void geometry_engine(Scene&, GeometryList& out);

      virtual void create_geometry(Scene&, GeometryList& out) = 0;

      virtual void updateGeneralInfo();

    private:
      bool render_; //!< GUI render setting

    };

  }
}

#endif

// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
